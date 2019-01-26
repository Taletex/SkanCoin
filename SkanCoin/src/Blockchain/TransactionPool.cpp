#include "TransactionPool.hpp"

using namespace std;

TransactionPool::TransactionPool(){
  unconfirmedTransactions = {};
}

/* Returns the list of unconfirmed transaction of the transaction pool */
vector<Transaction> TransactionPool::getTransactionPool() {
  return { begin(this->unconfirmedTransactions), end(this->unconfirmedTransactions) };
}

string TransactionPool::toString(){
  string ret = "[";
  list<Transaction>::iterator it;
  for(it = unconfirmedTransactions.begin(); it != unconfirmedTransactions.end(); ++it){
    if(it != unconfirmedTransactions.begin()){
      ret = ret + ", ";
    }
    ret = ret + it->toString();
  }
  ret = ret + "]";
  return ret;
}

/* Adds a transaction to the transaction pool (if this transaction is valid ). */
bool TransactionPool::addToTransactionPool(Transaction tx, vector<UnspentTxOut> unspentTxOuts) {
  if(!validateTransaction(tx, unspentTxOuts) || !isValidTxForPool(tx)) {
    cout << endl;
    throw "EXCEPTION: Trying to add invalid transaction to transaction pool!";
  }
  cout << "Adding to transaction pool: " << tx.toString() << endl;
  unconfirmedTransactions.push_back(tx);

  return true;
}

/* Aggiorna la transaction pool eliminando le transazioni non valide*/
void TransactionPool::updateTransactionPool(vector<UnspentTxOut> unspentTxOuts) {
  list<Transaction>::iterator it1;
  vector<TxIn>::iterator it2;

  for(it1 = unconfirmedTransactions.begin(); it1 != unconfirmedTransactions.end(); ++it1) {
    for(it2 = it1->txIns.begin(); it2 != it1->txIns.end(); ++it2) {
      if(!hasTxIn(*it2, unspentTxOuts)) {
        cout << "Removing the following transaction from transaction pool: " << it1->toString();
        unconfirmedTransactions.erase(it1);
        break;
      }
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
    txIns.insert(txIns.end(), it->txIns.begin(), it->txIns.end());
  }

  return txIns;
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
