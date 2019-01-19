#include <vector>
#include <map>
#include <typeinfo>
#include "ecc.h"
#include "picosha2.h"
#include <regex>

#ifndef __TRANSACTION_DEFINITION__
#define __TRANSACTION_DEFINITION__

using namespace std;

const int COINBASE_AMOUNT = 10;

class UnspentTxOut {
  //NOTE: vedi come rendere non modificabili questi campi
  public:
    string txOutId;
    string address;
    int txOutIndex;
    float amount;

    UnspentTxOut() {
      this->txOutId = "";
      this->txOutIndex = -1;
      this->address = "";
      this->amount = -1;
    }

    UnspentTxOut(string txOutId, int txOutIndex, string address, float amount) {
      this->txOutId = txOutId;
      this->txOutIndex = txOutIndex;
      this->address = address;
      this->amount = amount;
    }

    bool isEqual(UnspentTxOut other){
      return (this->txOutId == other.txOutId && this->txOutIndex == other.txOutIndex);
    }
};

class TxIn {
  public:
    string txOutId;
    string signature;
    int txOutIndex;

    TxIn() {
        this->txOutId = "";
        this->signature = "";
        this->txOutIndex = -1;
    }

    TxIn(string txOutId, string signature, int txOutIndex) {
        this->txOutId = txOutId;
        this->signature = signature;
        this->txOutIndex = txOutIndex;
    }

    string toString(){
      return "{'txOutId': " + this->txOutId + ", 'signature': " + this->signature + ", 'txOutIndex': " + to_string(this->txOutIndex) + "}";
    }
};

class TxOut {
  public:
    string address;
    float amount;

    TxOut(){
      this->address = "";
      this->amount = -1;
    }

    TxOut(string address, float amount) {
        this->address = address;
        this->amount = amount;
    }

    string toString(){
      return "{'address': " + this->address + ",\n 'amount': " + to_string(this->amount) + "}";
    }
};

class Transaction {
  public:
    string id;
    vector<TxIn> txIns;
    vector<TxOut> txOuts;

    Transaction(){
      this->id = -1;
      this->txIns = {};
      this->txOuts = {};
    }
    Transaction(string id, vector<TxIn> txIns, vector<TxOut> txOuts){
      this->id = id;
      this->txIns.swap(txIns);
      this->txOuts.swap(txOuts);
    }

    string toString(){
      string ret = "{'Id': " + this->id + ",\n'txIns': [";
      vector<TxIn>::iterator it;
      for(it = this->txIns.begin(); it != this->txIns.end(); ++it){
        ret = ret + it->toString() + ",\n";
      }

      ret = ret + "],\n'txOuts': [";
      vector<TxOut>::iterator it2;
      for(it2 = this->txOuts.begin(); it2 != this->txOuts.end(); ++it2){
        ret = ret + it2->toString() + "\n";
      }

      ret = ret + "]}\n";
      return  ret;
    }
};

//verifica se l'UnspentTxOut passato come primo parametro è presente nell'array passato come secondo
bool isPresentUnspentTxOut(UnspentTxOut find, vector<UnspentTxOut> TxOuts){
  vector<UnspentTxOut>::iterator it;
  for(it = TxOuts.begin(); it != TxOuts.end(); ++it){
    if(it->isEqual(find)){
      return true;
    }
  }
  return false;
}

//ritorna l'id (hash) della transazione, che verrà firmato da chi invia i coin
string getTransactionId (Transaction transaction){
  string txInContent = "";
  string txOutContent = "";
  vector<TxIn>::iterator it;
  for(it = transaction.txIns.begin(); it != transaction.txIns.end(); ++it){
    txInContent = txInContent + it->txOutId + to_string(it->txOutIndex);
  }
  vector<TxOut>::iterator it2;
  for(it2 = transaction.txOuts.begin(); it2 != transaction.txOuts.end(); ++it2){
    txOutContent = txOutContent + it2->address + to_string(it2->amount);
  }
  return picosha2::hash256_hex_string(txInContent + txOutContent);;
};

//controlla se il vettore di input contiene dei duplicati
bool hasDuplicates(vector<TxIn> txIns){
  // Create a map to store the frequency of each element in vector
  map<string, int> countMap;

  // Iterate over the vector and store the frequency of each element in map
  for (auto & txIn : txIns){
  	auto result = countMap.insert(pair<string, int>(txIn.txOutId + to_string(txIn.txOutIndex), 1));
  	if (result.second == false) //l'elemento esiste già
  		result.first->second++;
  }

  for (auto & item : countMap){
  	//esistono elementi duplicati
  	if (item.second > 1){
  		cout <<"duplicate txIn: " << item.first;
      return true;
  	}
  }
  return false;
};

string getPublicKey(string privateKey){
    //TODO: Calcolo chiave pubblica da quella privata (stringa esadecimale), trovare una libreria adatta
    //return keyFromPrivate(aPrivateKey, 'hex').getPublic().encode('hex');
    return "ciao";
};

// valid address is a valid ecdsa public key in the 04 + X-coordinate + Y-coordinate format
bool isValidAddress(string address){
  if(!regex_match (address, regex("^[0][4][a-fA-F0-9]{128}$") )){
    cout << "public key must contain only 130 hex characters and must begin with '04': " << address << endl;
    return false;
  }
  return true;
};


//validazione della struttura dell'input (type check)
//NOTE: cercare un modo migliore per il type checking
bool isValidTxInStructure(TxIn txIn){
    if (typeid(txIn.signature) != typeid(string)) {
        cout << "invalid signature type in txIn" << endl;
        return false;
    } else if (typeid(txIn.txOutId) != typeid(string)) {
        cout << "invalid txOutId type in txIn" << endl;
        return false;
    } else if (typeid(txIn.txOutIndex) != typeid(int)) {
        cout << "invalid txOutIndex type in txIn" << endl;
        return false;
    } else {
        return true;
    }
};

//validazione struttura dell'output (type check)
//NOTE: cercare un modo migliore per il type checking
bool isValidTxOutStructure(TxOut txOut){
    if (typeid(txOut.address) != typeid(string)) {
        cout << "invalid address type in txOut" << endl;
        return false;
    } else if (!isValidAddress(txOut.address)) {
        cout << "invalid TxOut address" << endl;
        return false;
    } else if (typeid(txOut.amount) != typeid(float)) {
        cout << "invalid amount type in txOut" << endl;
        return false;
    } else {
        return true;
    }
};

//validazione della struttura (type check) della transazione e di tutti i suoi input e output
//NOTE: cercare un modo migliore per il type checking
bool isValidTransactionStructure(Transaction transaction){
    if (typeid(transaction.id) != typeid(string)) {
        cout << "transactionId missing" << endl;
        return false;
    }
    if (!(typeid(transaction.txIns) ==  (typeid(vector<TxIn>)))) {
        cout << "invalid txIns type in transaction" << endl;
        return false;
    }
    vector<TxIn>::iterator it;
    for(it = transaction.txIns.begin(); it != transaction.txIns.end(); ++it){
      if(!isValidTxInStructure(*it)){
        return false;
      }
    }

    if (!(typeid(transaction.txOuts) ==  (typeid(vector<TxOut>)))) {
        cout << "invalid txIns type in transaction" << endl;
        return false;
    }
    vector<TxOut>::iterator it2;
    for(it2 = transaction.txOuts.begin(); it2 != transaction.txOuts.end(); ++it2){
      if(!isValidTxOutStructure(*it2)){
        return false;
      }
    }
    return true;
};

//Controlla se l'input fa riferimento ad un output non speso
//verifica la firma della nuova transazione che deve corrispondere alla chiave pubblica che è la destinazione di quell'output
bool validateTxIn(TxIn txIn, Transaction transaction, vector<UnspentTxOut> aUnspentTxOuts){
    UnspentTxOut referencedUTxOut = UnspentTxOut("", -1, "", 0);
    vector<UnspentTxOut>::iterator it;
    for(it = aUnspentTxOuts.begin(); it != aUnspentTxOuts.end(); ++it){
      if(it->txOutId == txIn.txOutId && it->txOutIndex == txIn.txOutIndex){
        referencedUTxOut = *it;
      }
    }
    if (referencedUTxOut.txOutIndex == -1) {
        cout << "referenced txOut not found: " << txIn.toString();
        return false;
    }


    string address = referencedUTxOut.address;
    /*
    TODO: VERIFICA DELLA FIRMA, TROVARE UNA LIBRERIA ADATTA
    if (ecdsa_verify(addr, hash, sign) == 0) {
        cout << "invalid txIn signature: " + txIn.signature + ", txId: " + transaction.id + ", address: " + referencedUTxOut.address;
        return false;
    }
    */
    return true;
};

//Dato l'input controlla il valore dell'output non speso a cui questo fa riferimento (la sua esistenza è gia stata verificata in validateTxIn)
float getTxInAmount(TxIn txIn, vector<UnspentTxOut> aUnspentTxOuts){
  vector<UnspentTxOut>::iterator it;
  for(it = aUnspentTxOuts.begin(); it != aUnspentTxOuts.end(); ++it){
    if(it->txOutId == txIn.txOutId && it->txOutIndex == txIn.txOutIndex){
      return it->amount;
    }
  }
  return 0;
};

//Validazione della transazione (tipi di dati e procedure di sicurezza)
bool validateTransaction(Transaction transaction, vector<UnspentTxOut> aUnspentTxOuts){

    //validazione struttura
    if (!isValidTransactionStructure(transaction)) {
        return false;
    }

    //verifica l'hash per rilevare eventuali modifiche non permesse
    if (getTransactionId(transaction).compare(transaction.id) != 0) {
        cout << "invalid tx id: " << transaction.id << endl;
        return false;
    }

    //validazione inputs (verifica firma e output non spesi di provenienza)
    vector<TxIn>::iterator it;
    for(it = transaction.txIns.begin(); it != transaction.txIns.end(); ++it){
      if(!validateTxIn(*it, transaction, aUnspentTxOuts)){
        cout << "some of the txIns are invalid in tx: " << transaction.id << endl;
        return false;
      }
    }

    //calcolo totale inputs
    float totalTxInValues = 0;
    for(it = transaction.txIns.begin(); it != transaction.txIns.end(); ++it){
      float singleAmount = getTxInAmount(*it, aUnspentTxOuts);
      if(singleAmount == 0){
        return false;
      }
      totalTxInValues = totalTxInValues + singleAmount;
    }

    //calcolo totale outputs
    vector<TxOut>::iterator it2;
    float totalTxOutValues = 0;
    for(it2 = transaction.txOuts.begin(); it2 != transaction.txOuts.end(); ++it2){
      totalTxOutValues = totalTxOutValues + it2->amount;
    }

    //le somme di outputs e inputs devono essere uguali
    if (totalTxOutValues != totalTxInValues) {
        cout << "totalTxOutValues != totalTxInValues in tx: " + transaction.id << endl;
        return false;
    }

    return true;
};


//Validazione della transazione di Coinbase
bool validateCoinbaseTx(Transaction transaction, int blockIndex){

    //controllo che non ci siano modifiche
    if (getTransactionId(transaction) != transaction.id) {
        cout << "invalid coinbase tx id: " << transaction.id << endl;
        return false;
    }
    //la coinbase transaction ha un solo input
    if (transaction.txIns.size() != 1) {
        cout << "one txIn must be specified in the coinbase transaction" << endl;
        return false;
    }
    //l'indice dell'output di provenienza per l'input è l'indice del blocco nella blockchain
    if (transaction.txIns[0].txOutIndex != blockIndex) {
        cout << "the txIn signature in coinbase tx must be the block height" << endl;
        return false;
    }
    //la transazione i coinbase ha un solo output
    if (transaction.txOuts.size() != 1) {
        cout << "invalid number of txOuts in coinbase transaction" << endl;
        return false;
    }
    //l'output deve essere pari al coinbase amount
    if (transaction.txOuts[0].amount != COINBASE_AMOUNT) {
        cout << "invalid coinbase amount in coinbase transaction" << endl;
        return false;
    }
    return true;
};

//validazione di tutte le transazioni del blocco
bool validateBlockTransactions(vector<Transaction> aTransactions, vector<UnspentTxOut> aUnspentTxOuts, int blockIndex){
    Transaction coinbaseTx = aTransactions[0];
    if (!validateCoinbaseTx(coinbaseTx, blockIndex)) {
        cout << "invalid coinbase transaction: " << coinbaseTx.toString();
        return false;
    }

    // check for duplicate txIns. Each txIn can be included only once
    vector<TxIn> txIns;
    vector<Transaction>::iterator it;
    for(it = aTransactions.begin(); it != aTransactions.end(); ++it){

      //concatenzazion più efficiente di un semplice insert iterato perchè si fa l'allocazione una volta sola
      txIns.reserve(txIns.size() + it->txIns.size());
      copy(it->txIns.begin(), it->txIns.end(), txIns.end());
    }

    //Non possono esserci input usati due volte
    if (hasDuplicates(txIns)) {
        return false;
    }

    // Validazione di tutte le transazioni esclusa quella coinbase
    vector<Transaction> normalTransactions = aTransactions;
    normalTransactions.erase (normalTransactions.begin());
    for(it = normalTransactions.begin(); it != normalTransactions.end(); ++it){
      if(!validateTransaction(*it, aUnspentTxOuts)){
        return false;
      }
    }
    return true;
};

//Cerca un UnspentTxOut dati i suoi campi
UnspentTxOut findUnspentTxOut(string outId, int index, vector<UnspentTxOut> aUnspentTxOuts){
  UnspentTxOut ret = UnspentTxOut("", -1, "", 0);
  vector<UnspentTxOut>::iterator it;
  for(it = aUnspentTxOuts.begin(); it != aUnspentTxOuts.end(); ++it){
    if(it->txOutId == outId && it->txOutIndex == index){
      ret = *it;
    }
  }
    return ret;
};

//genera coinbase transaction per il nuovo blocco
Transaction getCoinbaseTransaction(string address,int blockIndex){
    Transaction t;
    TxIn txIn;
    txIn.signature = "";
    txIn.txOutId = "";
    txIn.txOutIndex = blockIndex;

    t.txIns = {txIn};
    t.txOuts = {TxOut(address, COINBASE_AMOUNT)};
    t.id = getTransactionId(t);
    return t;
};

//Applica la firma digitale ad un input della transazione, che viene preventivamente validato
string signTxIn(Transaction transaction, int txInIndex, string privateKey, vector<UnspentTxOut> aUnspentTxOuts){
    TxIn txIn = transaction.txIns[txInIndex];
    string dataToSign = transaction.id;
    UnspentTxOut referencedUnspentTxOut = findUnspentTxOut(txIn.txOutId, txIn.txOutIndex, aUnspentTxOuts);
    if (referencedUnspentTxOut.txOutIndex == -1) {
        cout << "could not find referenced txOut" << endl;
        return ""; //NOTE: segnalare un errore al chiamante
    }
    string referencedAddress = referencedUnspentTxOut.address;

    if (getPublicKey(privateKey) != referencedAddress) {
        cout << "trying to sign an input with private key that does not match the address that is referenced in txIn";
        return ""; //NOTE: segnalare un errore al chiamante
    }
    string signature = "ciao";
    //TODO: produrre validSignature, trovare libreria adatta
    /*
    key = ec.keyFromPrivate(privateKey, 'hex');
    string signature= toHexString(key.sign(dataToSign).toDER());

    */
    return signature;
};

//Aggiornamento della lista di output non spesi dopo un nuovo blocco di transazioni
vector<UnspentTxOut> updateUnspentTxOuts(vector<Transaction> aTransactions, vector<UnspentTxOut> aUnspentTxOuts){
    vector<UnspentTxOut> newUnspentTxOuts;
    vector<UnspentTxOut> consumedTxOuts;
    vector<Transaction>::iterator it;
    int index;


    for(it = aTransactions.begin(); it != aTransactions.end(); ++it){
      vector<TxOut>::iterator it2;
      index = 0;
      //per ognuno degli output creo un nuovo unspentTxOut da aggiungere alla lista aUnspentTxOuts
      for(it2 = it->txOuts.begin(); it2 != it->txOuts.end(); ++it2){
        newUnspentTxOuts.push_back(UnspentTxOut(it->id, index, it2->address, it2->amount));
        index++;
      }

      //Colleziono tutti gli UnspentTxOut che vengon spesi in questa transazione
      vector<TxIn>::iterator it3;
      for(it3 = it->txIns.begin(); it3 != it->txIns.end(); ++it3){
        consumedTxOuts.push_back(UnspentTxOut(it3->txOutId, it3->txOutIndex, " ", 0));
      }
    }

    //rimuovo da aUnspentTxOuts gli output spesi da questa transazione
    vector<UnspentTxOut>::iterator it4;
    for (it4 = aUnspentTxOuts.begin(); it4 != aUnspentTxOuts.end(); ) {
      if (isPresentUnspentTxOut(*it4, consumedTxOuts)) {
        it4 = aUnspentTxOuts.erase(it4);
      } else {
        ++it;
      }
    }

    //Aggiungo gli output non spesi creati dalla nuova transazione alla lista aUnspentTxOuts
    //uso un modo di concatenzazion più efficiente di un semplice insert iterato perchè si fa l'allocazione una volta sola
    aUnspentTxOuts.reserve(aUnspentTxOuts.size() + newUnspentTxOuts.size());
    copy(newUnspentTxOuts.begin(), newUnspentTxOuts.end(), aUnspentTxOuts.end());

    return aUnspentTxOuts;
};

//validazione del blocco di transazioni e aggiornamento della lista di output non spesi
vector <UnspentTxOut> processTransactions(vector<Transaction> aTransactions, vector<UnspentTxOut> aUnspentTxOuts, int blockIndex){

    if (!validateBlockTransactions(aTransactions, aUnspentTxOuts, blockIndex)) {
        cout << "invalid block transactions";
        return updateUnspentTxOuts({}, aUnspentTxOuts);; //NOTE: potremmo ritornare qualcosa per sollevare un errore o controllare esternamente che aUnspentTxOuts sia effettivamente cambiato (se no la transazione va abortita)
    }
    return updateUnspentTxOuts({aTransactions}, aUnspentTxOuts);
};

#endif
