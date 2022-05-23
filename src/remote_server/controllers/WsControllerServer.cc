#include "WsControllerServer.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

void WsControllerServer::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    Json::Value ret;
    LOG_DEBUG << "from per mes: " << message;
    if (type == WebSocketMessageType::Ping) {
        LOG_DEBUG << "recv a ping";
    } else if (type == WebSocketMessageType::Text) {
        wsConnPtr->send(message);
        // auto &s = wsConnPtr->getContextRef<SubScriber>();
        // _psService.publish(s.name, message);
    }
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
                                     (void)message;
                                 });
    wsConnPtr->setContext(std::make_shared<SubScriber>(std::move(s)));
}

void WsControllerServer::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    LOG_DEBUG << "websocket closed connection..";
    auto &s = wsConnPtr->getContextRef<SubScriber>();
    _psService.unsubscribe(s.name, s.id);
}
