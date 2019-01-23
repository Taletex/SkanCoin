//header files//
#include "Blockchain/Blockchain.hpp"
#include "Blockchain/Transactions.hpp"
#include "Blockchain/TransactionPool.hpp"
#include "Blockchain/Wallet.hpp"
#include "HttpServer/HttpServer.hpp"
#include "P2PServer/P2PServer.hpp"

//source files//
#include "Blockchain/TransactionPool.cpp"
#include "Blockchain/Transactions.cpp"
#include "Blockchain/Blockchain.cpp"
#include "HttpServer/HttpServer.cpp"
#include "Blockchain/Wallet.cpp"
#include "P2PServer/P2PServer.cpp"
#include <iostream>
#include <thread>

using namespace std;


int main(){
  cout <<"Starting node..." << endl << "Generating blockchain..." << endl;
  try{
    cout << "BLOCKCHAIN: " << endl <<  BlockChain::getInstance().toString() << endl;
  }catch(const char* msg){
    cout << msg << endl;
    cout << "EXCEPTION: Blockchain generation failed!" << endl;
    return 0;
  }
  initWallet();
  thread httpServer (initHttpServer);
  P2PServer::getInstance().initP2PServer();
  httpServer.join();

  return 0;
}
