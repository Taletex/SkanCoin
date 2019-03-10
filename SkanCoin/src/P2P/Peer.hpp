#ifndef __P2P_SERVER_DEFINITION__
#define __P2P_SERVER_DEFINITION__

#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_set>
#include "document.h"
#include "easywsclient.hpp"
#include "config.hpp"
#include "../Blockchain/Block.hpp"
#include "../Blockchain/Blockchain.hpp"
#include "../Blockchain/Transactions.hpp"
#include "../Blockchain/TransactionPool.hpp"
#include "../HttpServer/HttpServer.hpp"

/*I tipi di messaggi che i peer possono scambiarsi*/
enum MessageType {
    QUERY_LATEST_BLOCK = 0,
    QUERY_BLOCKCHAIN = 1,
    RESPONSE_BLOCKCHAIN = 2,
    QUERY_POOL = 3,
    RESPONSE_POOL = 4,
    POOL_STATS = 5,
    RESPONSE_LATEST = 6
};

/*Questa classe modella un messaggio di un peer ed ha un metodo toString
che permette di inviarlo su una web socket*/
class Message {
  public:
    MessageType type;
    std::string data;
    int index;
    double duration;
    std::string address;
    bool isLast;
    Message(MessageType type, std::string data);
    Message(MessageType type, std::string data, int index, double duration);
    Message(MessageType type, std::string data, std::string address, bool isLast);
    std::string toString();
};

class OncomingChain {
public:
    std::list<Block> blocks;
    std::string address;

    OncomingChain(std::string address){
        this->blocks = {};
        this->address = address;
    }

    std::list<Block>getBlocks() {
        return blocks;
    }

    void addBlock(Block b){
        this->blocks.push_back(b);
    }

    const std::string getAddress(){
        return address;
    }
};

/*Questa è una classe Singleton, il suo ruolo è gestire le connessioni verso gli altri peer della rete, gestendo in messaggi in arrivo e le operazioni richieste dall'utente*/
class Peer {
  public:
    /*Usiamo un mutex perchè le liste usate per gestire le connessioni ricevute e
    quelle aperte non possono essere usate da thread diversi contemporaneamente
    (e noi abbiamo bisogno di usare due thread differenti per gestire questi due tipi di connessione) */
    std::mutex connectionsMtx;
    /*Lista di connessioni aperte dal thread client*/
    std::vector<easywsclient::WebSocket::pointer> openedConnections;
    /*Lista di connessioni ricevute dal thread server*/
    std::unordered_set<crow::websocket::connection*> receivedConnections;
    /*Questa è una variabile di supporto usata dall'handler dei messaggi in arrivo
     per gestire un evento per una socket secifica (vedi spiegazione più approfondita
     in corrispondenza dell'implementazione del metodo - clientMessageHandler)*/
    easywsclient::WebSocket::pointer tempClientWs;
    /*
     * Per poter utilizzare lo stesso metodo anche per i messaggi ricevuti dal thread server, utilizziamo lo stesso pattern
     * con riferimento alla socket corrente, in questo modo sarà possibile gestire i due casi utilizzando la stessa interfaccia
     * per il metodo. Questa scelta è stata fatta in quanto (sempre a causa di forzature nelle interfacce richieste dalle
     * librerie) si usano delle reference, tipi che non possono mai puntare ad un oggetto nullo, dunque non è stato possibile
     * discriminare i due casi sulla base del fatto che un campo fosse valorizzato o meno.
     * */
    crow::websocket::connection *tempServerWs;

    /*
     * Questa lista viene utilizzata per collezionare i blocchi ricevuti dai vari peer che stanno inviando la propria
     * versione di blockchain, infatti non è possibile inviare la blockchain tutta in un messaggio in quanto se questa
     * è troppo grande potrebbero verificarsi dei malfunzionamenti nelle socket client
     * */
    std::list<OncomingChain> oncomingChains;

    /*Metodo getInstance per l'implementazione del pattern Singleton*/
    static Peer& getInstance() {
       static Peer peer;
       return peer;
    }

    /* Business logic per un messaggio di tipo RESPONSE_BLOCKCHAIN, questa funzione
    è chiamata nell'handler dei messaggi in arrivo. Ritorna true se ha aggiunto un
    singolo blocco nella blockchain, altrimenti false (anche in caso di replace) */
    bool newBlocksHandler(std::list<Block> receivedBlocks);

    /*Questo metodo effettua un broadcast di un certo messaggio, per fare ciò lo invia
     su tutte le socket aperte dal thread client e su tutte quelle aperte da altri
     client verso il thread server*/
    void broadcast(std::string message);

    /*Metodi per il broadcast dei messaggi*/
    void broadcastLatestBlock(int index, double duration);
    void broadCastPool();
    void broadcastPoolStat(std::vector<std::string>);
    void broadcastQueryPool();

    /*Dato l'url di un server P2P viene aperta una nuova connessione verso di esso dal thread client*/
    void connectToPeer(std::string peer);

    /*Ritorna il numero d peer (si considerano sia le connessioni aperte dal thread client che quelle ricevute dal thread server)*/
    int countPeers();

    /*Questo metodo viene usato per collezionare i blocchi in arrivo da un peer che sta inviando la propria versione
     * di blockchain. Quando questa viene interamente collezionata viene invocato il metodo per confrontarla con
     * quella locale e scegliere la versione da mantenere*/
    void handleBlockchainResponse(Block b, std::string address, bool isLast);

    /*Inizializzazione del server P2P*/
    void initP2PServer(int port);

    /*Controllo della validità del tipo di messaggio (se appartiene all'enumeratore)*/
    bool isValidType(int type);

    /*Business logic per la gestione dei messaggi in arrivo dai peer*/
    void peerMessageHandler(const std::string & data, int isServer);

    /*metodi per la costruzione dei messaggi da inviare sulle socket*/
    std::string poolStatsMessage(std::vector<std::string> stats);
    std::string queryBlockchainMsg();
    std::string queryLatestBlockMsg();
    std::string queryPoolMsg();
    std::string responseBlockchainMsg(Block b);
    std::string responseLatestBlockMsg(int index, double duration);
    std::string responsePoolMsg(Transaction t);

    /*Invio della BlockChain verso la socket temporanea, i blocchi vengono inviati ad uno ad uno per evitare
     *lo scambio di messaggi troppo grandi che possono causare malfunzionamenti nelle socket client*/
    void sendBlockChain(int isServer);

    /*Invio della transaction pool verso la socket temporanea, le transazioni vengono inviate ad una ad una per evitare
     *lo scambio di messaggi troppo grandi che possono causare malfunzionamenti nelle socket client*/
    void sendPool(int isServer);

    /*Avvio del polling del client sulle socket aperte*/
    void startClientPoll();
  private:
    /*Il pattern singleton viene implementato rendendo il costruttore di default privato
    ed eliminando il costruttore di copia e l'operazione di assegnamento*/
    Peer(){}
    Peer(const Peer&) = delete;
    Peer& operator=(const Peer&) = delete;

    /* Questa funzione esegue il polling sulla lista delle connessioni aperte dal
    thread client, se ci sono nuovi messaggi ricevuti viene eseguito l'handler adatto*/
    void checkReceivedMessage();

    /*Questo metodo controlla il vettore di socket (client), eliminando quelle
    che sono state chiuse*/
    void clearClosedWs();
};

/*Gestore per messaggi in arrivo dalle socke aperte dal thread client (easywsclient::WebSocket)
Questo metodo deve avere un solo parametro (const string & data), questo è il motivo per cui
abbiamo bisogno di una variabile esterna di supporto (tempClientWs) che sia un
riferimento alla socket corrente. Sempre per rispettare l'interfaccia richiesta
dalla libreria esso non può essere membro della classe Peer in quanto è
utilizzato come handler per i messaggi in arrivo dalle socket*/
void clientMessageHandler(const std::string & message);

#endif


/* Definizione della macro BOOST_SYSTEM_NO_DEPRECATED per risolvere il bug
   boost::system::throws della libreria Boost
   http://boost.2283326.n4.nabble.com/Re-Boost-build-System-Link-issues-using-BOOST-ERROR-CODE-HEADER-ONLY-td4688963.html
   https://www.boost.org/doc/libs/1_56_0/libs/system/doc/reference.html#Header-
*/
#ifndef BOOST_SYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED
#endif
