#include "Peer.hpp"

using namespace std;
using easywsclient::WebSocket;

//Handler for oncoming message from socket opened by the client thread (easywsclient::WebSocket)
//This method must have a single parameter (const string & data), this is why we need a support variable to reference the current socket (tempWs)
void handleClientMessage(const string & data){
  //Parsing json object (stringified Message Object) received from the websocket
  rapidjson::Document document;
  document.Parse(data.c_str());
  const rapidjson::Value& type = document["type"];

  //Check if received message is valid
  if(document["type"].IsNull() || Peer::getInstance().isValidType(type.GetInt()) == false){
    cout << "Error parsing data: Received message type is not valid" << endl;
    return;
  }
  //Map json object to a new istance of Message
  Message message = Message(static_cast<MessageType>(document["type"].GetInt()), document["data"].GetString());
  cout << "Received message: " << message.toString() << endl;

  //Lists used for some type of oncoming messages
  list<Block> receivedBlocks;
  vector<Transaction> receivedTransactions;

  //This switch contains the business logic for messages handling, just invoke correct methods, no implementation here
  switch (message.type) {
    //Some peer sent a query message for last block, send it
    case QUERY_LATEST:
      Peer::getInstance().tempWs->send(Peer::getInstance().responseLatestMsg());
      break;

    //Some peer sent a query for entire blockchain, send it
    case QUERY_ALL:
      Peer::getInstance().tempWs->send(Peer::getInstance().responseChainMsg());
      break;

    //Some peer sent his version of blockchain, parse it and call secific method to handle this event
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
      Peer::getInstance().handleBlockchainResponse(receivedBlocks);
      break;

    //Some peer sent a query for transaction pool, send it
    case QUERY_TRANSACTION_POOL:
      Peer::getInstance().tempWs->send(Peer::getInstance().responseTransactionPoolMsg());
      break;

    //Some peer sent his version of transaction pool
    case RESPONSE_TRANSACTION_POOL:
    //Parsing received transaction pool
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
      //For each transaction, Blockchain method to elaborate it is called, then the updated transaction pool is broadcasted
      vector<Transaction>::iterator it;
      for(it = receivedTransactions.begin(); it != receivedTransactions.end(); ++it){
        try{
            BlockChain::getInstance().handleReceivedTransaction(*it);
            Peer::getInstance().broadCastTransactionPool();
        }catch(const char* msg) {
            cout << msg << endl;
            cout << "EXCEPITON: Error inserting new transaction in transactionPool..." << endl;
        }
      }
      break;

    default:
      cout << "Received message has no valid message type" << endl;
      break;
  }
}

//MESSAGE METHODS//
Message::Message(MessageType type, string data){
  this->type = type;
  this->data = data;
}
//Print json representation of Message Object
string Message::toString(){
  return "{\"type\": " + to_string(type) + ", \"data\": \"" + data + "\"}";
}

//PEER METHODS//

//This function executes the polling over the list of opened connections, if some message has been received, the handler function for it is executed
void Peer::checkReceivedMessage(){
  //Flag to notify that some socket has been closed, we cannot remove it in the same loop because we need direct reference to the easywsclient::WebSocket in the firts, and an iterator in the second
  bool flag = false;

  //Lock mutex to access to the list of connections
  std::lock_guard<std::mutex> _(ConnectionsMtx);

  //Look for oncoming messages on each socket
  for(auto ws: openedConnections){
    //Check if socket has been closed
    if(ws->getReadyState() != WebSocket::CLOSED) {
      //If some message has been received, the buffer used by the handler function is fille here
      ws->poll();
      //Assign support reference to the current socket, so we can use it in the handler function
      tempWs = ws;
      //Executes the handler function for the received message
      ws->dispatch(handleClientMessage);
    }else{
      //The socket has been closed
      flag = true;
    }
  }
  //removing closed sockets (can't use iterator or erase on references vector)
  if(flag == true){
    clearClosedWs();
  }
}

void Peer::clearClosedWs(){
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
void Peer::connectToPeers(std::string peer){
  cout << "Adding " << peer << " to the list of peers..." << endl;
  WebSocket::pointer ws;
  ws = WebSocket::from_url(peer);
  assert(ws);
  std::lock_guard<std::mutex> _(ConnectionsMtx);
  ws->send(queryChainLengthMsg());
  openedConnections.push_back(ws);
  broadcast(queryTransactionPoolMsg());
}
int Peer::countPeers(){
  return receivedConnections.size() + openedConnections.size();
}
//business logic, questa funzione è chiamata nell'handler dei messaggi in arrivo//
void Peer::handleBlockchainResponse(list<Block> receivedBlocks){
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
void Peer::initP2PServer(int port){
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
void Peer::startClientPoll(){
  cout << "Starting P2P client...." << endl;
  while(true){
    while(openedConnections.size() == 0){
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    checkReceivedMessage();
  }
}
void Peer::messageHandler(crow::websocket::connection& connection, const string& data){
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
bool Peer::isValidType(int type){
  MessageType receivedType = static_cast<MessageType>(type);
  for ( int val = QUERY_LATEST; val != RESPONSE_TRANSACTION_POOL; val++ ){
    if(receivedType == val)return true;
  }
  return false;
}


//metodi per la costruzione dei messaggi//
void Peer::broadcast(string message){
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
void Peer::broadCastTransactionPool(){
  broadcast(responseTransactionPoolMsg());
}
void Peer::broadcastLatest(){
  broadcast(responseLatestMsg());
}
string Peer::queryChainLengthMsg(){
  return Message(QUERY_LATEST, "").toString();
}
string Peer::queryAllMsg(){
  return Message(QUERY_ALL, "").toString();
}
string Peer::queryTransactionPoolMsg(){
  return Message(QUERY_TRANSACTION_POOL, "").toString();
}
string Peer::responseChainMsg(){
  return Message(RESPONSE_BLOCKCHAIN, BlockChain::getInstance().toString()).toString();
}
string Peer::responseLatestMsg(){
  return Message(RESPONSE_BLOCKCHAIN, "[" + BlockChain::getInstance().getLatestBlock().toString() + "]").toString();
}
string Peer::responseTransactionPoolMsg(){
  return Message(RESPONSE_TRANSACTION_POOL, TransactionPool::getInstance().toString()).toString();
}
