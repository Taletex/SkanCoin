#include "Transactions.hpp"

#ifndef __TRANSACTIONPOOL_DEFINITION__
#define __TRANSACTIONPOOL_DEFINITION__

using namespace std;

/* TODO: VERIFICARE SE FARE UN SINGLETON PER LA TRANSACTION POOL E SE SERVE ESPORRE LA GETTRANSACTIONPOOL E SE SERVE PASSARE AI METODI PUBBLICI LA TRANSACTIONPOOL LIST (Unconfirmed transaction)*/
class TransactionPool {
  public:
    TransactionPool();

    vector<Transaction> getTransactionPool();
    bool addToTransactionPool(Transaction tx, vector<UnspentTxOut> unspentTxOuts);
    void updateTransactionPool(vector<UnspentTxOut> unspentTxOuts);

  private:
    list<Transaction> unconfirmedTransactions;      // List of unconfirmed transaction of the node
    bool hasTxIn(TxIn txIn, vector<UnspentTxOut> unspentTxOuts);
    vector<TxIn> getTxPoolIns();
    bool isValidTxForPool(Transaction tx);
};

#endif
