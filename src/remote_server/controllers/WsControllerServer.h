#pragma once

#include <drogon/WebSocketController.h>
#include <drogon/utils/Utilities.h>
#include <drogon/PubSubService.h>
#include <drogon/HttpAppFramework.h>

using namespace drogon;

struct SubScriber
{
    std::string name;
    drogon::SubscriberID id;
};

struct SensorStateTemporaryJson
{
    std::string contorlMes;
    std::string thresholdMes;
};

// identity 5
struct Latest24HourSensorData
{
    int hour_step_before;
    std::string dataListStringJson;
    // float temperature[24];
    // float humidity[24];
    // float pressure[24];
    // float sample[24];
};

class WsControllerServer : public drogon::WebSocketController<WsControllerServer>
{
  public:
    virtual void handleNewMessage(const WebSocketConnectionPtr&,
                                  std::string &&,
                                  const WebSocketMessageType &) override;
    virtual void handleNewConnection(const HttpRequestPtr &,
                                     const WebSocketConnectionPtr&) override;
    virtual void handleConnectionClosed(const WebSocketConnectionPtr&) override;
    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/periodicDataUpload");
    WS_PATH_LIST_END

  private:
    PubSubService<std::string> _psService;
    // TemporaryBuffer _tBuffer;
};
