#ifndef __TRANSACTIONPOOL_DEFINITION__
#define __TRANSACTIONPOOL_DEFINITION__

#include <list>
#include <chrono>
#include "config.hpp"
#include "TransactionComponents.hpp"
#include "Transactions.hpp"

class TransactionStat {
  public:
    std::chrono::high_resolution_clock::time_point insertionTimestamp;
    std::string transactionId;

    /*Costruttore per un elemento del vettore contenente il tempo di inserimento
    della transazione del pool (per la valutazione del tempo di attesa per la conferma)*/
    TransactionStat(std::string transactionId);

    /*Ritorna il numero di millisecondi trascorsi dall'inserimento della transazione nel pool*/
    long getDiffTime();

    /*Ritorna la stringa da inserire nel file relativo alla statistica del
    tempo di attesa nel pool, per la transazione che sta per essere prelevata dal pool*/
    std::string getDiffTimeString();
};

class TransactionPool {
  public:
    /*Metodo getInstance per l'implementazione del pattern Singleton*/
    static TransactionPool& getInstance() {
       static TransactionPool tp;
       return tp;
    }

    /*Ritorna la lista delle transazioni non confermate nel nodo
    (quelle contenute nel transaction pool) */
    std::vector<Transaction> getPool();

    /* Verifica la validità della transazione data e la inserisce nel transaction pool */
    bool addToPool(Transaction transaction, std::vector<UnspentTransOut> unspentTransOuts);

    /*Data una transazione ritorna il relativo elemento del vettore contenente
     i dati relativi ai tempi di attesa delle transazioni nel pool */
    std::string getStatString(std::string transactionId);

    /*Rappresentazione in formato stringa del pool di transazioni*/
    std::string toString();

    /* Aggiorna la transaction pool eliminando le transazioni non valide*/
    void updatePool(std::vector<UnspentTransOut> unspentTransOuts);
  private:
    /*Il pattern singleton viene implementato rendendo il costruttore di default privato
    ed eliminando il costruttore di copia e l'operazione di assegnamento*/
    TransactionPool();
    TransactionPool(const TransactionPool&) = delete;
    TransactionPool& operator=(const TransactionPool&) = delete;

    //Lista delle transazioni non confermate nel nodo
    std::list<Transaction> unconfirmedTransactions;
    //Lista dei tempi di ricezione delle transazioni nel pool (per la raccolta di statistiche)
    std::list<TransactionStat> stats;

    /*Rimuove dal vettore di TransactionStat l'elemento corrispondente all'id di transazione indicato*/
    void deleteStat(std::string transactionId);

    /*Ritorna un vettore contentente tutti gli input di transazione contenuti
    nelle transazioni presenti nel transaction pool */
    std::vector<TransIn> getPoolIns();

    /*Verifica se il TransIn passato come primo parametro è presente nell'array passato come secondo */
    bool hasTransIn(TransIn transIn, std::vector<UnspentTransOut> unspentTransOuts);

    /*Ritorna true se la transazione data non contiene input già presenti nel
     transaction pool (indicati in altre transazioni), altrimenti false*/
    bool isValidTransForPool(Transaction transaction);
};

#endif
