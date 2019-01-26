#ifndef __P2P_SERVER_DEFINITION__
#define __P2P_SERVER_DEFINITION__

#include <iostream>
#include <unordered_set>
#include <mutex>
#include "document.h"
#include "../HttpServer/HttpServer.hpp"
#include "easywsclient.hpp"
#include <assert.h>
#include <chrono>
#include <thread>

enum MessageType {
    QUERY_LATEST = 0,
    QUERY_ALL = 1,
    RESPONSE_BLOCKCHAIN = 2,
    QUERY_TRANSACTION_POOL = 3,
    RESPONSE_TRANSACTION_POOL = 4
};

class Message {
  public:
    MessageType type;
    std::string data;

    Message(MessageType type, std::string data);

    std::string toString();
};

class P2PServer {
  public:
    //I due thread (client e server) devono agire uno alla volta, in quanto non è possibile accedere alle istanze delle WS da thread diversi contemporaneamente
    std::mutex ConnectionsMtx;

    std::unordered_set<crow::websocket::connection*> receivedConnections;
    std::vector<easywsclient::WebSocket::pointer> openedConnections;
    easywsclient::WebSocket::pointer tempWs;
    static P2PServer& getInstance() {
       static P2PServer peer;
       return peer;
    }

    void initP2PServer(int port);
    void clearClosedWs();
    void broadcast(std::string message);
    void broadCastTransactionPool();
    void broadcastLatest();
    void startClientPoll();
    int countPeers();

    void checkReceivedMessage();
    //business logic, questa funzione è chiamata nell'handler dei messaggi in arrivo//
    void handleBlockchainResponse(std::list<Block> receivedBlocks);
    //metodi per la costruzione dei messaggi//
    std::string queryChainLengthMsg();
    std::string queryAllMsg();
    std::string queryTransactionPoolMsg();
    std::string responseChainMsg();
    std::string responseLatestMsg();
    std::string responseTransactionPoolMsg();
    void connectToPeers(std::string peer);
    bool isValidType(int type);
    void messageHandler(crow::websocket::connection& connection, const std::string& data);

  private:
    P2PServer(){}
    P2PServer(const P2PServer&) = delete;
    P2PServer& operator=(const P2PServer&) = delete;
};

void handleClientMessage(const std::string & message);
#endif
