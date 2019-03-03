#include "HttpServer.hpp"

using namespace std;

vector<TxIn> parseTxInVector(const rapidjson::Value &txIns){
  if(!txIns.IsArray() || txIns.IsNull()){
    cout << endl;
    throw "Error parsing request: <TxIns Array>";
  }
  vector<TxIn> ret;
  for (rapidjson::SizeType i = 0; i < txIns.Size(); i++){
    ret.push_back(TxIn(txIns[i]["txOutId"].GetString(), txIns[i]["signature"].GetString(), txIns[i]["txOutIndex"].GetInt()));
  }
  return ret;
}

vector<TxOut> parseTxOutVector(const rapidjson::Value &txOuts){
  if(!txOuts.IsArray() || txOuts.IsNull()){
    cout << endl;
    throw "Error parsing request: <TxOuts Array>";
  }
  vector<TxOut> ret;
  for (rapidjson::SizeType i = 0; i < txOuts.Size(); i++){
    ret.push_back(TxOut(txOuts[i]["address"].GetString(), txOuts[i]["amount"].GetFloat()));
  }
  return ret;
}

vector<Transaction> parseTransactionVector(const rapidjson::Value &transactions){
  if(!transactions.IsArray() || transactions.IsNull()){
    cout << endl;
    throw "Error parsing request: <Transactions Array>";
  }
  vector<Transaction> ret;
  try{
    for (rapidjson::SizeType i = 0; i < transactions.Size(); i++){
      ret.push_back(Transaction(transactions[i]["id"].GetString(), parseTxInVector(transactions[i]["txIns"]), parseTxOutVector(transactions[i]["txOuts"])));
    }
  }catch(const char* msg){
    cout << endl;
    throw "Error parsing request: <Transactions Array>";
  }

  return ret;
}

list<Block> parseBlockList(const rapidjson::Value &blocks){
  if(!blocks.IsArray() || blocks.IsNull()){
    cout << endl;
    throw "Error parsing request: <Block List>";
  }
  list<Block> ret;
  return ret;
  try{
    for (rapidjson::SizeType i = 0; i < blocks.Size(); i++){
      ret.push_back(Block(blocks[i]["index"].GetInt(), blocks[i]["hash"].GetString(), blocks[i]["previousHash"].GetString(), long(blocks[i]["timestamp"].GetInt()), parseTransactionVector(blocks[i]["data"]), blocks[i]["difficulty"].GetInt(), blocks[i]["nonce"].GetInt()));
    }
  }catch(const char* msg){
    cout << endl;
    throw "Error parsing request: <Block List>";
  }
  return ret;
}

string printUnspentTxOuts(vector<UnspentTxOut> unspentTxOuts){
  string ret = "[";
  vector<UnspentTxOut>::iterator it;
  for(it = unspentTxOuts.begin(); it != unspentTxOuts.end(); ++it){
    if(it != unspentTxOuts.begin()){
      ret = ret + ", ";
    }
    ret = ret + it->toString();
  }
  ret = ret + "]";
  return ret;
}

string printTransactions(vector<Transaction> transactions){
  string ret = "[";
  vector<Transaction>::iterator it;
  for(it = transactions.begin(); it != transactions.end(); ++it){
    if(it != transactions.begin()){
      ret = ret + ", ";
    }
    ret = ret + it->toString();
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

crow::response optionsResponse() {
  crow::response resp;
  resp.code = 200;
  resp.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  resp.add_header("Access-Control-Allow-Headers", "Content-Type");
  resp.add_header("Access-Control-Allow-Origin", "*");
  return resp;
}

crow::response createResponse(string data, int code) {
  crow::response resp(data);
  resp.code = code;
  resp.add_header("Content-Type", "application/json");
  resp.add_header("Access-Control-Allow-Origin", "*");
  return resp;
}

void initHttpServer(int port){
  crow::SimpleApp app;

  // Ritorna la chiave pubblica del wallet dell'utente corrente
  CROW_ROUTE(app, "/webresources/pubblickey")([]() {
      return createResponse("{\"success\" :true, \"pubblickey\": \"" + getPublicFromWallet() + "\"}", 200);
  });

  // Ritorna una stringa contenente la blockchain
  CROW_ROUTE(app, "/webresources/blocks")([]() {
      return createResponse("{\"success\" :true, \"blockchain\": " + BlockChain::getInstance().toString() + "}", 200);
  });

  // Ritorna un blocco della blockchain dato il suo hash
  CROW_ROUTE(app,"/webresources/blocks/<string>")([](string hash){
    try{
      return createResponse("{\"success\" :true, \"block\": " + getBlockFromHash(BlockChain::getInstance().getBlockchain(), hash).toString() + "}", 200);
    }catch(const char* msg){
      CROW_LOG_INFO << msg << "\n";
      return createResponse("{\"success\": false, \"message\": \"Error: block not found!\" }", 200);
    }
  });

  // Ritorna una transazione della blockchain dato il suo id
  CROW_ROUTE(app, "/webresources/transactions/<string>")([](string id){
    list<Block> blockchain = BlockChain::getInstance().getBlockchain();
    try{
      return createResponse("{\"success\": true, \"transaction\": " + getTransactionFromId(blockchain, id).toString() + "}", 200);
    }catch(const char* msg){
      CROW_LOG_INFO << msg << "\n";
      return createResponse("{\"success\": false, \"message\": \"Error: transaction not found!\" }", 200);
    }
  });

  // Ritorna la lista degli output non spesi dell'intera blockchain
  CROW_ROUTE(app, "/webresources/unspentTransactionOutputs")([]() {
      return createResponse("{\"success\" :true, \"unspentTxOuts\": " + printUnspentTxOuts(BlockChain::getInstance().getUnspentTxOuts()) + "}", 200);
  });

  // Ritorna la lista degli output non spesi appartenenti ad un certo indirizzo (wallet)
  CROW_ROUTE(app, "/webresources/unspentTransactionOutputs/<string>")([](string address){
      return createResponse("{\"success\": true, \"unspentTxOuts\":" + printUnspentTxOuts(findUnspentTxOutsOfAddress(address, BlockChain::getInstance().getUnspentTxOuts())) + "}", 200);
  });

  // Ritorna la lista degli output non spesi relativi al wallet dell'utente
  CROW_ROUTE(app, "/webresources/myUnspentTransactionOutputs")([]() {
      return createResponse("{\"success\" :true, \"myUnspentTxOuts\": " + printUnspentTxOuts(BlockChain::getInstance().getMyUnspentTransactionOutputs()) + "}", 200);
  });

  // Effettua il mining di un nuovo blocco utilizzando le transazioni passate come argomento (no coinbase transaction)
  CROW_ROUTE(app, "/webresources/blocks/transactions").methods("POST"_method, "OPTIONS"_method)([](const crow::request& req){
    if(req.method == "OPTIONS"_method) {
      return optionsResponse();           // Per gestire il CORS
    } else {
      vector<Transaction> transactions;
      rapidjson::Document document;
      document.Parse(req.body.c_str());
      const rapidjson::Value& data = document["data"];

      if(!data.IsArray() || data.IsNull()){
        return createResponse("{\"success\": false, \"message\": \"Error parsing request: Block data not present\" }", 200);
      }
      try{
        transactions = parseTransactionVector(data);
        Block newBlock = BlockChain::getInstance().generateRawNextBlock(transactions);
        return createResponse("{\"success\" :true, \"newBlock\": " + newBlock.toString() + "}", 200);
      }catch(const char* msg){
        CROW_LOG_INFO << msg << "\n";
        return createResponse("{\"success\": false, \"message\": \"Error: an error occurs during the generation of the new block\" }", 200);
      }
    }
  });

  // Effettua il mining di un nuovo blocco utilizzando le transazioni del transaction pool (più la coinbase transaction)
  CROW_ROUTE(app, "/webresources/blocks/pool").methods("POST"_method, "OPTIONS"_method)([](const crow::request& req){
    if(req.method == "OPTIONS"_method) {
      return optionsResponse();           // Per gestire il CORS
    } else {
      try{
        Block newBlock = BlockChain::getInstance().generateNextBlock();
        return createResponse("{\"success\" :true, \"newBlock\": " + newBlock.toString() + "}", 200);
      }catch(const char* msg){
        CROW_LOG_INFO << msg << "\n";
        return createResponse("{\"success\": false, \"message\": \"Errore durante la generazione del nuovo blocco\" }", 200);
      }
    }
  });

  // Effettua il mining di un nuovo blocco utilizzando una nuova transazione creata a partire dall'amount e address passati (più la coinbase transaction)
  CROW_ROUTE(app, "/webresources/blocks/transaction").methods("POST"_method, "OPTIONS"_method)([](const crow::request& req){
    if(req.method == "OPTIONS"_method) {
      return optionsResponse();           // Per gestire il CORS
    } else {
      rapidjson::Document document;
      document.Parse(req.body.c_str());
      if(document["amount"].IsNull() || document["address"].IsNull()){
        return createResponse("{\"success\": false, \"message\": \"Error parsing request: invalid address or amount\" }", 200);
      }
      try {
        Block newBlock = BlockChain::getInstance().generateNextBlockWithTransaction(document["address"].GetString(), document["amount"].GetFloat());
        return createResponse("{\"success\" :true, \"newBlock\": " + newBlock.toString() + "}", 200);
      } catch (const char* msg) {
        CROW_LOG_INFO << msg << "\n";
        return createResponse("{\"success\": false, \"message\": \"Error generating new block\" }", 200);
      }
    }
  });

  // Crea una nuova transazione e la inserisce nel transaction pool
  CROW_ROUTE(app, "/webresources/transactions").methods("POST"_method, "OPTIONS"_method)([](const crow::request& req){
    if(req.method == "OPTIONS"_method) {
      return optionsResponse();           // Per gestire il CORS
    } else {
      rapidjson::Document document;
      document.Parse(req.body.c_str());
      if(document["amount"].IsNull() || document["address"].IsNull()){
        return createResponse("{\"success\": false, \"message\": \"Error parsing request: invalid address or amount\" }", 200);
      }
      try {
        Transaction tx = BlockChain::getInstance().sendTransaction(document["address"].GetString(), document["amount"].GetFloat());
        return createResponse("{\"success\" :true, \"Transaction\": " + tx.toString() + "}", 200);
      } catch (const char* msg) {
        CROW_LOG_INFO << msg << "\n";
        return createResponse("{\"success\": false, \"message\": \"Error sending the transaction\" }", 200);
      }
    }
  });

  // Ritorna il balance del wallet
  CROW_ROUTE(app, "/webresources/balance")([]() {
    return createResponse("{\"success\" :true, \"balance\": " + to_string(BlockChain::getInstance().getAccountBalance()) + "}", 200);
  });

  // Ritorna la transaction pool del nodo
  CROW_ROUTE(app, "/webresources/transactionPool")([]() {
      return createResponse("{\"success\" :true, \"transactionPool\": " + printTransactions(TransactionPool::getInstance().getTransactionPool()) + "}", 200);
  });

  // Aggiunge un peer alla lista di peer (dato il suo indirizzo IP)
  CROW_ROUTE(app, "/webresources/peers").methods("POST"_method, "OPTIONS"_method)([](const crow::request& req){
    if(req.method == "OPTIONS"_method) {
      return optionsResponse();           // Per gestire il CORS
    } else {
      rapidjson::Document document;
      document.Parse(req.body.c_str());
      if(document["peer"].IsNull()){
        return createResponse("{\"success\": false, \"message\": \"Error parsing request: cannot find field <peer>\" }", 200);
      }
      string peer = document["peer"].GetString();
      try{
        Peer::getInstance().connectToPeers(peer);
        return createResponse("{\"success\": true, \"peer\": \"" + peer + "\" }", 200);
      }catch (const char* msg) {
        CROW_LOG_INFO << msg << "\n";
        return createResponse("{\"success\": false, \"message\": \"Error connecting to new peer\" }", 200);
      }
    }
  });

  // Ritorna il numero di peer cui è connesso il nodo
  CROW_ROUTE(app, "/webresources/peers")([]() {
    return createResponse("{\"success\" :true, \"peers\": " + to_string(Peer::getInstance().countPeers() + 1) + "}", 200);
  });

  // Rest che consente di leggere le righe di un file, ritornate in un array. Questa rest serve ai fini delle statistiche del client rf
  // Param: blockchainStats -> statistiche sulla blockchain, blocksminingtime -> statistiche sul tempo di mining dei blocchi, transactionWaitingTime -> statistiche sul tempo di attesa delle transazioni
  CROW_ROUTE(app, "/webresources/stats/<string>")([](string filename){
    bool isFirst = true;
    string data = "[";
    string line;
    string ret;
    try{
      ifstream inFile;
      inFile.open(filename + ".txt", ios::in);  // blockchainstats, blocksminingtime, transactionwaitingtime
      if(inFile) {
        while (getline(inFile, line)) {
          if(isFirst) {
            data += line;
            isFirst = false;
          } else {
            data = data + ", " + line;
          }
        }
        ret = "{\"success\": true, \"data\": " + data + "}";
      } else {
        throw "Errore: non è stato possibile aprire il file per leggere il tempo di mining del blocco!";
      }
    }catch(const char* msg){
      cout << msg << endl;
      cout << endl;
      ret = "{\"success\": false, \"message\": \"Errore durante l'apertura del file " + filename + ".txt\"" + "}";
    }
    return createResponse(ret, 200);

  });

  // Start del server HTTP sulla porta designata
  cout << "Starting Http Server on port" << port << "..." << endl;
  app.port(port).run();
}
