#include "WebSocketServer.h"

void WebSocketServer::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    if (wsConnPtr->connected()) {
        LOG_DEBUG << "ws mess:" << message;
        wsConnPtr->send(message + ", Ack!!!");
    }
}

void WebSocketServer::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    LOG_DEBUG << "ws conn!";
}

void WebSocketServer::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    LOG_DEBUG << "ws closed!";
}
