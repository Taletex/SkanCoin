#include "HttpServer/HttpServer.cpp"
#include "P2PServer/P2PServer.cpp"
#include "Blockchain/Wallet.cpp"
#include "Blockchain/Blockchain.cpp"
#include "Blockchain/Transaction.cpp"
#include "Blockchain/TransactionPool.cpp"
#include <iostream>
#include <thread>

using namespace std;


int main(){
  //thread httpServer (initHttpServer);
  //thread p2pServer (initP2PServer);
  //initWallet();
  list<Block> chain = getBlockchain();
  list<Block>::iterator it;
  for(it = chain.begin(); it != chain.end(); ++it){
    cout << "Hash del primo blocco: " << it->hash << endl;
    cout << "ID della transazione: " << getTransactionId(it->data[0]) <<endl;
  }

  //httpServer.join();
  //p2pServer.join();

  return 0;
}
