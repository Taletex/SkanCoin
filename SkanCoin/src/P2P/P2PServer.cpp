#include "P2PServer.hpp"

using namespace std;
using easywsclient::WebSocket;

Message::Message(MessageType type, string data){
  this->type = type;
  this->data = data;
}

string Message::toString(){
  return "{\"type\": " + to_string(type) + ", \"data\": \"" + data + "\"}";
}

void P2PServer::startClientPoll(){
  cout << "Starting P2P client...." << endl;
  while(true){
    while(openedConnections.size() == 0){
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    checkReceivedMessage();
  }
}

void P2PServer::checkReceivedMessage(){
  bool flag = false;
  std::lock_guard<std::mutex> _(ConnectionsMtx);
  for(auto ws: openedConnections){
    if(ws->getReadyState() != WebSocket::CLOSED) {
      ws->poll();
      tempWs = ws;
      ws->dispatch(handleClientMessage);
    }else{
      flag = true;
    }
  }
  //removing closed sockets (can't use iterator or erase on references vector)
  if(flag == true){
    clearClosedWs();
  }

}

void P2PServer::clearClosedWs(){
  vector<easywsclient::WebSocket::pointer> temp;
  for(auto ws: openedConnections){
    if(ws->getReadyState() != WebSocket::CLOSED) {
      temp.push_back(ws);
    }
  }
  openedConnections.clear();
  openedConnections.reserve(openedConnections.size() + temp.size());
  openedConnections.insert(openedConnections.end(), temp.begin(), temp.end());
}

void P2PServer::connectToPeers(std::string peer){
  cout << "Adding " << peer << " to the list of peers..." << endl;
  WebSocket::pointer ws;
  ws = WebSocket::from_url(peer);
  assert(ws);
  std::lock_guard<std::mutex> _(ConnectionsMtx);
  ws->send(queryChainLengthMsg());
  openedConnections.push_back(ws);
  broadcast(queryTransactionPoolMsg());
}

void P2PServer::initP2PServer(int port){
  crow::SimpleApp app;
  CROW_ROUTE(app, "/").websocket()
    .onopen([&](crow::websocket::connection& connection){
      CROW_LOG_INFO << "new websocket connection";
      std::lock_guard<std::mutex> _(ConnectionsMtx);
      receivedConnections.insert(&connection);
      connection.send_text(queryChainLengthMsg());
      broadcast(queryTransactionPoolMsg());
    })
    .onclose([&](crow::websocket::connection& connection, const std::string& reason){
      CROW_LOG_INFO << "websocket connection closed: " << reason;
      std::lock_guard<std::mutex> _(ConnectionsMtx);
      receivedConnections.erase(&connection);
    })
    .onmessage([&](crow::websocket::connection& connection, const std::string& data, bool){
      std::lock_guard<std::mutex> _(ConnectionsMtx);
      messageHandler(connection, data);
    });

  cout << "Starting P2PServer on port " << port << "..." << endl;
  app.port(port).run();
}

int P2PServer::countPeers(){
  return receivedConnections.size() + openedConnections.size();
}
//business logic, questa funzione è chiamata nell'handler dei messaggi in arrivo//
void P2PServer::handleBlockchainResponse(list<Block> receivedBlocks){
    if (receivedBlocks.size() == 0) {
        cout << "Received block chain size of 0" << endl;
        return;
    }
    Block latestBlockReceived = receivedBlocks.back();
    if (BlockChain::getInstance().isValidBlockStructure(latestBlockReceived) == false) {
        cout << "Block structuture not valid" << endl;
        return;
    }
    Block latestBlockHeld = BlockChain::getInstance().getLatestBlock();
    if (latestBlockReceived.index > latestBlockHeld.index) {
        cout << "Blockchain possibly behind. We got: " << latestBlockHeld.index << " Peer got: " << latestBlockReceived.index << endl;
        if (latestBlockHeld.hash == latestBlockReceived.previousHash) {
            if (BlockChain::getInstance().addBlockToChain(latestBlockReceived)) {
                broadcast(responseLatestMsg());
            }
        } else if (receivedBlocks.size() == 1) {
            cout << "We have to query the chain from our peer" << endl;
            broadcast(queryAllMsg());
        } else {
            cout << "Received blockchain is longer than current blockchain" << endl;
            BlockChain::getInstance().replaceChain(receivedBlocks);
        }
    } else {
        cout << "Received blockchain is not longer than received blockchain. Do nothing" << endl;
    }
}
//metodi per la costruzione dei messaggi//
string P2PServer::queryChainLengthMsg(){
  return Message(QUERY_LATEST, "").toString();
}
string P2PServer::queryAllMsg(){
  return Message(QUERY_ALL, "").toString();
}
string P2PServer::queryTransactionPoolMsg(){
  return Message(QUERY_TRANSACTION_POOL, "").toString();
}
string P2PServer::responseChainMsg(){
  return Message(RESPONSE_BLOCKCHAIN, BlockChain::getInstance().toString()).toString();
}
string P2PServer::responseLatestMsg(){
  return Message(RESPONSE_BLOCKCHAIN, "[" + BlockChain::getInstance().getLatestBlock().toString() + "]").toString();
}
string P2PServer::responseTransactionPoolMsg(){
  return Message(RESPONSE_TRANSACTION_POOL, TransactionPool::getInstance().toString()).toString();
}

bool P2PServer::isValidType(int type){
  MessageType receivedType = static_cast<MessageType>(type);
  for ( int val = QUERY_LATEST; val != RESPONSE_TRANSACTION_POOL; val++ ){
    if(receivedType == val)return true;
  }
  return false;
}

void P2PServer::messageHandler(crow::websocket::connection& connection, const string& data){
  rapidjson::Document document;
  document.Parse(data.c_str());
  const rapidjson::Value& type = document["type"];
  if(document["type"].IsNull() || isValidType(type.GetInt()) == false){
    cout << "Error parsing data: Received message type is not valid" << endl;
    return;
  }
  Message message = Message(static_cast<MessageType>(document["type"].GetInt()), document["data"].GetString());

  cout << "Received message: " << message.toString() << endl;
  list<Block> receivedBlocks;
  vector<Transaction> receivedTransactions;
  switch (message.type) {
    case QUERY_LATEST:
      connection.send_text(responseLatestMsg());
      break;
    case QUERY_ALL:
      connection.send_text(responseChainMsg());
      break;
    case RESPONSE_BLOCKCHAIN:
      if(document["data"].IsNull()){
        cout << "Error parsing data: No data received" << endl;
        return;
      }
      try{
        receivedBlocks = parseBlockList(document["data"]);
      }catch(const char* msg){
        cout << msg << endl;
        cout << "EXCEPTION: Error parsing message!" << endl;
        return;
      }
      handleBlockchainResponse(receivedBlocks);
      break;
    case QUERY_TRANSACTION_POOL:
      connection.send_text(responseTransactionPoolMsg());
      break;
    case RESPONSE_TRANSACTION_POOL:
      if(document["data"].IsNull()){
        cout << "Error parsing data: No data received" << endl;
        return;
      }
      try{
        receivedTransactions = parseTransactionVector(document["data"]);
      }catch(const char* msg){
        cout << msg << endl;
        cout << "EXCEPTION: Error parsing message!" << endl;
        return;
      }
      vector<Transaction>::iterator it;
      for(it = receivedTransactions.begin(); it != receivedTransactions.end(); ++it){
        try{
            BlockChain::getInstance().handleReceivedTransaction(*it);
            broadCastTransactionPool();
        }catch(const char* msg) {
            cout << msg << endl;
            cout << "EXCEPITON: Error inserting new transaction in transactionPool..." << endl;
        }
      }
      break;
  }
}

//La funzione non può essere un metodo di una classe in quanto è usata come callback per la ricezione dei messaggi di wsclient
void handleClientMessage(const string & data){
  rapidjson::Document document;
  document.Parse(data.c_str());
  const rapidjson::Value& type = document["type"];
  if(document["type"].IsNull() || P2PServer::getInstance().isValidType(type.GetInt()) == false){
    cout << "Error parsing data: Received message type is not valid" << endl;
    return;
  }
  Message message = Message(static_cast<MessageType>(document["type"].GetInt()), document["data"].GetString());

  cout << "Received message: " << message.toString() << endl;
  list<Block> receivedBlocks;
  vector<Transaction> receivedTransactions;
  switch (message.type) {
    case QUERY_LATEST:
      P2PServer::getInstance().tempWs->send(P2PServer::getInstance().responseLatestMsg());
      break;
    case QUERY_ALL:
      P2PServer::getInstance().tempWs->send(P2PServer::getInstance().responseChainMsg());
      break;
    case RESPONSE_BLOCKCHAIN:
      if(document["data"].IsNull()){
        cout << "Error parsing data: No data received" << endl;
        return;
      }
      try{
        receivedBlocks = parseBlockList(document["data"]);
      }catch(const char* msg){
        cout << msg << endl;
        cout << "EXCEPTION: Error parsing message!" << endl;
        return;
      }
      P2PServer::getInstance().handleBlockchainResponse(receivedBlocks);
      break;
    case QUERY_TRANSACTION_POOL:
      P2PServer::getInstance().tempWs->send(P2PServer::getInstance().responseTransactionPoolMsg());
      break;
    case RESPONSE_TRANSACTION_POOL:
      if(document["data"].IsNull()){
        cout << "Error parsing data: No data received" << endl;
        return;
      }
      try{
        receivedTransactions = parseTransactionVector(document["data"]);
      }catch(const char* msg){
        cout << msg << endl;
        cout << "EXCEPTION: Error parsing message!" << endl;
        return;
      }
      vector<Transaction>::iterator it;
      for(it = receivedTransactions.begin(); it != receivedTransactions.end(); ++it){
        try{
            BlockChain::getInstance().handleReceivedTransaction(*it);
            P2PServer::getInstance().broadCastTransactionPool();
        }catch(const char* msg) {
            cout << msg << endl;
            cout << "EXCEPITON: Error inserting new transaction in transactionPool..." << endl;
        }
      }
      break;
  }
}

void P2PServer::broadcast(string message){
  //broadcasting to opened sockets
  for(auto ws: openedConnections){
    if(ws->getReadyState() != WebSocket::CLOSED) {
      ws->send(message);
    }
  }

  //broadcasting to received Connections
  for(auto ws: receivedConnections){
    ws->send_text(message);
  }
}
void P2PServer::broadCastTransactionPool(){
  broadcast(responseTransactionPoolMsg());
}
void P2PServer::broadcastLatest(){
  broadcast(responseLatestMsg());
}
