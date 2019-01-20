#include "HttpServer/HttpServer.cpp"
#include "P2PServer/P2PServer.cpp"
#include "Blockchain/Transactions.hpp"
#include "Blockchain/TransactionPool.hpp"
#include "Blockchain/Blockchain.hpp"
#include "Blockchain/Wallet.hpp"
#include <iostream>
#include <thread>

using namespace std;


int main(){
  //thread httpServer (initHttpServer);
  //thread p2pServer (initP2PServer);
  //initWallet();
  /*BlockChain blockchain;
  try{
    blockchain = BlockChain();
  }catch(const char* msg){
    cout << msg << endl;
    return 0;
  }
  list<Block> chain = blockchain.getBlockchain();
  list<Block>::iterator it;
  for(it = chain.begin(); it != chain.end(); ++it){
    cout << "Hash del primo blocco: " << it->hash << endl;
    cout << "ID della transazione: " << getTransactionId(it->data[0]) <<endl;
  }*/

  //httpServer.join();
  //p2pServer.join();

  return 0;
}
