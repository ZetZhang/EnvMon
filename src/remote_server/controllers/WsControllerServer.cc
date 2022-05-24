#include "WsControllerServer.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

void WsControllerServer::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    LOG_DEBUG << "from per mes: " << message;

    if (type == WebSocketMessageType::Ping) {
        LOG_DEBUG << "recv a ping";
    } else if (type == WebSocketMessageType::Text) {
        Json::Value root;
        Json::Reader reader;
        bool parsingSucc = reader.parse(message, root);
        const Json::Value identity = root["identity"];
        if (!identity)
            return;
        auto &s = wsConnPtr->getContextRef<SubScriber>();

        int id = identity.asInt();
        if (id == 1) { // from gateway, hand out data
            // redis
            // postgresql

            // merge and publish
            Json::Value sensor = root["sensor"];
            if (sensor) {
                Json::Value newSensorValue;
                Json::StreamWriterBuilder wbuilder;
                newSensorValue["identity"] = 2;
                newSensorValue["sensor"] = sensor;
                std::string document = Json::writeString(wbuilder, newSensorValue);
                _psService.publish(s.name, document);
            }
            //
            // threshold publish
            Json::Value threshold = root["threshold"];
            if (threshold) {
                Json::Value newThresholdValue;
                Json::StreamWriterBuilder wbuilder;
                newThresholdValue["identity"] = 4;
                newThresholdValue["threshold"] = threshold;
                std::string document = Json::writeString(wbuilder, newThresholdValue);
                _psService.publish(s.name, document);
            }

            // control publish
            Json::Value control = root["control"];
            if (control) {
                Json::Value newControlValue;
                Json::StreamWriterBuilder wbuilder;
                newControlValue["identity"] = 3;
                newControlValue["control"] = control;
                std::string document = Json::writeString(wbuilder, newControlValue);
                _psService.publish(s.name, document);
            }
        } else if (id == 2) { // sensor ignore
        //     // wsConnPtr->send("fuck");
        //     const Json::Value 
        //     auto &s = wsConnPtr->getContextRef<SubScriber>();
        //     _psService.publish(s.name, message);
            LOG_DEBUG << "error message";
        } else if (id == 3) { // control
            LOG_DEBUG << "control message";
        } else if (id == 4) { // threshold
            LOG_DEBUG << "threshold message";
        } else if (id == 0) { // other
            LOG_DEBUG << "okfuck";
            // wsConnPtr->send("okfuck");
        }
        _psService.publish(s.name, message);
    }
    // wsConnPtr->send("response");
}

void WsControllerServer::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    LOG_DEBUG << "new websocket connection..";
    SubScriber s;
    s.name= req->getParameter("room_name");
    s.id = _psService.subscribe(s.name,
                                 [wsConnPtr](const std::string &topic,
                                        const std::string &message) {
                                     // Supress unused variable warning
                                     (void)topic;
                                     wsConnPtr->send(message);
                                     // (void)message;
                                 });
    wsConnPtr->setContext(std::make_shared<SubScriber>(std::move(s)));
}

void WsControllerServer::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    LOG_DEBUG << "websocket closed connection..";
    auto &s = wsConnPtr->getContextRef<SubScriber>();
    _psService.unsubscribe(s.name, s.id);
}
