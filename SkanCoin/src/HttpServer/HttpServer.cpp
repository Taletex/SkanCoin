#include "HttpServer.hpp"

using namespace std;

/* ==== SERVER HTTP E RICHIESTE REST ====
 * Nota: le REST POST accettano come parametri nel body solo elementi di tipo
 * application/json o text/plain (raw data). Non è possibile inviare dati in
 * formato application/www-x-form-urlencoded o multipart/form-data            */
void initHttpServer(int port){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  crow::SimpleApp app;

  /* REST: Ritorna la chiave pubblica del wallet dell'utente corrente */
  CROW_ROUTE(app, "/webresources/publickey")([]() {
      return createResponse("{\"success\" :true, \"publickey\": \"" + getWalletPublicKey() + "\"}", 200);
  });

  /* REST: Ritorna la blockchain */
  CROW_ROUTE(app, "/webresources/blocks")([]() {
      return createResponse("{\"success\" :true, \"length\": " + to_string(BlockChain::getInstance().getBlockchain().size()) + ", \"blockchain\": " + BlockChain::getInstance().toString() + "}", 200);
  });

  /* REST: Ritorna un blocco della blockchain, dato il suo hash */
  CROW_ROUTE(app,"/webresources/blocks/<string>")([](string hash){
    try{
      return createResponse("{\"success\" :true, \"block\": " + BlockChain::getInstance().getBlockFromHash(hash).toString() + "}", 200);
    }catch(const char* msg){
      CROW_LOG_INFO << msg << "\n";
      return createResponse("{\"success\": false, \"message\": \"Error: block not found!\" }", 200);
    }
  });

  /* REST: Ritorna una transazione della blockchain dato il suo id */
  CROW_ROUTE(app, "/webresources/transactions/<string>")([](string id){
    try{
      return createResponse("{\"success\": true, \"transaction\": " + BlockChain::getInstance().getTransactionFromId(id).toString() + "}", 200);
    }catch(const char* msg){
      CROW_LOG_INFO << msg << "\n";
      return createResponse("{\"success\": false, \"message\": \"Error: transaction not found!\" }", 200);
    }
  });

  /* REST: Ritorna la lista degli output non spesi dell'intera blockchain */
  CROW_ROUTE(app, "/webresources/unspentTransOuts")([]() {
      return createResponse("{\"success\" :true, \"unspentTransOuts\": " + printUnspentTransOuts(BlockChain::getInstance().getUnspentTransOuts()) + "}", 200);
  });

  /* REST: Ritorna la lista degli output non spesi appartenenti ad un certo indirizzo (wallet) */
  CROW_ROUTE(app, "/webresources/unspentTransOuts/<string>")([](string address){
      return createResponse("{\"success\": true, \"unspentTransOuts\":" + printUnspentTransOuts(getUnspentTransOutsOfAddress(address, BlockChain::getInstance().getUnspentTransOuts())) + "}", 200);
  });

  /* REST: Ritorna la lista degli output non spesi relativi al wallet dell'utente */
  CROW_ROUTE(app, "/webresources/myUnspentTransOuts")([]() {
      return createResponse("{\"success\" :true, \"myUnspentTransOuts\": " + printUnspentTransOuts(BlockChain::getInstance().getMyUnspentTransactionOutputs()) + "}", 200);
  });

  /* REST: Ritorna il balance del wallet */
  CROW_ROUTE(app, "/webresources/balance")([]() {
    return createResponse("{\"success\" :true, \"balance\": " + to_string(BlockChain::getInstance().getAccountBalance()) + "}", 200);
  });

  /* REST: Ritorna la transaction pool del nodo */
  CROW_ROUTE(app, "/webresources/transactionPool")([]() {
      return createResponse("{\"success\" :true, \"length\": " + to_string(TransactionPool::getInstance().getPool().size()) + ", \"transactionPool\": " + printTransactions(TransactionPool::getInstance().getPool()) + "}", 200);
  });

  /* REST: Crea una nuova transazione e la inserisce nel transaction pool */
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
        Transaction transaction = BlockChain::getInstance().sendTransaction(document["address"].GetString(), document["amount"].GetFloat());
        return createResponse("{\"success\" :true, \"transaction\": " + transaction.toString() + "}", 200);
      } catch (const char* msg) {
        CROW_LOG_INFO << msg << "\n";
        return createResponse("{\"success\": false, \"message\": \"Error sending the transaction\" }", 200);
      }
    }
  });

  /* REST: Effettua il mining di un nuovo blocco prelevando fino a 2 transazioni del transaction pool (più la coinbase transaction) */
  CROW_ROUTE(app, "/webresources/blocks/pool").methods("POST"_method, "OPTIONS"_method)([](const crow::request& req){
    if(req.method == "OPTIONS"_method) {
      return optionsResponse();           // Per gestire il CORS
    } else {
      try{
        Block newBlock = BlockChain::getInstance().createNextBlock();
        return createResponse("{\"success\" :true, \"newBlock\": " + newBlock.toString() + "}", 200);
      }catch(const char* msg){
        CROW_LOG_INFO << msg << "\n";
        return createResponse("{\"success\": false, \"message\": \"Errore durante la generazione del nuovo blocco\" }", 200);
      }
    }
  });

  /* REST: Effettua il mining di un nuovo blocco contente la coinbase transaction
  e una transazione con uno o più output destinazione */
  CROW_ROUTE(app, "/webresources/blocks/transaction").methods("POST"_method, "OPTIONS"_method)([](const crow::request& req){
    if(req.method == "OPTIONS"_method) {
      return optionsResponse();           // Per gestire il CORS
    } else {
      vector<TransOut> transactions;
      rapidjson::Document document;
      document.Parse(req.body.c_str());
      const rapidjson::Value& data = document["data"];

      if(!data.IsArray() || data.IsNull()){
        return createResponse("{\"success\": false, \"message\": \"Error parsing request: Block data not present\" }", 200);
      }
      try{
        transactions = parseTransOutVector(data);
        Block newBlock = BlockChain::getInstance().createNextBlockWithTransaction(transactions);
        return createResponse("{\"success\" :true, \"newBlock\": " + newBlock.toString() + "}", 200);
      }catch(const char* msg){
        CROW_LOG_INFO << msg << "\n";
        return createResponse("{\"success\": false, \"message\": \"Error: an error occurs during the generation of the new block\" }", 200);
      }
    }
  });

  /* REST: Aggiunge un peer alla lista di peer (dato il suo indirizzo IP e numero di porta) */
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
        Peer::getInstance().connectToPeer(peer);
        return createResponse("{\"success\": true, \"peer\": \"" + peer + "\" }", 200);
      }catch (const char* msg) {
        CROW_LOG_INFO << msg << "\n";
        return createResponse("{\"success\": false, \"message\": \"Error connecting to new peer\" }", 200);
      }
    }
  });

  /* REST: Ritorna il numero di peer cui è connesso il nodo */
  CROW_ROUTE(app, "/webresources/peers")([]() {
    return createResponse("{\"success\" :true, \"peers\": " + to_string(Peer::getInstance().countPeers() + 1) + "}", 200);
  });

  /* REST: Consente di leggere le righe di un file, ritornate in un array.
   * Questa rest serve ai fini delle statistiche del client R.
   * Possibili valori del parametro sono: blockchainStats -> statistiche sulla blockchain,
   * blocksminingtime -> statistiche sul tempo di mining dei blocchi, transactionWaitingTime
   * -> statistiche sul tempo di attesa delle transazioni */
  CROW_ROUTE(app, "/webresources/stats/<string>")([](string filename){
    bool isFirst = true;
    string data = "[";
    string line;
    string ret;
    try{
      ifstream inFile;
      inFile.open(filename + ".txt", ios::in);  // blockchainstats, blocksminingtime, transactionwaitingtime
      if(inFile.is_open()) {
        while (getline(inFile, line)) {
          if(isFirst) {
            data += line;
            isFirst = false;
          } else {
            data = data + ", " + line;
          }
        }
        inFile.close();
        ret = "{\"success\": true, \"data\": " + data + "]}";
      } else {
        throw "ERRORE: non è stato possibile aprire il file per leggere il tempo di mining del blocco!";
      }
    }catch(const char* msg){
      cout << msg << endl << endl;
      ret = "{\"success\": false, \"message\": \"Errore durante l'apertura del file " + filename + ".txt\"" + "}";
    }
    return createResponse(ret, 200);

  });

  // Start del server HTTP sulla porta designata
  cout << "Avvio del Server HTTP sulla porta " << port << "..." << endl;
  app.port(port).run();
}

/* ==== FUNZIONI PER LA GESTIONE DELLE RESPONSE CROW ==== */
/* Ritorna una response crow per le richiste di tipo OPTIONS (per gestire il CORS) */
crow::response optionsResponse() {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  crow::response resp;
  resp.code = 200;
  resp.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  resp.add_header("Access-Control-Allow-Headers", "Content-Type");
  resp.add_header("Access-Control-Allow-Origin", "*");
  return resp;
}

/* Ritorna una response crow con un body contenente data e un codice code */
crow::response createResponse(string data, int code) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  crow::response resp(data);
  resp.code = code;
  resp.add_header("Content-Type", "application/json");
  resp.add_header("Access-Control-Allow-Origin", "*");
  return resp;
}


/* ==== FUNZIONI PER IL PARSING E GESTIONE DEI JSON (tramite rapidjson) ====
 * Parsing da collezioni C++ a stringhe C++ (da usare dentro le risposte HTTP)
 * e da valori rapidjson a collezioni C++                                     */

/* Effettua il parsing di un vettore di transaction input: JSON (rapidjson) -> vector<TransIn> */
vector<TransIn> parseTransInVector(const rapidjson::Value &transIns){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if(!transIns.IsArray() || transIns.IsNull()){
    cout << endl;
    throw "Error parsing request: <TransIns Array>";
  }
  vector<TransIn> ret;
  for (rapidjson::SizeType i = 0; i < transIns.Size(); i++){
    ret.push_back(TransIn(transIns[i]["transOutId"].GetString(), transIns[i]["signature"].GetString(), transIns[i]["transOutIndex"].GetInt()));
  }
  return ret;
}

/* Effettua il parsing di un vettore di transaction output: JSON (rapidjson) -> vector<TransOut> */
vector<TransOut> parseTransOutVector(const rapidjson::Value &transOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if(!transOuts.IsArray() || transOuts.IsNull()){
    cout << endl;
    throw "Error parsing request: <TransOuts Array>";
  }
  vector<TransOut> ret;
  for (rapidjson::SizeType i = 0; i < transOuts.Size(); i++){
    ret.push_back(TransOut(transOuts[i]["address"].GetString(), transOuts[i]["amount"].GetFloat()));
  }
  return ret;
}

/* Effettua il parsing di un vettore di transazioni: JSON (rapidjson) -> vector<Transaction> */
vector<Transaction> parseTransactionVector(const rapidjson::Value &transactions){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if(!transactions.IsArray() || transactions.IsNull()){
    cout << endl;
    throw "Error parsing request: <Transactions Array>";
  }
  vector<Transaction> ret;
  try{
    for (rapidjson::SizeType i = 0; i < transactions.Size(); i++){
      ret.push_back(Transaction(transactions[i]["id"].GetString(), parseTransInVector(transactions[i]["transIns"]), parseTransOutVector(transactions[i]["transOuts"])));
    }
  }catch(const char* msg){
    cout << endl;
    throw "Error parsing request: <Transactions Array>";
  }

  return ret;
}

/* Effettua il parsing di una lista di blocchi: JSON (rapidjson) -> list<Block> */
list<Block> parseBlockList(const rapidjson::Value &blocks){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if(!blocks.IsArray() || blocks.IsNull()){
    cout << endl;
    throw "Error parsing request: <Block List>";
  }
  list<Block> ret;
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

/* Stampa un vector di unspentTransOuts: vector<UnspentTransOut> -> String (da inserire in un JSON) */
string printUnspentTransOuts(vector<UnspentTransOut> unspentTransOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  string ret = "[";
  vector<UnspentTransOut>::iterator it;
  for(it = unspentTransOuts.begin(); it != unspentTransOuts.end(); ++it){
    if(it != unspentTransOuts.begin()){
      ret = ret + ", ";
    }
    ret = ret + it->toString();
  }
  ret = ret + "]";
  return ret;
}

/* Stampa un vector di transazioni: vector<Transaction> -> String (da inserire in un JSON) */
string printTransactions(vector<Transaction> transactions){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

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
