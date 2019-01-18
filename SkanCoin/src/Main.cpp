#include "HttpServer/HttpServer.cpp"
#include "P2PServer/P2PServer.cpp"
#include "Blockchain/Wallet.cpp"
#include "Blockchain/Blockchain.cpp"
#include "Blockchain/Transaction.cpp"
#include "Blockchain/TransactionPool.cpp"
#include <iostream>
#include <thread>

std::string hexToBinaryLookupTable(char c){
  std::string ret= "";
  switch(c){
      case '0': ret = "0000";
      break;
      case '1': ret = "0001";
      break;
      case '2': ret = "0010";
      break;
      case '3': ret = "0011";
      break;
      case '4': ret = "0100";
      break;
      case '5': ret = "0101";
      break;
      case '6': ret = "0110";
      break;
      case '7': ret = "0111";
      break;
      case '8': ret = "1000";
      break;
      case '9': ret = "1001";
      break;
      case 'a': ret = "1010";
      break;
      case 'b': ret = "1011";
      break;
      case 'c': ret = "1100";
      break;
      case 'd': ret = "1101";
      break;
      case 'e': ret = "1110";
      break;
      case 'f': ret = "1111";
      break;
      default: ret = "";
  }
  return ret;
}

std::string hexToBinary(std::string s){
    std::string ret = "";
    for(char& c : s) {
      ret = ret + hexToBinaryLookupTable(c);
    }
    return ret;
};


int main(){
  std::thread httpServer (initHttpServer);
  std::thread p2pServer (initP2PServer);
  initWallet();

  std::list<Block> chain = getBlockchain();

  std::list<Block>::iterator it;
  for(it = chain.begin(); it != chain.end(); ++it){
    std::cout << "Hash del primo blocco: " << it->hash << std::endl;
  }

  httpServer.join();
  p2pServer.join();

  return 0;
}
