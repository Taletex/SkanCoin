#include "crow.h"
#include "../Blockchain/Blockchain.hpp"

using namespace std;

const int httpPort= 3001;

string printBlockChain(list<Block> blockchain){
  string ret = "[";
  list<Block>::iterator it;
  for(it = blockchain.begin(); it != blockchain.end(); ++it){
    ret = ret + it->toString() + ", ";
  }
  ret = ret + "]";
  return ret;
}

Block getBlockFromHash(list<Block> blockchain, string hash){
  list<Block>::iterator it;
  for(it = blockchain.begin(); it != blockchain.end(); ++it){
    if(it->hash == hash){
      return *it;
    }
  }
  throw "EXCEPTION: Block not found!";
}

Transaction getTransactionFromId(list<Block> blockchain, string id){
  list<Block>::iterator it;
  vector<Transaction>::iterator it2;
  for(it = blockchain.begin(); it != blockchain.end(); ++it){
    for(it2 = it->data.begin(); it2 != it->data.end(); ++it2){
      if(it2->id == id){
        return *it2;
      }
    }
  }
  throw "EXCEPTION: Transaction not found in the blockchain";
}

void initHttpServer()
{
  crow::SimpleApp app;
  CROW_ROUTE(app, "/blocks")([]() {
      return "{'success' :true, 'BlockChain': " + printBlockChain(BlockChain::getInstance().getBlockchain()) + "}";
  });

  CROW_ROUTE(app,"/block/<string>")([](string hash){
    cout << hash << endl;
    try{
      return "{'success' :true, Block: " + getBlockFromHash(BlockChain::getInstance().getBlockchain(), hash).toString() + "}";
    }catch(const char* msg){
      cout << msg << endl;
      string ret = "{'success': false, 'message': \"Block not found\" }";
      return ret;
    }
  });

  CROW_ROUTE(app,"/transaction/<string>")([](string id){
    list<Block> blockchain = BlockChain::getInstance().getBlockchain();
    try{
      return "{'success': true, 'Transaction': " + getTransactionFromId(blockchain, id).toString() + "}";
    }catch(const char* msg){
      cout << msg << endl;
      string ret = "{'success': false, 'message': \"Transaction not found in the blockchain!\" }";
      return ret;
    }
  });

  CROW_ROUTE(app,"/address/<string>")([](string address){
      vector<UnspentTxOut> unspentTxOuts = BlockChain::getInstance().getUnspentTxOuts();
      vector<UnspentTxOut>::iterator it;
      for (it = unspentTxOuts.begin(); it != unspentTxOuts.end(); ) {
        if (it->address != address) {
          it = unspentTxOuts.erase(it);
        } else {
          ++it;
        }
      }
      string ret = "{'success': true, 'UnspentTxOuts': [";
      for(it = unspentTxOuts.begin(); it != unspentTxOuts.end(); ++it){
        if(it != unspentTxOuts.begin()){
          ret = ret + ", ";
        }
        ret = ret + it->toString();
      }
      ret = ret +  + "]}";
      return ret;
  });

  std::cout << "Starting Http Server..." << std::endl;
  app.port(3001).run();
}
