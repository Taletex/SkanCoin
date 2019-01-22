#include "HttpServer/HttpServer.cpp"
#include "P2PServer/P2PServer.cpp"
#include "Blockchain/Transactions.hpp"
#include "Blockchain/Transactions.cpp"
#include "Blockchain/TransactionPool.hpp"
#include "Blockchain/TransactionPool.cpp"
#include "Blockchain/Blockchain.hpp"
#include "Blockchain/Blockchain.cpp"
#include "Blockchain/Wallet.hpp"
#include "Blockchain/Wallet.cpp"
#include <iostream>
#include <thread>

using namespace std;


int main(){
  cout <<  BlockChain::getInstance().toString();
thread httpServer (initHttpServer);
  //thread p2pServer (initP2PServer);
  //initWallet();
  // BlockChain blockchain;
  // list<Block> chain = blockchain.getBlockchain();
  // list<Block>::iterator it;
  // for(it = chain.begin(); it != chain.end(); ++it){
  //   cout << "Hash del primo blocco: " << it->hash << endl;
  //   cout << "ID della transazione: " << getTransactionId(it->data[0]) <<endl;
  // }

  httpServer.join();
  //p2pServer.join();

  return 0;
}
