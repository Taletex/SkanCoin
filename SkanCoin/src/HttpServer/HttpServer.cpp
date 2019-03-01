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

void initHttpServer(int port){
  crow::SimpleApp app;

  // Ritorna una stringa contenente la blockchain
  CROW_ROUTE(app, basepath + "blocks")([]() {
      return "{\"success\" :true, \"blockchain\": " + BlockChain::getInstance().toString() + "}";
  });

  // Ritorna un blocco della blockchain dato il suo hash
  CROW_ROUTE(app,"webresources/block/<string>")([](string hash){
    try{
      return "{\"success\" :true, \"block\": " + getBlockFromHash(BlockChain::getInstance().getBlockchain(), hash).toString() + "}";
    }catch(const char* msg){
      CROW_LOG_INFO << msg << "\n";
      string ret = "{\"success\": false, \"message\": \"Block not found\" }";
      return ret;
    }
  });

  // Ritorna una transazione della blockchain dato il suo id
  CROW_ROUTE(app, basepath + "transaction/<string>")([](string id){
    list<Block> blockchain = BlockChain::getInstance().getBlockchain();
    try{
      return "{\"success\": true, \"transaction\": " + getTransactionFromId(blockchain, id).toString() + "}";
    }catch(const char* msg){
      CROW_LOG_INFO << msg << "\n";
      string ret = "{\"success\": false, \"message\": \"Transaction not found in the blockchain!\" }";
      return ret;
    }
  });

  // Ritorna la lista degli output non spesi appartenenti ad un certo indirizzo (wallet)
  CROW_ROUTE(app, basepath + "address/<string>")([](string address){
      string ret = "{\"success\": true, \"unspentTxOuts\":" + printUnspentTxOuts(findUnspentTxOutsOfAddress(address, BlockChain::getInstance().getUnspentTxOuts())) + "}";
      return ret;
  });

  // Ritorna la lista degli output non spesi dell'intera blockchain
  CROW_ROUTE(app, basepath + "unspentTransactionOutputs")([]() {
      return "{\"success\" :true, \"unspentTxOuts\": " + printUnspentTxOuts(BlockChain::getInstance().getUnspentTxOuts()) + "}";
  });

  // Ritorna la lista degli output non spesi relativi al wallet dell'utente
  CROW_ROUTE(app, basepath + "myUnspentTransactionOutputs")([]() {
      return "{\"success\" :true, \"myUnspentTxOuts\": " + printUnspentTxOuts(BlockChain::getInstance().getMyUnspentTransactionOutputs()) + "}";
  });

  // Effettua il mine di un raw block (TODO: Controllare bene cosa bisogna passargli)
  CROW_ROUTE(app, basepath + "mineRawBlock").methods("POST"_method)([](const crow::request& req){
    rapidjson::Document document;
    document.Parse(req.body.c_str());
    const rapidjson::Value& data = document["data"];
    if(!data.IsArray() || data.IsNull()){
      string ret = "{\"success\": false, \"message\": \"Error parsing request: Block data not present\" }";
      return ret;
    }
    vector<Transaction> transactions;
    try{
      transactions = parseTransactionVector(data);
    }catch(const char* msg){
      CROW_LOG_INFO << msg << "\n";
      string ret = "{\"success\": false, \"message\": \"Error parsing request: <Transaction vector>\" }";
      return ret;
    }
    try{
      Block newBlock = BlockChain::getInstance().generateRawNextBlock(transactions);
      return "{\"success\" :true, \"newBlock\": " + newBlock.toString() + "}";
    }catch(const char* msg){
      CROW_LOG_INFO << msg << "\n";
      string ret = "{\"success\": false, \"message\": \"Error generating new block\" }";
      return ret;
    }
  });

  // Effettua il mine di un blocco normale (senza transazioni)
  CROW_ROUTE(app, basepath + "mineBlock").methods("POST"_method)([](){
    try{
      Block newBlock = BlockChain::getInstance().generateNextBlock();
      return "{\"success\" :true, \"newBlock\": " + newBlock.toString() + "}";
    }catch(const char* msg){
      CROW_LOG_INFO << msg << "\n";
      string ret = "{\"success\": false, \"message\": \"Error generating new block\" }";
      return ret;
    }
  });

  // Ritorna il balance del wallet
  CROW_ROUTE(app, basepath + "balance")([]() {
    return "{\"success\" :true, \"balance\": " + to_string(BlockChain::getInstance().getAccountBalance()) + "}";
  });

  // Effettua il mine di una transazione (TODO: Controllare bene cosa bisogna passargli)
  CROW_ROUTE(app, basepath + "mineTransaction").methods("POST"_method)([](const crow::request& req){
    rapidjson::Document document;
    document.Parse(req.body.c_str());
    if(document["amount"].IsNull() || document["address"].IsNull()){
      string ret = "{\"success\": false, \"message\": \"Error parsing request: invalid address or amount\" }";
      return ret;
    }
    string address = document["address"].GetString();
    float amount = document["amount"].GetFloat();
    try {
      Block newBlock = BlockChain::getInstance().generatenextBlockWithTransaction(address, amount);
      return "{\"success\" :true, \"newBlock\": " + newBlock.toString() + "}";
    } catch (const char* msg) {
      CROW_LOG_INFO << msg << "\n";
      string ret = "{\"success\": false, \"message\": \"Error generating new block\" }";
      return ret;
    }
  });

  // Invia una transazione (TODO: Controllare bene cosa bisogna passargli)
  CROW_ROUTE(app, basepath + "sendTransaction").methods("POST"_method)([](const crow::request& req){
    rapidjson::Document document;
    document.Parse(req.body.c_str());
    if(document["amount"].IsNull() || document["address"].IsNull()){
      string ret = "{\"success\": false, \"message\": \"Error parsing request: invalid address or amount\" }";
      return ret;
    }
    string address = document["address"].GetString();
    float amount = document["amount"].GetFloat();
    try {
      Transaction tx = BlockChain::getInstance().sendTransaction(address, amount);
      return "{\"success\" :true, \"Transaction\": " + tx.toString() + "}";
    } catch (const char* msg) {
      CROW_LOG_INFO << msg << "\n";
      string ret = "{\"success\": false, \"message\": \"Error sending the transaction\" }";
      return ret;
    }
  });

  // Ritorna la transaction pool del nodo
  CROW_ROUTE(app, basepath + "transactionPool")([]() {
      return "{\"success\" :true, \"transactionPool\": " + printTransactions(TransactionPool::getInstance().getTransactionPool()) + "}";
  });

  // Aggiunge un peer alla lista di peer
  CROW_ROUTE(app, basepath + "addPeer").methods("POST"_method)([](const crow::request& req){
    rapidjson::Document document;
    document.Parse(req.body.c_str());
    if(document["peer"].IsNull()){
      string ret = "{\"success\": false, \"message\": \"Error parsing request: cannot find field <peer>\" }";
      return ret;
    }
    string peer = document["peer"].GetString();
    try{
      P2PServer::getInstance().connectToPeers(peer);
      string ret = "{\"success\": true, \"peer\": \"" + peer + "\" }";
      return ret;
    }catch (const char* msg) {
      CROW_LOG_INFO << msg << "\n";
      string ret = "{\"success\": false, \"message\": \"Error connecting to new peer\" }";
      return ret;
    }
  });

  // Ritorna i peer del nodo
  CROW_ROUTE(app, basepath + "peers")([]() {
    return "{\"success\" :true, \"peers\": " + to_string(P2PServer::getInstance().countPeers() + 1) + "}";
  });

  // TODO: testare
  // Rest che consente di leggere le righe di un file, ritornate in un array. Questa rest serve ai fini delle statistiche del client rf
  // Param: blockchainStats -> statistiche sulla blockchain, blocksminingtime -> statistiche sul tempo di mining dei blocchi, transactionWaitingTime -> statistiche sul tempo di attesa delle transazioni
  CROW_ROUTE(app, basepath + "stats/<string>")([](string filename){
    bool isFirst = true;
    string data = "[";
    string line;
    ifstream inFile;
    inFile.open(to_string(filename) + ".txt");  // blockchainstats, blocksminingtime, transactionwaitingtime

    while (getline(inFile, line)) {
      if(isFirst) {
        data += line;
        isFirst = false;
      } else {
        data = data + ", " + line;
      }
    }

    return "{\"success\": true, \"data\": " + data + "}";
  });

  // TODO: testare
  // Ritorna il tempo di attesa per la conferma di ogni transazione della blockchain
  CROW_ROUTE(app, basepath + "blocksminingtime")([](){
  });

  // Start del server HTTP sulla porta designata
  cout << "Starting Http Server on port" << port << "..." << endl;
  app.port(port).run();
}
