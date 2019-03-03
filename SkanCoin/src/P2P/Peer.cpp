#include "Peer.hpp"

using namespace std;
using easywsclient::WebSocket;

/*Gestore per messaggi in arrivo dalle socke aperte dal thread client (easywsclient::WebSocket)
Questo metodo deve avere un solo parametro (const string & data), questo è il motivo per cui
 abbiamo bisogno di una variabile esterna di supporto (tempWs) che sia un riferimento alla socket corrente*/
void handleClientMessage(const string & data){
  /*std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  cout << "Ricevuto: " << data << endl;
  Peer::getInstance().broadcast("ciao");
  return;*/
  //Parsing dell'oggetto JSON ricevuto dalla socket
  rapidjson::Document document;
  cout << endl << "Client Peer: Messaggio ricevuto: " << data;
  document.Parse(data.c_str());

  //Controllo che il messaggio ricevuto sia valido
  if(document["type"].IsNull() || Peer::getInstance().isValidType(document["type"].GetInt()) == false){
    cout << endl << "ERRORE (handleClientMessage): il tipo di messaggio ricevuto non è valido" << endl;
    return;
  }

  //Mapping dell'oggetto in una nuova istanza di Message
  Message message = Message(static_cast<MessageType>(document["type"].GetInt()), "");

  /*Liste usade per la gestione di alcuni tipi di messaggi (non è possibile inizializzare
   nuove variabili all'interno dei singoli case, se non si usano le parentesi graffe)*/
  list<Block> receivedBlocks;
  vector<Transaction> receivedTransactions;
  vector<Transaction>::iterator it;
  ofstream myfile;
  //Questo switch contiene la logica di gestione dei vari tipi di messaggi in arrivo
  switch (message.type) {
    //Un peer ha richiesto l'ultimo blocco, esso viene inserito nella risposta
    case QUERY_LATEST:
      cout << " - QUERY_LATEST" << endl;
      Peer::getInstance().tempWs->send(Peer::getInstance().responseLatestMsg(""));
      break;
    //Un peer ha richiesto la blockchain, essa viene inserita nella risposta
    case QUERY_ALL:
      cout << " - QUERY_ALL" << endl;
      Peer::getInstance().tempWs->send(Peer::getInstance().responseChainMsg());
      break;
    //Un peer ha inviato la propria versione di blockchain
    case RESPONSE_BLOCKCHAIN:
      cout << " - RESPONSE_BLOCKCHAIN" << endl;
      if(document["data"].IsNull()){
        cout << "ERROR (handleClientMessage - RESPONSE_BLOCKCHAIN): Nessun dato ricevuto" << endl;
        return;
      }
      try{
        receivedBlocks = parseBlockList(document["data"]);
        Peer::getInstance().handleBlockchainResponse(receivedBlocks);
      }catch(const char* msg){
        cout << msg << endl;
        cout << "EXCEPTION (handleClientMessage - RESPONSE_BLOCKCHAIN): Errore durante l'elaborazione' del messaggio!" << endl;
        return;
      }
      /*Aggiornamento dei dati relativi al tempo di mining del blocco (questo campo
       è non nullo solo se siamo in corrispondenza della diffuzione di un nuovo
        blocco che è stato minato ed aggiunto alla blockchain)*/
      if(!document["stat"].IsNull()){
        ofstream myfile;
        myfile.open ("blocksminingtime.txt", ios::out | ios::app);
        if(myfile.is_open()) {
          myfile << document["stat"].GetString();
        } else {
          cout << "ERRORE (handleClientMessage - RESPONSE_BLOCKCHAIN): non è stato possibile aprire il file per salvare il tempo di mining del blocco!";
        }
        myfile.close();
      }
      break;
    //Un peer ha richiesto il transaction pool, esso viene inserito nella risposta
    case QUERY_TRANSACTION_POOL:
      cout << " - QUERY_TRANSACTION_POOL" << endl;
      Peer::getInstance().tempWs->send(Peer::getInstance().responseTransactionPoolMsg());
      break;

    //Un peer ha inviato la propria versione di transaction pool
    case RESPONSE_TRANSACTION_POOL:
      cout << " - RESPONSE_TRANSACTION_POOL" << endl;

      //Parsing del pool ricevuto
      if(document["data"].IsNull()){
        cout << "ERRORE (handleClientMessage - RESPONSE_TRANSACTION_POOL) nessun dato ricevuto!" << endl;
        return;
      }
      try{
        receivedTransactions = parseTransactionVector(document["data"]);
      }catch(const char* msg){
        cout << msg << endl;
        cout << "ERRORE (handleClientMessage - RESPONSE_TRANSACTION_POOL): errore durante il parsing del messaggio!" << endl;
        return;
      }
      /*Per ogni transazione questa viene elaborata e successivamente si
      effettua un broadcast del transaction pool aggiornato*/
      for(it = receivedTransactions.begin(); it != receivedTransactions.end(); ++it){
        try{
            BlockChain::getInstance().handleReceivedTransaction(*it);
            Peer::getInstance().broadCastTransactionPool();
        }catch(const char* msg) {
            cout << msg << endl;
            cout << "ERRORE (handleClientMessage - RESPONSE_TRANSACTION_POOL): Errore durante l'inserimento della transazione nel pool" << endl;
        }
      }
      break;
      /*Un peer ha prelevato delle transazioni dal proprio pool,, aggiornamento
      del file contenente le statistiche*/
      case TRANSACTION_POOL_STATS:
        cout << " - TRANSACTION_POOL_STATS" << endl;
        if(!document["data"].IsNull()){
          try{
            myfile.open ("transactionwaitingtime.txt", ios::out | ios::app);
            if(myfile.is_open()) {
              myfile << document["data"].GetString();
              myfile.close();
            } else {
              throw "EXCEPTION (handleClientMessage - TRANSACTION_POOL_STATS): non è stato possibile aprire il file per salvare le statistiche di attesa delle transazioni!";
            }
          }catch(const char* msg) {
              cout << msg << endl;
              cout << "EXCEPTION (handleClientMessage - TRANSACTION_POOL_STATS): Errore durante l'apertura del file per aggiornare le statistiche" << endl;
          }
        }
        break;

    default:
      cout << "ERRORE (handleClientMessage): Il messaggio non ha un tipo valido!" << endl;
  }
}

Message::Message(MessageType type, string data){
  this->type = type;
  this->data = data;
  this->stat = "";
  if(data.compare("") == 0){
    this->data = "\"\"";
  }
}
Message::Message(MessageType type, string data, string stat){
  this->type = type;
  this->data = data;
  this->stat = stat;
  if(data.compare("") == 0){
    this->data = "\"\"";
  }
}

/*Rappresentazione in formato JSON dell'oggetto Message*/
string Message::toString(){
  return "{\"type\": " + to_string(type) + ", \"data\": " + data + ", \"stat\": \"" + stat + "\"}";
}

/* Questa funzione esegue il polling sulla lista delle connessioni aperte dal
thread client, se ci sono nuovi messaggi ricevuti viene eseguito l'handler adatto*/
void Peer::checkReceivedMessage(){
  /*Flag per segnalare che ci sono socket che sono state chiuse, questo viene conservato
  perchè non è possibile rimuovere la socket nello stesso loop del polling, infatti
  abbiamo bisogno di un riferimento diretto in tale loop, mentre è necessario utilizzare
  un iteratore per rimuovere l'elemento dal vettore */
  bool flag = false;
  connectionsMtx.lock();
  //Controllo dei messaggi in arrivo in ogni socket
  for(auto ws: openedConnections){
    //Controlla se la socket è stat chiusa
    if(ws->getReadyState() != WebSocket::CLOSED) {
      /*Se ci sono messaggi in arrivo, questo metodo riempie il buffer che verrà
       utilizzato dall'handler con i dati ricevuti /*/
      ws->poll();

      /*Assegno il riferimento corrente alla variabile di supporto, in modo da
      poterla utilizare all'interno dell'handler (infatti come spiegato sopra
        non è possibile passare ad esso un secondo parametro)*/
      tempWs = ws;

      //Esecuzione dell'handler per il messaggio ricevuto
      ws->dispatch(handleClientMessage);
    }else{

      //La socket è stat chiusa, dunque setto il relativo flag
      flag = true;
    }
  }
  //Rimozione delle socket chiuse
  if(flag == true){
    clearClosedWs();
  }

  connectionsMtx.unlock();
}

/*Questo metodo controlla il vettore di socket (client), eliminando quelle
che sono state chiuse*/
void Peer::clearClosedWs(){
  vector<easywsclient::WebSocket::pointer> temp; //Creo un vettore di appoggio
  for(auto ws: openedConnections){
    //Inserisco nel nuovo vettore solo le socket aperte
    if(ws->getReadyState() != WebSocket::CLOSED) {
      temp.push_back(ws);
    }
  }
  //Svuoto il vecchio vettore e facendo una sola allocazione inserisco gli elementi presenti nel vettore di appoggio
  openedConnections.clear();
  openedConnections.reserve(openedConnections.size() + temp.size());
  openedConnections.insert(openedConnections.end(), temp.begin(), temp.end());
}

/*Dato l'url di un server P2P viene aperta una nuova connessione verso di esso dal thread client*/
void Peer::connectToPeers(std::string peer){
  cout << "Client Peer: Aggiunta di { " << peer << " } alla lista dei peer..." << endl;
  WebSocket::pointer ws;
  ws = WebSocket::from_url(peer);

  if(!ws) throw "EXCEPTION (ConnectToPeers): Errore durante la connessione al peer!";

  //Lock del mutex sulle liste di connessioni per l'inserimento della nuova connessione
  connectionsMtx.lock();
  ws->send(queryChainLengthMsg());
  openedConnections.push_back(ws);
  connectionsMtx.unlock();
  //Viene triggerata la diffusione del transaction pool di ogni nodo
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  connectionsMtx.lock();
  broadcast(queryTransactionPoolMsg());
  connectionsMtx.unlock();
}

/*Ritorna il numero d peer (si considerano sia le connessioni aperte dal thread client che quelle ricevute dal thread server)*/
int Peer::countPeers(){
  connectionsMtx.lock();
  int count = receivedConnections.size() + openedConnections.size();
  connectionsMtx.unlock();
  return count;
}

/*Business logic per un messaggio di tipo RESPONSE_BLOCKCHAIN, questa funzione
è chiamata nell'handler dei messaggi in arrivo*/
void Peer::handleBlockchainResponse(list<Block> receivedBlocks){
    if (receivedBlocks.size() == 0) {
        cout << "ERRORE (handleBlockchainResponse): La blockchain ricevuta ha lunghezza 0!" << endl;
        return;
    }
    Block latestBlockReceived = receivedBlocks.back();
    if (BlockChain::getInstance().isValidBlockStructure(latestBlockReceived) == false) {
        cout << "ERRORE (handleBlockchainResponse): la struttura del blocco non è valida" << endl;
        return;
    }
    Block latestBlockHeld = BlockChain::getInstance().getLatestBlock();
    if (latestBlockReceived.index > latestBlockHeld.index) {
      //La blockchain locale è probabilmente obsoleta
      if (latestBlockHeld.hash == latestBlockReceived.previousHash) {
        //Il nuovo blocco è l'unico da aggiungere
        if (BlockChain::getInstance().addBlockToChain(latestBlockReceived)) {
            broadcast(responseLatestMsg(""));
        }
      }else if (receivedBlocks.size() == 1) {
        //La blockchain locale è indietro di più di un blocco, richiedo in broadcast le version di blockchain dei vari peer
        broadcast(queryAllMsg());
      } else {
        /*Nel caso in cui abbiamo ricevuto l'intera blockchain e non solo l'ultimo
         blocco, dopo aver rilevato che la blockchain locale è indietro rispetto
         a quella ricevuta, si effettua la sostituzione*/
        BlockChain::getInstance().replaceChain(receivedBlocks);
      }
    }

    /*Se si è ricevuta una blockchain che non è più lunga di quella locale non
    è necessario fare nulla*/

}

/*Inizializzazione del server P2P*/
void Peer::initP2PServer(int port){
  crow::SimpleApp app;
  CROW_ROUTE(app, "/").websocket()
    //Gesitione della ricezione di nuove connessioni
    .onopen([&](crow::websocket::connection& connection){
      CROW_LOG_INFO << "Server Peer: ricevuta nuova connessione";
      connectionsMtx.lock();
      receivedConnections.insert(&connection);
      connection.send_text(queryChainLengthMsg());
      connectionsMtx.unlock();
      //
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      connectionsMtx.lock();
      broadcast(queryTransactionPoolMsg());
      connectionsMtx.unlock();

    })
    //Gestione della chiusura di una socket
    .onclose([&](crow::websocket::connection& connection, const std::string& reason){
      CROW_LOG_INFO << "Server Peer: Una websocket è stata chiusa: " << reason;
      connectionsMtx.lock();
      receivedConnections.erase(&connection);
      connectionsMtx.unlock();
    })
    //Gestione della ricezione di un messaggio
    .onmessage([&](crow::websocket::connection& connection, const std::string& data, bool){
      connectionsMtx.lock();
      handleServerMessage(connection, data);
      connectionsMtx.unlock();
    });

  cout << "Starting P2PServer on port " << port << "..." << endl;
  app.port(port).run();
}

/*Avvio del polling del client sulle socket aperte*/
void Peer::startClientPoll(){
  cout << "Starting P2P client...." << endl;
  while(true){
    /*Il polling avviene con una cadenza di un secondo, non ci sono particolari
    vincoli temporali quindi non è necessario sovraccaricare la rete o il nodo effettuando un polling moto frequente*/
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    checkReceivedMessage();
  }
}

/*Gestore dei messaggi in arrivo al Server Peer*/
void Peer::handleServerMessage(crow::websocket::connection& connection, const string& data){
  /*std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  cout << "Ricevuto: " << data << endl;
  broadcast("ciao2");
  return;*/
  //Parsing dell'oggetto JSON ricevuto dalla socket

  rapidjson::Document document;
  cout << endl << "Server Peer: Messaggio ricevuto: " << data;
  document.Parse(data.c_str());

  //Controllo che il messaggio ricevuto sia valido
  if(document["type"].IsNull() || isValidType(document["type"].GetInt()) == false){
    cout << endl << "ERRORE (handleServerMessage): il tipo di messaggio ricevuto non è valido" << endl;    return;
  }

  //Mapping dell'oggetto in una nuova istanza di Message
  Message message = Message(static_cast<MessageType>(document["type"].GetInt()), "");

  /*Liste usade per la gestione di alcuni tipi di messaggi (non è possibile inizializzare
   nuove variabili all'interno dei singoli case, se non si usano le parentesi graffe)*/
  list<Block> receivedBlocks;
  vector<Transaction> receivedTransactions;
  vector<Transaction>::iterator it;
  ofstream myfile;
  //Questo switch contiene la logica di gestione dei vari tipi di messaggi in arrivo
  switch (message.type) {
    //Un peer ha richiesto l'ultimo blocco, esso viene inserito nella risposta
    case QUERY_LATEST:
      cout << " - QUERY_LATEST" << endl;
      connection.send_text(responseLatestMsg(""));
      break;
    //Un peer ha richiesto la blockchain, essa viene inserita nella risposta
    case QUERY_ALL:
      cout << " - QUERY_ALL" << endl;
      connection.send_text(responseChainMsg());
      break;
    //Un peer ha inviato la propria versione di blockchain
    case RESPONSE_BLOCKCHAIN:
      cout << " - RESPONSE_BLOCKCHAIN" << endl;
      if(document["data"].IsNull()){
        cout << "ERROR (handleServerMessage - RESPONSE_BLOCKCHAIN): Nessun dato ricevuto" << endl;
        return;
      }
      try{
        receivedBlocks = parseBlockList(document["data"]);
        Peer::getInstance().handleBlockchainResponse(receivedBlocks);
      }catch(const char* msg){
        cout << msg << endl;
        cout << "EXCEPTION (handleServerMessage - RESPONSE_BLOCKCHAIN): Errore durante l'elaborazione' del messaggio!" << endl;
        return;
      }
      /*Aggiornamento dei dati relativi al tempo di mining del blocco (questo campo
       è non nullo solo se siamo in corrispondenza della diffuzione di un nuovo
        blocco che è stato minato ed aggiunto alla blockchain)*/
      if(!document["stat"].IsNull()){
        ofstream myfile;
        myfile.open ("blocksminingtime.txt", ios::out | ios::app);
        if(myfile.is_open()) {
          myfile << document["stat"].GetString();
        } else {
          cout << "ERRORE (handleServerMessage - RESPONSE_BLOCKCHAIN): non è stato possibile aprire il file per salvare il tempo di mining del blocco!";
        }
        myfile.close();
      }
      break;
    //Un peer ha richiesto il transaction pool, esso viene inserito nella risposta
    case QUERY_TRANSACTION_POOL:
      cout << " - QUERY_TRANSACTION_POOL" << endl;
      connection.send_text(responseTransactionPoolMsg());
      break;
    //Un peer ha inviato la propria versione di transaction pool
    case RESPONSE_TRANSACTION_POOL:
      cout << " - RESPONSE_TRANSACTION_POOL" << endl;

      if(document["data"].IsNull()){
        cout << "ERRORE (handleServerMessage - RESPONSE_TRANSACTION_POOL) nessun dato ricevuto!" << endl;
        return;
      }
      try{
        receivedTransactions = parseTransactionVector(document["data"]);
      }catch(const char* msg){
        cout << msg << endl;
        cout << "ERRORE (handleServerMessage - RESPONSE_TRANSACTION_POOL): errore durante il parsing del messaggio!" << endl;
        return;
      }
      /*Per ogni transazione questa viene elaborata e successivamente si
      effettua un broadcast del transaction pool aggiornato*/
      for(it = receivedTransactions.begin(); it != receivedTransactions.end(); ++it){
        try{
            BlockChain::getInstance().handleReceivedTransaction(*it);
            broadCastTransactionPool();
        }catch(const char* msg) {
            cout << msg << endl;
            cout << "ERRORE (handleServerMessage - RESPONSE_TRANSACTION_POOL): Errore durante l'inserimento della transazione nel pool" << endl;
        }
      }
      break;
    /*Un peer ha prelevato delle transazioni dal proprio pool,, aggiornamento
    del file contenente le statistiche*/
    case TRANSACTION_POOL_STATS:
      cout << " - TRANSACTION_POOL_STATS" << endl;
      if(!document["data"].IsNull()){
        try{
          myfile.open("transactionwaitingtime.txt", ios::out | ios::app);
          if(myfile.is_open()) {
            myfile << document["data"].GetString();
          } else {
            throw "EXCEPTION (handleServerMessage - TRANSACTION_POOL_STATS): non è stato possibile aprire il file per salvare le statistiche di attesa delle transazioni!";
          }
        }catch(const char* msg) {
            cout << msg << endl;
            cout << "ERRORE (handleServerMessage - TRANSACTION_POOL_STATS): Errore durante l'apertura del file per aggiornare le statistiche" << endl;
        }
        myfile.close();
      }
      break;
    default:
    cout << "ERRORE (handleServerMessage): Il messaggio non ha un tipo valido!" << endl;
  }
}

/*Controllo della validità del tipo di messaggio (se appartiene all'enumeratore)*/
bool Peer::isValidType(int type){
  MessageType receivedType = static_cast<MessageType>(type);
  for ( int val = QUERY_LATEST; val != TRANSACTION_POOL_STATS+1; val++ ){
    if(receivedType == val)return true;
  }
  return false;
}

/*Questo metodo effettua un broadcast di un certo messaggio, per fare ciò lo invia
 su tutte le socket aperte dal thread client e su tutte quelle aperte da altri
 client verso il thread server*/
void Peer::broadcast(string message){
  //Broadcast sulle socket aperte dal thread client
  for(auto ws: openedConnections){
    if(ws->getReadyState() != WebSocket::CLOSED) {
      ws->send(message);
    }
  }
  //Broadcast sulle socket aperte da altri client verso il thread server
  for(auto ws: receivedConnections){
    ws->send_text(message);
  }
}

/*Metodi per il broadcast dei messaggi*/
void Peer::broadCastTransactionPool(){
  broadcast(responseTransactionPoolMsg());
}
void Peer::broadcastLatest(string stat){
  broadcast(responseLatestMsg(stat));
}

void Peer::broadcastTxPoolStat(vector<string> stats){
  broadcast(txPoolStatsMessage(stats));
}


/*metodi per la costruzione dei messaggi da inviare sulle socket*/
string Peer::queryChainLengthMsg(){
  return Message(QUERY_LATEST, "").toString();
}
string Peer::queryAllMsg(){
  return Message(QUERY_ALL, "").toString();
}
string Peer::queryTransactionPoolMsg(){
  return Message(QUERY_TRANSACTION_POOL, "").toString();
}
string Peer::responseChainMsg(){
  return Message(RESPONSE_BLOCKCHAIN, BlockChain::getInstance().toString()).toString();
}
string Peer::responseLatestMsg(string stat){
  return Message(RESPONSE_BLOCKCHAIN, "[" + BlockChain::getInstance().getLatestBlock().toString() + "]",  stat).toString();
}
string Peer::responseTransactionPoolMsg(){
  return Message(RESPONSE_TRANSACTION_POOL, TransactionPool::getInstance().toString()).toString();
}
string Peer::txPoolStatsMessage(vector<string> stats){
  string msg = "";
  vector<string>::iterator it;
  for(it = stats.begin(); it != stats.end(); ++it){
    msg = msg + *it + "\n";
  }
  return Message(TRANSACTION_POOL_STATS, msg).toString();
}
