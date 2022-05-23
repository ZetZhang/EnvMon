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
        // Json::Value root;
        // Json::Reader reader;
        // bool parsingSucc = reader.parse(message, root);
        // const Json::Value identity = root["identity"];
        // if (!identity)
        //     return;
        // int id = identity.asInt();

        // if (id == 1) {
        //     // redis
        //     // postgresql
        //     // control publish
        //     const Json::Value control = root["control"];
        //     int controlValue = 0;
        //     if (control)
        //         controlValue = control.asInt();
        //     // threshold publish
        //     const Json::Value threshold = root["threshold"];
        //     int thresholdList = 0;
        //     // if (threshold)
        //         // thresholdList = control.as<typename T>(); // byte:14 length buffer

        //     // merge and publish

        //     // response
        //     wsConnPtr->send('ok');
        // } else if (id == 2) {
        //     // wsConnPtr->send("fuck");
        //     const Json::Value 
        //     auto &s = wsConnPtr->getContextRef<SubScriber>();
        //     _psService.publish(s.name, message);
        // }
        auto &s = wsConnPtr->getContextRef<SubScriber>();
        _psService.publish(s.name, message);
    }
    wsConnPtr->send("response");
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
                                     // wsConnPtr->send(message);
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
