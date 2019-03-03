#ifndef __P2P_SERVER_DEFINITION__
#define __P2P_SERVER_DEFINITION__
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_set>
#include "../HttpServer/HttpServer.hpp"
#include "document.h"
#include "easywsclient.hpp"

//Possible types of messages exchanged between the peers
enum MessageType {
    QUERY_LATEST = 0,
    QUERY_ALL = 1,
    RESPONSE_BLOCKCHAIN = 2,
    QUERY_TRANSACTION_POOL = 3,
    RESPONSE_TRANSACTION_POOL = 4,
    TRANSACTION_POOL_STATS = 5
};

//This class models a peer message and contains a toString method used to send it in a websocket
class Message {
  public:
    MessageType type;
    std::string data;
    std::string stat;
    Message(MessageType type, std::string data);
    Message(MessageType type, std::string data, std::string stat);
    std::string toString();
};

/*Questa è una classe Singleton, il suo ruolo è gestire le connessioni verso gli altri peer della rete, gestendo in messaggi in arrivo e le operazioni richieste dall'utente*/
class Peer {
  public:
    /*Usiamo un mutex perchè le liste usate per gestire le connessioni ricevute e
    quelle aperte non possono essere usate da thread diversi contemporaneamente
    (e noi abbiamo bisogno di usare due thread differenti per gestire questi due tipi di connessione) */
    std::mutex connectionsMtx;
    /*Lista di connessioni ricevute dal thread server*/
    std::unordered_set<crow::websocket::connection*> receivedConnections;
    /*Lista di connessioni aperte dal thread client*/
    std::vector<easywsclient::WebSocket::pointer> openedConnections;
    /*Questa è una variabile di supporto usata dall'handler dei messaggi in arrivo
     per gestire un evento per una socket secifica (vedi spiegazione più approfondita
       in corrispondenza dell'implementazione del metodo - handleClientMessage)*/
    easywsclient::WebSocket::pointer tempWs;

    /*Metodo getInstance per l'implementazione del pattern Singleton*/
    static Peer& getInstance() {
       static Peer peer;
       return peer;
    }

    /*Ritorna il numero d peer (si considerano sia le connessioni aperte dal thread client che quelle ricevute dal thread server)*/
    int countPeers();

    /*Controllo della validità del tipo di messaggio (se appartiene all'enumeratore)*/
    bool isValidType(int type);

    /*metodi per la costruzione dei messaggi da inviare sulle socket*/
    std::string queryChainLengthMsg();
    std::string queryAllMsg();
    std::string queryTransactionPoolMsg();
    std::string responseChainMsg();
    std::string responseLatestMsg(std::string stat);
    std::string responseTransactionPoolMsg();
    std::string txPoolStatsMessage(std::vector<std::string> stats);

    /*Inizializzazione del server P2P*/
    void initP2PServer(int port);

    /*Metodi per il broadcast dei messaggi*/
    void broadCastTransactionPool();
    void broadcastLatest(std::string stat);
    void broadcastTxPoolStat(std::vector<std::string>);

    /*Avvio del polling del client sulle socket aperte*/
    void startClientPoll();

    /*Dato l'url di un server P2P viene aperta una nuova connessione verso di esso dal thread client*/
    void connectToPeers(std::string peer);

    /*Business logic per un messaggio di tipo RESPONSE_BLOCKCHAIN, questa funzione
    è chiamata nell'handler dei messaggi in arrivo*/
    void handleBlockchainResponse(std::list<Block> receivedBlocks);

    /*Questo metodo effettua un broadcast di un certo messaggio, per fare ciò lo invia
     su tutte le socket aperte dal thread client e su tutte quelle aperte da altri
     client verso il thread server*/
    void broadcast(std::string message);

  private:
    /*Il pattern singleton viene implementato rendendo il costruttore di default privato
    ed eliminando il costruttore di copia e l'operazione di assegnamento*/
    Peer(){}
    Peer(const Peer&) = delete;
    Peer& operator=(const Peer&) = delete;

    /*Questo metodo controlla il vettore di socket (client), eliminando quelle
    che sono state chiuse*/
    void clearClosedWs();

    /*Gestore dei messaggi in arrivo al Server Peer*/
    void handleServerMessage(crow::websocket::connection& connection, const std::string& data);

    /* Questa funzione esegue il polling sulla lista delle connessioni aperte dal
    thread client, se ci sono nuovi messaggi ricevuti viene eseguito l'handler adatto*/
    void checkReceivedMessage();
};

/*Gestore per messaggi in arrivo dalle socke aperte dal thread client (easywsclient::WebSocket)
Questo metodo deve avere un solo parametro (const string & data), questo è il motivo per cui
abbiamo bisogno di una variabile esterna di supporto (tempWs) che sia un
riferimento alla socket corrente. Sempre per rispettare l'interfaccia richiesta
dalla libreria esso non può essere membro della classe Peer in quanto è
utilizzato come handler per i messaggi in arrivo dalle socket*/
void handleClientMessage(const std::string & message);

#endif
