#ifndef __TRANSACTIONPOOL_DEFINITION__
#define __TRANSACTIONPOOL_DEFINITION__

#include "Transactions.hpp"
#include <list>

class TransactionStat {
  public:
    std::string transactionId;
    std::chrono::high_resolution_clock::time_point insertionTimestamp;

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
    static TransactionPool& getInstance() {
       static TransactionPool tp;
       return tp;
    }
    /*Rappresentazione in formato stringa del pool di transazioni*/
    std::string toString();

    /*Ritorna la lista delle transazioni non confermate nel nodo
    (quelle contenute nel transaction pool) */
    std::vector<Transaction> getTransactionPool();

    /* Verifica la validità della transazione data e la inserisce nel transaction pool */
    bool addToTransactionPool(Transaction tx, std::vector<UnspentTxOut> unspentTxOuts);

    /* Aggiorna la transaction pool eliminando le transazioni non valide*/
    void updateTransactionPool(std::vector<UnspentTxOut> unspentTxOuts);

    /*Ritorna un vettore di stringhe, che sono le entries da inserire nell'apposito
    file per raccogliere le statistiche relative al tempo di attesa
    delle transazioni nel pool prima di essere confermate*/
    std::vector<std::string> getStatStrings();
  private:
    TransactionPool();
    TransactionPool(const TransactionPool&) = delete;
    TransactionPool& operator=(const TransactionPool&) = delete;
    //Lista delle transazioni non confermate nel nodo
    std::list<Transaction> unconfirmedTransactions;
    //Lista dei tempi di ricezione delle transazioni nel pool (per la raccolta di statistiche)
    std::list<TransactionStat> stats;

    /*Verifica se il TxIn passato come primo parametro è presente nell'array passato come secondo */
    bool hasTxIn(TxIn txIn, std::vector<UnspentTxOut> unspentTxOuts);

    /*Ritorna un vettore contentente tutti gli input di transazione contenuti
    nelle transazioni presenti nel transaction pool */
    std::vector<TxIn> getTxPoolIns();

    /*Ritorna true se la transazione data non contiene input già presenti nel
     transaction pool (indicati in altre transazioni), altrimenti false*/
    bool isValidTxForPool(Transaction tx);

    /*Rimuove dal vettore di TransactionStat l'elemento corrispondente all'id di transazione indicato*/
    void deleteStat(std::string transactionId);
};

#endif
