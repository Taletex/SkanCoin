#ifndef __TRANSACTIONPOOL_DEFINITION__
#define __TRANSACTIONPOOL_DEFINITION__

#include "Transactions.hpp"
#include <list>

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
  private:
    TransactionPool();
    TransactionPool(const TransactionPool&) = delete;
    TransactionPool& operator=(const TransactionPool&) = delete;
    std::list<Transaction> unconfirmedTransactions;      // List of unconfirmed transaction of the node
    bool hasTxIn(TxIn txIn, std::vector<UnspentTxOut> unspentTxOuts);
    std::vector<TxIn> getTxPoolIns();
    bool isValidTxForPool(Transaction tx);
};

#endif
