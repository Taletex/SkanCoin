#ifndef __TRANSACTIONPOOL_DEFINITION__
#define __TRANSACTIONPOOL_DEFINITION__

#include "Transactions.hpp"
#include <list>

class TransactionStat {
  public:
    std::string transactionId;
    std::chrono::high_resolution_clock::time_point insertionTimestamp;

    TransactionStat(std::string transactionId);
    long getDiffTime();
    std::string getDiffTimeString();
};

class TransactionPool {
  public:
    static TransactionPool& getInstance() {
       static TransactionPool tp;
       return tp;
    }
    std::vector<Transaction> getTransactionPool();
    bool addToTransactionPool(Transaction tx, std::vector<UnspentTxOut> unspentTxOuts);
    void updateTransactionPool(std::vector<UnspentTxOut> unspentTxOuts);
    std::string toString();
    std::vector<std::string> getStatStrings();
  private:
    TransactionPool();
    TransactionPool(const TransactionPool&) = delete;
    TransactionPool& operator=(const TransactionPool&) = delete;
    std::list<Transaction> unconfirmedTransactions;      // List of unconfirmed transaction of the node
    std::list<TransactionStat> stats;                   //Lista dei tempi di ricezione delle transazioni nel pool
    bool hasTxIn(TxIn txIn, std::vector<UnspentTxOut> unspentTxOuts);
    std::vector<TxIn> getTxPoolIns();
    bool isValidTxForPool(Transaction tx);
};

#endif
