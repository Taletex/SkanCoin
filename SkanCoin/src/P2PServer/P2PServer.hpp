#include <iostream>
#include <unordered_set>
#include <mutex>
#include "crow.h"

#ifndef __HTTP_SERVER_DEFINITION__
#define __HTTP_SERVER_DEFINITION__

enum MessageType {
    QUERY_LATEST = 0,
    QUERY_ALL = 1,
    RESPONSE_BLOCKCHAIN = 2,
    QUERY_TRANSACTION_POOL = 3,
    RESPONSE_TRANSACTION_POOL = 4
};

class Message {
    MessageType type;
    std::string data;
};

class P2PServer {
  public:

    static P2PServer& getInstance() {
       static P2PServer peer;
       return peer;
    }

    std::mutex mtx;
    std::unordered_set<crow::websocket::connection*> users;

    void initP2PServer();

    void connectToPeers(std::string peer);

    std::string printPeers();

  private:
    P2PServer(){}
    P2PServer(const P2PServer&) = delete;
    P2PServer& operator=(const P2PServer&) = delete;
};
#endif
