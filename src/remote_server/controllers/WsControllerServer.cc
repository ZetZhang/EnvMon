#include "WsControllerServer.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

static const std::string RESPONSE_JSON = "{\"identity\": -1}";
static Latest24HourSensorData cache_data_g = {-1, {0}, {0}, {0}, {0}};
static SensorStateTemporaryJson sensor_sate_g = {"", ""};

void execSensorInsertHandler(std::string message) {
    // Json::Value root;
    // Json::Reader reader;
    // bool parsingSucc = reader.parse(message, root);
    // Json::Value sensor = root["sensor"];
    // const Json::Value sensor

    // orm::DbClientPtr psqlClient = app().getDbClient();
    // psqlClient->execSqlAsync("insert into sensor_data(temperature, humidity, pressure, sample, point) values($1, $2, $3, $4, current_timestamp)",
    //         [](const drogon::orm::Result &result) {

    //         },
    //         [](const drogon::orm::DrogonDbException &e) {
    //             std::cerr << "sensor data insert error:" << e.base().what() << std::endl;
    //         },
    //         1, 2, 3, 4);
}

void WsControllerServer::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    LOG_DEBUG << "from per mes: " << message;

    if (type == WebSocketMessageType::Ping) {
        LOG_DEBUG << "recv a ping";
    } else if (type == WebSocketMessageType::Text) {
        bool res;
        JSONCPP_STRING errs;
        Json::Value root;
        Json::Reader reader;
        Json::CharReaderBuilder readerBuilder;
        Json::StreamWriterBuilder writerBuilder;

        std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());

        res = jsonReader->parse(message.c_str(), message.c_str()+message.length(), &root, &errs);
        if (!res || !errs.empty())
            std::cout << "parseJson error: " << errs << std::endl;

        // bool parsingSucc = reader.parse(message, root);
        const Json::Value identity = root["identity"];
        if (!identity)
            return;

        auto &s = wsConnPtr->getContextRef<SubScriber>();

        int id = identity.asInt();
        if (id == 1) { // from gateway, hand out data
            const Json::Value sensor = root["sensor"];
            const Json::Value control = root["control"];
            const Json::Value threshold = root["threshold"];

            // merge and publish
            if (sensor) {
                execSensorInsertHandler(message);

                Json::Value newSensorValue;
                newSensorValue["identity"] = 2;
                newSensorValue["sensor"] = sensor;
                std::string document = Json::writeString(writerBuilder, newSensorValue);

                _psService.publish("users", document);
            }

            // control publish
            if (control) {
                Json::Value newControlValue;
                newControlValue["identity"] = 3;
                newControlValue["control"] = control;
                std::string document = Json::writeString(writerBuilder, newControlValue);

                sensor_sate_g.contorlMes = document;

                _psService.publish("users", document);
            }

            // threshold publish
            if (threshold) {
                Json::Value newThresholdValue;
                newThresholdValue["identity"] = 4;
                newThresholdValue["threshold"] = threshold;
                std::string document = Json::writeString(writerBuilder, newThresholdValue);
                sensor_sate_g.thresholdMes = document;

                _psService.publish("users", document);
            }

        } else if (id == 2) { // sensor ignore
            LOG_DEBUG << "error message";
        } else if (id == 3 || id == 4) { // control or threshold changed
            _psService.publish("users", message);
            _psService.publish("gateway", message);

            LOG_DEBUG << "control or threshold message";
        } else if (id == 5) {
            LOG_DEBUG << "reserve";
        } else if (id == 0) { // other
            _psService.publish("users", message);
            LOG_DEBUG << "[TEST]";
        }
    }
}

void WsControllerServer::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    LOG_DEBUG << "new websocket connection..";
    // send to control & threshold info
    if (sensor_sate_g.contorlMes != "")
        wsConnPtr->send(sensor_sate_g.contorlMes);
    if (sensor_sate_g.thresholdMes != "")
        wsConnPtr->send(sensor_sate_g.thresholdMes);

    SubScriber s;
    s.name= req->getParameter("who");
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
