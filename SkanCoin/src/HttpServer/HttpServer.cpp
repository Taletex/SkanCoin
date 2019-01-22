#include "crow.h"
#include "../Blockchain/Blockchain.hpp"

using namespace std;

const int httpPort= 3001;

string printUnspentTxOuts(vector<UnspentTxOut> unspentTxOuts){
  string ret = "[";
  vector<UnspentTxOut>::iterator it;
  for(it = unspentTxOuts.begin(); it != unspentTxOuts.end(); ++it){
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
  cout << endl;
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
  cout << endl;
  throw "EXCEPTION: Transaction not found in the blockchain";
}

void initHttpServer()
{
  crow::SimpleApp app;
  CROW_ROUTE(app, "/blocks")([]() {
      return "{'success' :true, 'blockchain': " + BlockChain::getInstance().toString() + "}";
  });

  CROW_ROUTE(app,"/block/<string>")([](string hash){
    try{
      return "{'success' :true, block: " + getBlockFromHash(BlockChain::getInstance().getBlockchain(), hash).toString() + "}";
    }catch(const char* msg){
      CROW_LOG_INFO << msg << "\n";
      string ret = "{'success': false, 'message': \"Block not found\" }";
      return ret;
    }
  });

  CROW_ROUTE(app,"/transaction/<string>")([](string id){
    list<Block> blockchain = BlockChain::getInstance().getBlockchain();
    try{
      return "{'success': true, 'transaction': " + getTransactionFromId(blockchain, id).toString() + "}";
    }catch(const char* msg){
      CROW_LOG_INFO << msg << "\n";
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
      string ret = "{'success': true, 'unspentTxOuts': [";
      for(it = unspentTxOuts.begin(); it != unspentTxOuts.end(); ++it){
        if(it != unspentTxOuts.begin()){
          ret = ret + ", ";
        }
        ret = ret + it->toString();
      }
      ret = ret +  + "]}";
      return ret;
  });

  CROW_ROUTE(app, "/unspentTransactionOutputs")([]() {
      return "{'success' :true, 'unspentTxOuts': " + printUnspentTxOuts(BlockChain::getInstance().getUnspentTxOuts()) + "}";
  });

  CROW_ROUTE(app, "/myUnspentTransactionOutputs")([]() {
      return "{'success' :true, 'myUnspentTxOuts': " + printUnspentTxOuts(BlockChain::getInstance().getMyUnspentTransactionOutputs()) + "}";
  });

  CROW_ROUTE(app, "/mineRawBlock").methods("POST"_method)([](const crow::request& req){
      CROW_LOG_INFO << "msg from client: " << req.body;
    //  broadcast(req.body);
      return "";
  });

  /*
  app.post('/mineRawBlock', (req, res) => {
      if (req.body.data == null) {
          res.send('data parameter is missing');
          return;
      }
      const newBlock: Block = generateRawNextBlock(req.body.data);
      if (newBlock === null) {
          res.status(400).send('could not generate block');
      } else {
          res.send(newBlock);
      }
  });
  */


  CROW_LOG_INFO << "Starting Http Server...\n";
  app.port(3001).run();
}
