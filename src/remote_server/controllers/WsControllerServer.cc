#include "WsControllerServer.h"
#include <drogon/utils/Utilities.h>

void WsControllerServer::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    LOG_DEBUG << "from per mes: " << message;
    wsConnPtr->send(message);
}

void WsControllerServer::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    LOG_DEBUG << "New connection..";
}

void WsControllerServer::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    LOG_DEBUG << "Clonsed connection..";
}
