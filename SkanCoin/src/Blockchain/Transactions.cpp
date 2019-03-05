#include "Transactions.hpp"

using namespace std;

const int COINBASE_AMOUNT = 10;

UnspentTxOut::UnspentTxOut() {}

UnspentTxOut::UnspentTxOut(string txOutId, int txOutIndex, string address, float amount) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  this->txOutId = txOutId;
  this->txOutIndex = txOutIndex;
  this->address = address;
  this->amount = amount;
}

string UnspentTxOut::toString(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return "{\"txOutId\": \"" + this->txOutId + "\", \"address\": \"" + this->address + "\", \"txOutIndex\": " + to_string(this->txOutIndex) + ", \"amount\": " + to_string(this->amount) + "}";
}

bool UnspentTxOut::isEqual(UnspentTxOut other){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return (this->txOutId == other.txOutId && this->txOutIndex == other.txOutIndex);
}

TxIn::TxIn() {}

TxIn::TxIn(string txOutId, string signature, int txOutIndex) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  this->txOutId = txOutId;
  this->signature = signature;
  this->txOutIndex = txOutIndex;
}

bool TxIn::isEqual(TxIn other){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return (this->txOutId == other.txOutId && this->txOutIndex == other.txOutIndex);
}

string TxIn::toString(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return "{\"txOutId\": \"" + txOutId + "\", \"signature\": \"" + signature + "\", \"txOutIndex\": " + to_string(txOutIndex) + "}";
}

TxOut::TxOut(){}

TxOut::TxOut(string address, float amount) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  this->address = address;
  this->amount = amount;
}

string TxOut::toString(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return "{\"address\": \"" + address + "\", \"amount\": " + to_string(amount) + "}";
}

Transaction::Transaction(){}
Transaction::Transaction(string id, vector<TxIn> txIns, vector<TxOut> txOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  this->id = id;
  this->txIns.swap(txIns);
  this->txOuts.swap(txOuts);
}

string Transaction::toString(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  string ret = "{\"id\": \"" + this->id + "\",\"txIns\": [";
  vector<TxIn>::iterator it;
  for(it = txIns.begin(); it != txIns.end(); ++it){
    if(it != txIns.begin()){
      ret = ret + ", ";
    }
    ret = ret + it->toString();
  }

  ret = ret + "],\"txOuts\": [";
  vector<TxOut>::iterator it2;
  for(it2 = txOuts.begin(); it2 != txOuts.end(); ++it2){
    if(it2 != txOuts.begin()){
      ret = ret + ", ";
    }
    ret = ret + it2->toString();
  }

  ret = ret + "]}";
  return  ret;
}

/*Verifica se l'UnspentTxOut passato come primo parametro è presente nell'array passato come secondo*/
bool isPresentUnspentTxOut(UnspentTxOut find, vector<UnspentTxOut> TxOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<UnspentTxOut>::iterator it;
  for(it = TxOuts.begin(); it != TxOuts.end(); ++it){
    if(it->isEqual(find)){
      return true;
    }
  }
  return false;
}

/*Ritorna l'id (hash) della transazione passata come parametro,
esso verrà firmato da chi invia i coin*/
string getTransactionId (Transaction transaction){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

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
  return picosha2::hash256_hex_string(txInContent + txOutContent);
}

/*Controlla se il vettore di input contiene dei duplicati*/
bool hasDuplicates(vector<TxIn> txIns){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  /*Creiamo una mappa per salvare il numero di ricorrenze di ogni valore
  all'interno del vettore di txIn*/
  map<string, int> countMap;

  for (auto & txIn : txIns){
  	auto result = countMap.insert(pair<string, int>(txIn.txOutId + to_string(txIn.txOutIndex), 1));
  	if (result.second == false) //l'elemento esiste già
  		result.first->second++;
  }

  for (auto & item : countMap){
  	//esistono elementi duplicati
  	if (item.second > 1){
  		cout <<"ERRORE (hasDuplicates): Input di transazione duplicato: " << item.first;
      return true;
  	}
  }
  return false;
}

/*Validazione della struttura (type checking) dell'input di transazione*/
bool isValidTxInStructure(TxIn txIn){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if (typeid(txIn.signature) != typeid(string)) {
    cout << "Errore (isValidTxInStructure): tipo della signature non valido: " << txIn.signature << endl;
    return false;
  }else if (typeid(txIn.txOutId) != typeid(string)) {
    cout << "Errore (isValidTxInStructure): tipo dell'id dell'output di transazione non valido: " << txIn.txOutId << endl;
    return false;
  }else if (typeid(txIn.txOutIndex) != typeid(int)) {
    cout << "Errore (isValidTxInStructure): tipo dell'indice dell'output di transazione non valido: " << txIn.txOutIndex << endl;
    return false;
  }else {
    return true;
  }
}

/*Validazione struttura (type checking) dell'output di transazione*/
bool isValidTxOutStructure(TxOut txOut){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if (typeid(txOut.address) != typeid(string)) {
    cout << "ERRORE (isValidTxOutStructure): tipo dell'indirizzo non valido nell'output di transazione: " << txOut.address << endl;
    return false;
  }else if (typeid(txOut.amount) != typeid(float)) {
    cout << "ERRORE (isValidTxOutStructure): tipo dell'importo non valido nell'output di transazione: " << txOut.amount << endl;
    return false;
  }else {
    return true;
  }
}

/*Validazione della struttura (type checking) della transazione e di tutti i suoi input e output*/
bool isValidTransactionStructure(Transaction transaction){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if (typeid(transaction.id) != typeid(string)) {
    cout << "ERRORE: tipo dell'id della transazione non vaido: " << transaction.id << endl;
    return false;
  }
  if (!(typeid(transaction.txIns) ==  (typeid(vector<TxIn>)))) {
    cout << "ERRORE: tipo della lista di input della transazione non valido: " << transaction.toString() << endl;
    return false;
  }
  vector<TxIn>::iterator it;
  for(it = transaction.txIns.begin(); it != transaction.txIns.end(); ++it){
    if(!isValidTxInStructure(*it)){
      return false;
    }
  }
  if (!(typeid(transaction.txOuts) ==  (typeid(vector<TxOut>)))) {
    cout << "ERRORE (isValidTransactionStructure): tipo della lista di output della transazione non valido: " << transaction.toString() << endl;
    return false;
  }
  vector<TxOut>::iterator it2;
  for(it2 = transaction.txOuts.begin(); it2 != transaction.txOuts.end(); ++it2){
    if(!isValidTxOutStructure(*it2)){
      return false;
    }
  }
  return true;
}

/*Validazione logica dell'input di transazione:
Controlla se l'input fa riferimento ad un output non speso,
verifica la firma applicata all'input di transazione, essa deve corrispondere alla
chiave pubblica che è la destinazione dell'output non speso a cui questo fa riferimento*/
bool validateTxIn(TxIn txIn, Transaction transaction, vector<UnspentTxOut> aUnspentTxOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  UnspentTxOut referencedUTxOut;
  bool found = false;
  vector<UnspentTxOut>::iterator it;
  //Ricerca dell'output di transazione non speso referenziato nell'input
  for(it = aUnspentTxOuts.begin(); it != aUnspentTxOuts.end(); ++it){
    if(it->txOutId == txIn.txOutId && it->txOutIndex == txIn.txOutIndex){
      referencedUTxOut = *it;
      found = true;
    }
  }
  if (!found) {
      cout << "ERRORE (validateTxIn): output di transazione referenziato nell'input non trovato: " << txIn.toString();
      return false;
  }

  /*Verifica della firma, effettuo le conversioni ad array di byte implementate
  in Wallet, per ottenere un formato compatibile con l'interfaccia della
  libreria per ECDSA*/

  //Chiave pubblica del proprietario dell'output non speso
  uint8_t p_public[ECC_BYTES+1];
  byteArrayFromString(referencedUTxOut.address, p_public);

  /*Id della transazione (hash su cui è stata fatta la firma). Su questo elemento
  non applichiamo la conversione implementata in wallet, poiche questo hash
  è una normale stringa che non rispetta la notazione puntata utilizzata nelle
  nostre conversioni, dunque si converte semplicemente ogni carattere in un byte*/
  uint8_t p_hash[ECC_BYTES];
  memcpy (p_hash, transaction.id.c_str(), ECC_BYTES);

  //Signature creata con la chiave privata del proprietario dell'output non speso
  uint8_t p_signature[ECC_BYTES*2];
  byteArrayFromString(txIn.signature, p_signature);

  int signOk = ecdsa_verify(p_public, p_hash , p_signature);
  cout << signOk << endl;
  if(signOk != 1){
    return false;
  }

  return true;
}

/*Dato l'input controlla il valore dell'output non speso a cui questo fa
riferimento (la sua esistenza è gia stata verificata in validateTxIn)*/
float getTxInAmount(TxIn txIn, vector<UnspentTxOut> aUnspentTxOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<UnspentTxOut>::iterator it;
  for(it = aUnspentTxOuts.begin(); it != aUnspentTxOuts.end(); ++it){
    if(it->txOutId == txIn.txOutId && it->txOutIndex == txIn.txOutIndex){
      return it->amount;
    }
  }
  cout << endl;
  throw "EXCEPTION (getTxInAmount): Output di transazione referenziato non trovato: " + txIn.toString();
}

/*Validazione della transazione (tipi di dati e procedure di sicurezza)*/
bool validateTransaction(Transaction transaction, vector<UnspentTxOut> aUnspentTxOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  //validazione struttura
  if (!isValidTransactionStructure(transaction)) {
      return false;
  }
  //Verifica l'hash per rilevare eventuali modifiche non permesse
  if (getTransactionId(transaction).compare(transaction.id) != 0) {
      cout << "ERRORE (validateTransaction): id della transazione non valido: " << transaction.toString() << endl;
      return false;
  }
  //Validazione inputs (verifica firma e output non spesi di provenienza)
  vector<TxIn>::iterator it;
  for(it = transaction.txIns.begin(); it != transaction.txIns.end(); ++it){
    if(!validateTxIn(*it, transaction, aUnspentTxOuts)){
      cout << "ERRORE (validateTransaction): la transazione contiene input non validi: " << it->toString() << endl;
      return false;
    }
  }
  //Calcolo del totale degli inputs
  float totalTxInValues = 0;
  for(it = transaction.txIns.begin(); it != transaction.txIns.end(); ++it){
    try{
      float singleAmount = getTxInAmount(*it, aUnspentTxOuts);
      totalTxInValues = totalTxInValues + singleAmount;
    }catch(const char* msg){
      cout << msg << endl;
      return false;
    }
  }
  //Calcolo del totale degli outputs
  vector<TxOut>::iterator it2;
  float totalTxOutValues = 0;
  for(it2 = transaction.txOuts.begin(); it2 != transaction.txOuts.end(); ++it2){
    totalTxOutValues = totalTxOutValues + it2->amount;
  }
  //Le somme di outputs e inputs devono essere uguali
  if (totalTxOutValues != totalTxInValues) {
      cout << "ERRORE (validateTransaction): la somma degli inputs non corrisponde a quella degli outputs: " + transaction.toString() << endl;
      return false;
  }
  return true;
}

/*Validazione della transazione di Coinbase*/
bool validateCoinbaseTx(Transaction transaction, int blockIndex){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  //Controllo che non ci siano modifiche controllando la validità dell'id (hash)
  if (getTransactionId(transaction) != transaction.id) {
      cout << "ERRORE (validateCoinbaseTx): id della transazione coinbase non valido: " << transaction.id << endl;
      return false;
  }
  //La coinbase transaction deve avere un solo input
  if (transaction.txIns.size() != 1) {
      cout << "ERRORE (validateCoinbaseTx): nessun input di transazione indicato nella transazione di coinbase" << endl;
      return false;
  }
  //l'indice dell'output di provenienza per l'input è l'indice del blocco nella blockchain
  if (transaction.txIns[0].txOutIndex != blockIndex) {
      cout << "ERRORE (validateCoinbaseTx): la signature nella transazione coinbase deve essere l'indice del blocco" << endl;
      return false;
  }
  //la transazione i coinbase ha un solo output
  if (transaction.txOuts.size() != 1) {
      cout << "ERRORE (validateCoinbaseTx): numero di output non valido nella transazione coinbase" << endl;
      return false;
  }
  //l'output deve essere pari al coinbase amount
  if (transaction.txOuts[0].amount != COINBASE_AMOUNT) {
      cout << "ERRORE (validateCoinbaseTx): importo non valido nella transazione coinbase" << endl;
      return false;
  }
  return true;
}

/*Validazione di tutte le transazioni del blocco*/
bool validateBlockTransactions(vector<Transaction> aTransactions, vector<UnspentTxOut> aUnspentTxOuts, int blockIndex){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  Transaction coinbaseTx = aTransactions[0];
  if (!validateCoinbaseTx(coinbaseTx, blockIndex)) {
      cout << "ERRORE (validateBlockTransactions): transazione coinbase non valida: " << coinbaseTx.toString() << endl;
      return false;
  }

  //Controllo che non ci siano input duplicati
  vector<TxIn> txIns = {};
  vector<Transaction>::iterator it;
  for(it = aTransactions.begin(); it != aTransactions.end(); ++it){
    //Concatenzazione più efficiente di un semplice insert iterato perchè si fa l'allocazione una volta sola
    txIns.reserve(txIns.size() + it->txIns.size());
    txIns.insert(txIns.end(), it->txIns.begin(), it->txIns.end());
  }
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
}

/*Cerca un UnspentTxOut dati i suoi campi*/
UnspentTxOut findUnspentTxOut(string outId, int index, vector<UnspentTxOut> aUnspentTxOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  UnspentTxOut ret = UnspentTxOut("", -1, "", 0);
  bool found = false;
  vector<UnspentTxOut>::iterator it;
  for(it = aUnspentTxOuts.begin(); it != aUnspentTxOuts.end(); ++it){
    if(it->txOutId == outId && it->txOutIndex == index){
      ret = *it;
      found = true;
    }
  }
  if(!found){
    cout << endl;
    throw "EXCEPTION (findUnspentTxOut): Output non speso non trovato!";
  }
    return ret;
}

/*Genera coinbase transaction per il nuovo blocco*/
Transaction getCoinbaseTransaction(string address,int blockIndex){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  Transaction t;
  TxIn txIn;
  txIn.signature = "";
  txIn.txOutId = "";
  txIn.txOutIndex = blockIndex;

  t.txIns = {txIn};
  t.txOuts = {TxOut(address, COINBASE_AMOUNT)};
  t.id = getTransactionId(t);
  return t;
}

/*Applica la firma digitale ad un input della transazione,
esso preventivamente controllato in termini di validità strutturale e logica*/
string signTxIn(Transaction transaction, int txInIndex, string privateKey, vector<UnspentTxOut> aUnspentTxOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  TxIn txIn = transaction.txIns[txInIndex];
  string dataToSign = transaction.id;
  string referencedAddress;
  try{
    UnspentTxOut referencedUnspentTxOut = findUnspentTxOut(txIn.txOutId, txIn.txOutIndex, aUnspentTxOuts);
    referencedAddress = referencedUnspentTxOut.address;
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (signTxIn): Firma dell'input di transazione abortita, output non speso referenziato non trovato!";
  }

  if (getPublicFromWallet().compare(referencedAddress) != 0) {
      throw "EXCEPTION (signTxIn): Firma dell'input di transazione abortita, tentativo di firmare con una chiave privata che non corrisponde all'indirizzo referenziato";
  }

  //Generazione della firma a partire dall'hash (id della transazione)
  uint8_t p_hash[ECC_BYTES];
  memcpy (p_hash, dataToSign.c_str(), ECC_BYTES);
  uint8_t p_private[ECC_BYTES];
  byteArrayFromString(privateKey, p_private);
  uint8_t p_signature[ECC_BYTES*2];
  int signCreated = ecdsa_sign(p_private, p_hash, p_signature);
  if(signCreated == false){
    throw "EXCEPTION (signTxIn): operazione di firma dell'input di transazione fallita!";
  }

  //Ritorno della signature prodotto, nel formato stringa in notazione puntata
  return stringFromByteArray(p_signature, ECC_BYTES*2);
}

/*Aggiornamento della lista di output non spesi dopo un nuovo blocco di transazioni*/
vector<UnspentTxOut> updateUnspentTxOuts(vector<Transaction> aTransactions, vector<UnspentTxOut> aUnspentTxOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<UnspentTxOut> newUnspentTxOuts;
  vector<UnspentTxOut> consumedTxOuts;
  vector<Transaction>::iterator it;
  int index;

  for(it = aTransactions.begin(); it != aTransactions.end(); ++it){
    vector<TxOut>::iterator it2;
    index = 0;
    //Per ognuno degli output creo un nuovo unspentTxOut da aggiungere alla lista aUnspentTxOuts
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

  //Rimuovo da aUnspentTxOuts gli output spesi da questa transazione
  vector<UnspentTxOut>::iterator it4;
  for (it4 = aUnspentTxOuts.begin(); it4 != aUnspentTxOuts.end(); ) {
    if (isPresentUnspentTxOut(*it4, consumedTxOuts)) {
      it4 = aUnspentTxOuts.erase(it4);
    } else {
      ++it4;
    }
  }
  /*Aggiungo gli output non spesi creati dalla nuova transazione alla
  lista aUnspentTxOuts, uso un modo di concatenzazion più efficiente di un
  semplice insert iterato perchè si fa l'allocazione una volta sola*/
  aUnspentTxOuts.reserve(aUnspentTxOuts.size() + newUnspentTxOuts.size());
  aUnspentTxOuts.insert(aUnspentTxOuts.end(), newUnspentTxOuts.begin(), newUnspentTxOuts.end());
  return aUnspentTxOuts;
}

/*Validazione del blocco di transazioni e aggiornamento della lista di output non spesi*/
vector <UnspentTxOut> processTransactions(vector<Transaction> aTransactions, vector<UnspentTxOut> aUnspentTxOuts, int blockIndex){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if (!validateBlockTransactions(aTransactions, aUnspentTxOuts, blockIndex)) {
      cout << endl;
      throw "EXCEPTION (processTransactions): il blocco contiene transazioni non valide";
  }
  return updateUnspentTxOuts({aTransactions}, aUnspentTxOuts);
}
