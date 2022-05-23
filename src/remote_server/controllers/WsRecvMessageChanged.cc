#include "WsRecvMessageChanged.h"
#include <chrono>

void WsRecvMessageChanged::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    for (;;) {
        wsConnPtr->send("test");
        std::chrono::seconds sec(1);
    }
}

void WsRecvMessageChanged::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    LOG_DEBUG << "Data change channel connection..";
}

void WsRecvMessageChanged::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    LOG_DEBUG << "Data change channel closed..";
}
