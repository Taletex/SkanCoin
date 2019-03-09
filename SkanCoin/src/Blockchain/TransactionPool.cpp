#include "TransactionPool.hpp"

using namespace std;

TransactionPool::TransactionPool(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  unconfirmedTransactions = {};
  stats = {};
}

/*Ritorna la lista delle transazioni non confermate nel nodo
(quelle contenute nel transaction pool) */
vector<Transaction> TransactionPool::getTransactionPool() {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  //Conversione da lista a vettore per una gestione più efficiente all'esterno della classe
  return { begin(this->unconfirmedTransactions), end(this->unconfirmedTransactions) };
}

/*Rappresentazione in formato stringa del pool di transazioni*/
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

/* Verifica la validità della transazione data e la inserisce nel transaction pool */
bool TransactionPool::addToTransactionPool(Transaction tx, vector<UnspentTxOut> unspentTxOuts) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if(!validateTransaction(tx, unspentTxOuts) || !isValidTxForPool(tx)) {
    cout << endl;
    throw "EXCEPTION (addToTransactionPool): La transazione che si vuole inserire nel pool non è valida!";
  }
  cout << "Nuova transazione aggiunta al pool: " << tx.toString() << endl;
  cout << "La transaction pool contiene " << TransactionPool::getInstance().getTransactionPool().size()+1 << " transazioni" << endl;
  unconfirmedTransactions.push_back(tx);
  stats.push_back(TransactionStat(tx.id));
  return true;
}

/* Aggiorna la transaction pool eliminando le transazioni non valide*/
void TransactionPool::updateTransactionPool(vector<UnspentTxOut> unspentTxOuts) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  list<Transaction> aux;
  list<Transaction>::iterator it1;
  vector<TxIn>::iterator it2;

  for(it1 = unconfirmedTransactions.begin(); it1 != unconfirmedTransactions.end(); ) {
    for(it2 = it1->txIns.begin(); it2 != it1->txIns.end(); ++it2) {
      if(!hasTxIn(*it2, unspentTxOuts)) {
        deleteStat(it1->id);
        cout << "Rimozione dalla transaction pool della transazione: " << it1->toString() << endl;
        it1 = unconfirmedTransactions.erase(it1);
        break;
      } else {
        ++it1;
      }
    }
  }
}

/*Rimuove dal vettore di TransactionStat l'elemento corrispondente all'id di transazione indicato*/
void TransactionPool::deleteStat(string transactionId) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  list<TransactionStat>::iterator it;
  for(it = stats.begin(); it != stats.end(); ++it) {
    if((it->transactionId.compare(transactionId))==0){
      stats.erase(it);
      return;
    }
  }
}

/*Verifica se il TxIn passato come primo parametro è presente nell'array passato come secondo */
bool TransactionPool::hasTxIn(TxIn txIn, vector<UnspentTxOut> unspentTxOuts) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<UnspentTxOut>::iterator it;
  for(it = unspentTxOuts.begin(); it != unspentTxOuts.end(); ++it){
    if(it->txOutId == txIn.txOutId && it->txOutIndex == txIn.txOutIndex){
      return true;
    }
  }
  return false;
}

/*Ritorna un vettore contentente tutti gli input di transazione contenuti
nelle transazioni presenti nel transaction pool */
vector<TxIn> TransactionPool::getTxPoolIns() {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<TxIn> txIns;
  list<Transaction>::iterator it;
  for(it = unconfirmedTransactions.begin(); it != unconfirmedTransactions.end(); ++it){
    txIns.reserve(txIns.size() + it->txIns.size());
    txIns.insert(txIns.end(), it->txIns.begin(), it->txIns.end());
  }
  return txIns;
}

/*Ritorna true se la transazione data non contiene input già presenti nel
 transaction pool (indicati in altre transazioni), altrimenti false*/
bool TransactionPool::isValidTxForPool(Transaction tx) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<TxIn>::iterator it1;
  vector<TxIn>::iterator it2;
  vector<TxIn> txPoolIns = getTxPoolIns();

  for(it1 = tx.txIns.begin(); it1 != tx.txIns.end(); ++it1) {
    for(it2 = txPoolIns.begin(); it2 != txPoolIns.end(); ++it2) {
      if(it1->isEqual(*it2)) {
        cout << "ERRORE (isValidTxForPool): L'input di transazione è già presente in una delle transazioni presenti nel pool!" << it1->toString() << endl;
        return false;
      }
    }
  }

  return true;
}

/*Ritorna un vettore di stringhe, che sono le entries da inserire nell'apposito
file per raccogliere le statistiche relative al tempo di attesa
delle transazioni nel pool prima di essere confermate*/
vector<string> TransactionPool::getStatStrings(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<string> ret = {};
  list<TransactionStat>::iterator it;
  for(it = stats.begin(); it != stats.end(); ++it){
    ret.push_back(it->getDiffTimeString());
  }
  return ret;
}

/*Costruttore per un elemento del vettore contenente il tempo di inserimento
della transazione del pool (per la valutazione del tempo di attesa per la conferma)*/
TransactionStat::TransactionStat(string transactionId){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  this->transactionId = transactionId;
  this->insertionTimestamp = chrono::high_resolution_clock::now();
}

/*Ritorna il numero di millisecondi trascorsi dall'inserimento della transazione nel pool*/
long TransactionStat::getDiffTime(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  chrono::high_resolution_clock::time_point t = chrono::high_resolution_clock::now();
  return chrono::duration_cast<chrono::microseconds>( t - this->insertionTimestamp ).count();
}

/*Ritorna la stringa da inserire nel file relativo alla statistica del
tempo di attesa nel pool, per la transazione che sta per essere prelevata dal pool*/
string TransactionStat::getDiffTimeString(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return "{\"transactionId\": \"" + this->transactionId + "\", \"millisWaitTime\": " + to_string(this->getDiffTime()) + "}";
}
