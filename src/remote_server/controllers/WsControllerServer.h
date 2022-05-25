#pragma once

#include <drogon/WebSocketController.h>
#include <drogon/utils/Utilities.h>
#include <drogon/PubSubService.h>
#include <drogon/HttpAppFramework.h>

using namespace drogon;

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
};

struct SubScriber
{
    std::string name;
    drogon::SubscriberID id;
};
