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
vector<Transaction> TransactionPool::getPool() {
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
bool TransactionPool::addToPool(Transaction transaction, vector<UnspentTransOut> unspentTransOuts) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if(!isValidTransaction(transaction, unspentTransOuts) || !isValidTransForPool(transaction)) {
    throw "INFO (addToPool): La transazione che si vuole inserire nel pool è già presente o non è valida...";
  }
  cout << "Nuova transazione aggiunta al pool: " << endl;
  cout << "La transaction pool contiene " << TransactionPool::getInstance().getPool().size()+1 << " transazioni" << endl;
  unconfirmedTransactions.push_back(transaction);
  stats.push_back(TransactionStat(transaction.id));
  return true;
}

/* Aggiorna la transaction pool eliminando le transazioni non valide*/
void TransactionPool::updatePool(vector<UnspentTransOut> unspentTransOuts) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  list<Transaction> aux;
  list<Transaction>::iterator it1;
  vector<TransIn>::iterator it2;
  cout << "Aggiornamento del transaction pool i corso..." << endl;
  for(it1 = unconfirmedTransactions.begin(); it1 != unconfirmedTransactions.end(); ) {
    for(it2 = it1->transIns.begin(); it2 != it1->transIns.end(); ++it2) {
      if(!hasTransIn(*it2, unspentTransOuts)) {
        deleteStat(it1->id);
        cout << "Una transazione è stata rimossa dal pool: il transaction pool adesso contiene " << unconfirmedTransactions.size() << " transazioni..." << endl;
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

/*Verifica se il TransIn passato come primo parametro è presente nell'array passato come secondo */
bool TransactionPool::hasTransIn(TransIn transIn, vector<UnspentTransOut> unspentTransOuts) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<UnspentTransOut>::iterator it;
  for(it = unspentTransOuts.begin(); it != unspentTransOuts.end(); ++it){
    if(it->transOutId == transIn.transOutId && it->transOutIndex == transIn.transOutIndex){
      return true;
    }
  }
  return false;
}

/*Ritorna un vettore contentente tutti gli input di transazione contenuti
nelle transazioni presenti nel transaction pool */
vector<TransIn> TransactionPool::getPoolIns() {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<TransIn> transIns;
  list<Transaction>::iterator it;
  for(it = unconfirmedTransactions.begin(); it != unconfirmedTransactions.end(); ++it){
    transIns.reserve(transIns.size() + it->transIns.size());
    transIns.insert(transIns.end(), it->transIns.begin(), it->transIns.end());
  }
  return transIns;
}

/*Ritorna true se la transazione data non contiene input già presenti nel
 transaction pool (indicati in altre transazioni), altrimenti false*/
bool TransactionPool::isValidTransForPool(Transaction transaction) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<TransIn>::iterator it1;
  vector<TransIn>::iterator it2;
  vector<TransIn> transPoolIns = getPoolIns();

  for(it1 = transaction.transIns.begin(); it1 != transaction.transIns.end(); ++it1) {
    for(it2 = transPoolIns.begin(); it2 != transPoolIns.end(); ++it2) {
      if(it1->isEqual(*it2)) {
        cout << "INFO (isValidTransForPool): L'input di transazione è già stato speso in una delle transazioni presenti nel pool!" << endl;
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

  return "{\"transactionId\": \"" + this->transactionId + "\", \"millisWaitTime\": " + to_string((this->getDiffTime())/1000000) + "}";
}
