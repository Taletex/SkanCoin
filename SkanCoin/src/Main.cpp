//Standard libraries//
#include <iostream>
#include <thread>
//header files//
#include "Blockchain/Blockchain.hpp"
#include "Blockchain/Transactions.hpp"
#include "Blockchain/TransactionPool.hpp"
#include "Blockchain/Wallet.hpp"
#include "HttpServer/HttpServer.hpp"
#include "P2P/Peer.hpp"
#include "easywsclient.hpp"
#include "easywsclient.cpp"
#include "ecc.h"

int debug = 1;

//source files//
#include "Blockchain/TransactionPool.cpp"
#include "Blockchain/Transactions.cpp"
#include "Blockchain/Blockchain.cpp"
#include "HttpServer/HttpServer.cpp"
#include "Blockchain/Wallet.cpp"
#include "P2P/Peer.cpp"
#include "ecc.c"

using namespace std;

void initP2PServer(int port){
  Peer::getInstance().initP2PServer(port);
}

void startP2PClient(){
  Peer::getInstance().startClientPoll();
}

int main(){
  try{
    cout <<"Starting node..." << endl << "Generating wallet..." << endl;
    initWallet();
  }catch(const char* msg){
    cout << msg << endl;
    cout << "EXCEPTION: Wallet creation failed!" << endl;
    return 0;
  }
  cout << "Generating blockchain..." << endl;
  try{
    cout << "BLOCKCHAIN: " << endl <<  BlockChain::getInstance().toString() << endl;
  }catch(const char* msg){
    cout << msg << endl;
    cout << "EXCEPTION: Blockchain generation failed!" << endl;
    return 0;
  }

  thread httpServer (initHttpServer,3001);
  thread p2pServer (initP2PServer,6001);
  thread p2pClient (startP2PClient);
  httpServer.join();
  p2pServer.join();
  p2pClient.join();
  return 0;
}
