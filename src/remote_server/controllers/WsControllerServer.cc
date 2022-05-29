#include "WsControllerServer.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <iomanip>
#include <ctime>
#include <chrono>
#include <string>

static const std::string RESPONSE_JSON = "{\"identity\": -1}";
static Latest24HourSensorData cache_data_g = {-1, {}};
static SensorStateTemporaryJson sensor_sate_g = {"", ""};

void _execCacheDataListHandler() {
    orm::DbClientPtr psqlClient = app().getDbClient();

    psqlClient->execSqlAsync("select temperature, humidity, pressure, sample from sensor_data_per_hours order by hour_point desc limit 24", 
            [](const drogon::orm::Result &result) {
                std::cout << "ooooooooooooooooooooooooooooooooooooook" << std::endl;
                int i = 0;
                int size = result.size();
                Json::Value root;
                Json::StreamWriterBuilder writerBuilder;

                for (auto row : result) {
                    // cache_data_g.temperature[23-i] = 
                    root["temperatureList"].append(row["temperature"].as<std::string>());
                    root["humidityList"].append(row["humidity"].as<std::string>());
                    root["pressureList"].append(row["pressure"].as<std::string>());
                    root["sampleList"].append(row["sample"].as<std::string>());
                    i++;
                }

                while (i++ < 24) {
                    root["temperatureList"].append("0.0");
                    root["humidityList"].append("0.0");
                    root["pressureList"].append("0.0");
                    root["sampleList"].append("0");
                }

                root["identity"] = 5;

                cache_data_g.dataListStringJson = std::move(Json::writeString(writerBuilder, root));
            },
            [](const drogon::orm::DrogonDbException &e) {
                std::cerr << "query data error:" << e.base().what() << std::endl;
            });
}

void _execSensorInsertHandler(float temperature, float humidity, float pressure, int sample) {
    orm::DbClientPtr psqlClient = app().getDbClient();

    // FIXME: logic should complected in SQL
    std::time_t tc = std::time(nullptr);
    char mbstr[11];
    int n_hour = -1;
    if (std::strftime(mbstr, sizeof(mbstr), "%Y%m%d%H", std::localtime(&tc))) {
        n_hour = atoi(mbstr);
        if (n_hour == -1) return;
    }

    std::cout << "currentTime: " << n_hour << ", latestTime: " << cache_data_g.hour_step_before << std::endl;

    if (cache_data_g.hour_step_before != -1) {
        if (n_hour > cache_data_g.hour_step_before) { // time step

            // data calc and move completed
            psqlClient->execSqlAsync("insert into sensor_data_per_hours(temperature, humidity, pressure, sample, hour_point) select avg(old.temperature), avg(old.humidity), avg(old.pressure), avg(old.sample), current_timestamp at time zone 'utc-8' from sensor_data old", 
                    [](const drogon::orm::Result &result) {
                        std::cout << "data moveda: " << result.size() <<  " executed !!!" << std::endl;
                    },
                    [](const drogon::orm::DrogonDbException &e) {
                        std::cerr << "truncate sensor_data error:" << e.base().what() << std::endl;
                    });

            psqlClient->execSqlAsync("truncate sensor_data restart identity", 
                    [](const drogon::orm::Result &result) {
                        std::cout << "truncate sensor_data: " << result.size() <<  " executed !!!" << std::endl;
                    },
                    [](const drogon::orm::DrogonDbException &e) {
                        std::cerr << "calc data insert to sensor_data_per_hours error:" << e.base().what() << std::endl;
                    });

            _execCacheDataListHandler();
            cache_data_g.hour_step_before = n_hour;
        }
        // pass
    } else {
        _execCacheDataListHandler();
        auto result = psqlClient->execSqlSync("select to_char(point, 'yyyymmddhh24') as latest_time from sensor_data order by point desc limit 1");

        for (auto row : result) {
            cache_data_g.hour_step_before = stoi(row["latest_time"].as<std::string>());
        }
    }

    psqlClient->execSqlAsync("insert into sensor_data(temperature, humidity, pressure, sample, point) values($1, $2, $3, $4, current_timestamp at time zone 'utf-8')",
            [](const drogon::orm::Result &result) {
                std::cout << "insert sensor data: " << result.size() <<  " executed !!!" << std::endl;
            },
            [](const drogon::orm::DrogonDbException &e) {
                std::cerr << "sensor data insert error:" << e.base().what() << std::endl;
            },
            std::to_string(temperature), std::to_string(humidity), std::to_string(pressure), std::to_string(sample));
}

void WsControllerServer::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    LOG_DEBUG << "from per mes: " << message;

    if (type == WebSocketMessageType::Ping) {
        LOG_DEBUG << "recv a ping";
    } else if (type == WebSocketMessageType::Text) {
        if (message == "keep alive") {
            wsConnPtr->send("ok");
            return;
        }
        bool res;
        JSONCPP_STRING errs;
        Json::Value root;
        Json::Reader reader;
        Json::CharReaderBuilder readerBuilder;
        Json::StreamWriterBuilder writerBuilder;

        std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());

        res = jsonReader->parse(message.c_str(), message.c_str()+message.length(), &root, &errs);
        if (!res || !errs.empty()) {
            std::cout << "parseJson error: " << errs << std::endl;
            wsConnPtr->send("json parse error");
            return;
        }

        const Json::Value identity = root["identity"];
        if (!identity) {
            std::cout << "identity error: " << errs << std::endl;
            wsConnPtr->send("identity error");
            return;
        }

        auto &s = wsConnPtr->getContextRef<SubScriber>();

        int id = identity.asInt();
        if (id == 1) { // from gateway, hand out data
            const Json::Value sensor = root["sensor"];
            const Json::Value control = root["control"];
            const Json::Value threshold = root["threshold"];

            // merge and publish
            if (sensor) {
                float temp = sensor["temperature"].asFloat();
                float humi = sensor["humidity"].asFloat();
                float pres = sensor["pressure"].asFloat();
                int sample = sensor["sample"].asInt();
                _execSensorInsertHandler(temp, humi, pres, sample);

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
        } else if (id == 5) { // 24 hour data from sensor_data_per_hours tables
            if (cache_data_g.hour_step_before != -1)
                wsConnPtr->send(cache_data_g.dataListStringJson);
            else {
                _execCacheDataListHandler();
                wsConnPtr->send(cache_data_g.dataListStringJson);
            }

            LOG_DEBUG << "latest 24 hour data get";
        } else if (id == 0) { // keep alive
            LOG_DEBUG << "[keep alive]";
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
