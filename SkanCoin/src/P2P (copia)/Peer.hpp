#ifndef __P2P_SERVER_DEFINITION__
#define __P2P_SERVER_DEFINITION__
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_set>
#include "../HttpServer/HttpServer.hpp"
#include "document.h"
#include "easywsclient.hpp"

//Possible types of messages exchanged between the peers
enum MessageType {
    QUERY_LATEST = 0,
    QUERY_ALL = 1,
    RESPONSE_BLOCKCHAIN = 2,
    QUERY_TRANSACTION_POOL = 3,
    RESPONSE_TRANSACTION_POOL = 4,
    TRANSACTION_POOL_STATS = 5
};

//This class models a peer message and contains a toString method used to send it in a websocket
class Message {
  public:
    MessageType type;
    std::string data;
    std::string stat;
    Message(MessageType type, std::string data);
    Message(MessageType type, std::string data, std::string stat);
    std::string toString();
};

//This is a Singleton class, his role is to manage the connections the other peers of the network handling oncoming messasges and user operation (made through the http interface)
class Peer {
  public:
    //We need a mutex because the lists used to manage incoming and opened connection cannot be used by multiple threads at the same time (and we need to use different threads to manage this 2 tipes of connections)
    std::mutex ConnectionsMtx;
    //List of connection received by the server thread
    std::unordered_set<crow::websocket::connection*> receivedConnections;
    //List of connection opened by the client thread
    std::vector<easywsclient::WebSocket::pointer> openedConnections;
    //This is a support reference used by he handler of incoming message to hande an event for a specific socket
    easywsclient::WebSocket::pointer tempWs;

    static Peer& getInstance() {
       static Peer peer;
       return peer;
    }
    bool isValidType(int type);
    int countPeers();
    std::string queryChainLengthMsg();
    std::string queryAllMsg();
    std::string queryTransactionPoolMsg();
    std::string responseChainMsg();
    std::string responseLatestMsg(std::string stat);
    std::string responseTransactionPoolMsg();
    std::string txPoolStatsMessage(std::vector<std::string> stats);
    void initP2PServer(int port);
    void clearClosedWs();
    void broadcast(std::string message);
    void broadCastTransactionPool();
    void broadcastLatest(std::string stat);
    void startClientPoll();
    void messageHandler(crow::websocket::connection& connection, const std::string& data);
    void checkReceivedMessage();
    void handleBlockchainResponse(std::list<Block> receivedBlocks);
    void connectToPeers(std::string peer);
    void broadcastTxPoolStat(std::vector<std::string>);

  private:
    //We declare this methods as private to implement Singleton pattern
    Peer(){}
    Peer(const Peer&) = delete;
    Peer& operator=(const Peer&) = delete;
};

//This function cannot be a member of a class because it's used as handler for incoming messages from the sockets opened by the client thread
void handleClientMessage(const std::string & message);
#endif
