#include "TransactionPool.hpp"

using namespace std;

/* Returns the list of unconfirmed transaction of the transaction pool */
list<Transaction> TransactionPool::getTransactionPool() {
  return this->unconfirmedTransactions;
}

/* Adds a transaction to the transaction pool (if this transaction is valid ). */
bool TransactionPool::addToTransactionPool(Transaction tx, vector<UnspentTxOut> unspentTxOuts) {
  if(!validateTransaction(tx, unspentTxOuts)) {
    return false;     // Error: Trying to add invalid transaction to transaction pool!
  }

  if (!isValidTxForPool(tx)) {
    return false;     // Error: Trying to add invalid transaction to transaction pool!
  }

  cout << "Adding to transaction pool: " << tx.toString() << endl;
  unconfirmedTransactions.push_back(tx);

  return true;
}

/* Aggiorna la transaction pool eliminando le transazioni non valide*/
void TransactionPool::updateTransactionPool(vector<UnspentTxOut> unspentTxOuts) {
  list<Transaction>::iterator it1;
  vector<TxIn>::iterator it2;

  for(it1 = unconfirmedTransactions.begin(); it1 != unconfirmedTransactions.end(); ++it1)
    for(it2 = it1->txIns.begin(); it2 != it1->txIns.end(); ++it2) {
      if(!hasTxIn(*it2, unspentTxOuts)) {
        unconfirmedTransactions.erase(it1);
        cout << "Removing the following transaction from transaction pool: " << it2->toString();
        break;
      }
    }
  }

/* verifica se il TxIn passato come primo parametro è presente nell'array passato come secondo */
bool TransactionPool::hasTxIn(TxIn txIn, vector<UnspentTxOut> unspentTxOuts) {
  vector<UnspentTxOut>::iterator it;
  for(it = unspentTxOuts.begin(); it != unspentTxOuts.end(); ++it){
    if(it->txOutId == txIn.txOutId && it->txOutIndex == txIn.txOutIndex){
      return true;
    }
  }
  return false;
}

/* Returns a vector of TxIn finded in the unconfirmedTransactions of transaction pool */
vector<TxIn> TransactionPool::getTxPoolIns() {
  vector<TxIn> txIns;
  list<Transaction>::iterator it;

  for(it = unconfirmedTransactions.begin(); it != unconfirmedTransactions.end(); ++it){
    txIns.reserve(txIns.size() + it->txIns.size());
    copy(it->txIns.begin(), it->txIns.end(), txIns.end());
  }
}

/* Returns true if the transaction passed as param is not present in the unconfirmedTransactions of transaction pool, else false */
bool TransactionPool::isValidTxForPool(Transaction tx) {
  vector<TxIn>::iterator it1;
  vector<TxIn>::iterator it2;
  vector<TxIn> txPoolIns = getTxPoolIns();

  for(it1 = tx.txIns.begin(); it1 != tx.txIns.end(); ++it1) {
    for(it2 = txPoolIns.begin(); it2 != txPoolIns.end(); ++it2) {
      if(it1->isEqual(*it2)) {
        cout << "Transaction input already found in the transaction pool!" << endl;
        return false;
      }
    }
  }

  return true;
}
