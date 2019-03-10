#include "Transactions.hpp"

using namespace std;

Transaction::Transaction(){}
Transaction::Transaction(string id, vector<TransIn> transIns, vector<TransOut> transOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  this->id = id;
  this->transIns.swap(transIns);
  this->transOuts.swap(transOuts);
}

string Transaction::toString(){
  string ret = "{\"id\": \"" + this->id + "\",\"transIns\": [";
  vector<TransIn>::iterator it;
  for(it = transIns.begin(); it != transIns.end(); ++it){
    if(it != transIns.begin()){
      ret = ret + ", ";
    }
    ret = ret + it->toString();
  }

  ret = ret + "]";
  if(transOuts.size()>0){
    ret = ret + ",\"transOuts\": [";
  }
  vector<TransOut>::iterator it2;
  for(it2 = transOuts.begin(); it2 != transOuts.end(); ++it2){
    if(it2 != transOuts.begin()){
      ret = ret + ", ";
    }
    ret = ret + it2->toString();
  }
  if(transOuts.size()>0){
    ret = ret + "]";
  }
  ret = ret + "}";
  return  ret;
}

/*Verifica se l'UnspentTransOut passato come primo parametro è presente nell'array passato come secondo*/
bool isPresentUnspentTransOut(UnspentTransOut find, vector<UnspentTransOut> TransOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<UnspentTransOut>::iterator it;
  for(it = TransOuts.begin(); it != TransOuts.end(); ++it){
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

  string transInContent = "";
  string transOutContent = "";
  vector<TransIn>::iterator it;
  for(it = transaction.transIns.begin(); it != transaction.transIns.end(); ++it){
    transInContent = transInContent + it->transOutId + to_string(it->transOutIndex);
  }
  vector<TransOut>::iterator it2;
  for(it2 = transaction.transOuts.begin(); it2 != transaction.transOuts.end(); ++it2){
    transOutContent = transOutContent + it2->address + to_string(it2->amount);
  }
  return picosha2::hash256_hex_string(transInContent + transOutContent);
}

/*Controlla se il vettore di input contiene dei duplicati*/
bool hasDuplicates(vector<TransIn> transIns){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  /*Creiamo una mappa per salvare il numero di ricorrenze di ogni valore
  all'interno del vettore di transIn*/
  map<string, int> countMap;

  for (auto & transIn : transIns){
  	auto result = countMap.insert(pair<string, int>(transIn.transOutId + to_string(transIn.transOutIndex), 1));
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
bool isTransInWellFormed(TransIn transIn){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if (typeid(transIn.signature) != typeid(string)) {
    cout << "Errore (isTransInWellFormed): tipo della signature non valido: " << transIn.signature << endl;
    return false;
  }else if (typeid(transIn.transOutId) != typeid(string)) {
    cout << "Errore (isTransInWellFormed): tipo dell'id dell'output di transazione non valido: " << transIn.transOutId << endl;
    return false;
  }else if (typeid(transIn.transOutIndex) != typeid(int)) {
    cout << "Errore (isTransInWellFormed): tipo dell'indice dell'output di transazione non valido: " << transIn.transOutIndex << endl;
    return false;
  }else {
    return true;
  }
}

/*Validazione struttura (type checking) dell'output di transazione*/
bool isTransOutWellFormed(TransOut transOut){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if (typeid(transOut.address) != typeid(string)) {
    cout << "ERRORE (isTransOutWellFormed): tipo dell'indirizzo non valido nell'output di transazione: " << transOut.address << endl;
    return false;
  }else if (typeid(transOut.amount) != typeid(float)) {
    cout << "ERRORE (isTransOutWellFormed): tipo dell'importo non valido nell'output di transazione: " << transOut.amount << endl;
    return false;
  }else {
    return true;
  }
}

/*Validazione della struttura (type checking) della transazione e di tutti i suoi input e output*/
bool isTransactionWellFormed(Transaction transaction){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if (typeid(transaction.id) != typeid(string)) {
    cout << "ERRORE: tipo dell'id della transazione non vaido: " << transaction.id << endl;
    return false;
  }
  if (!(typeid(transaction.transIns) ==  (typeid(vector<TransIn>)))) {
    cout << "ERRORE: tipo della lista di input della transazione non valido: " << transaction.toString() << endl;
    return false;
  }
  vector<TransIn>::iterator it;
  for(it = transaction.transIns.begin(); it != transaction.transIns.end(); ++it){
    if(!isTransInWellFormed(*it)){
      return false;
    }
  }
  if (!(typeid(transaction.transOuts) ==  (typeid(vector<TransOut>)))) {
    cout << "ERRORE (isTransactionWellFormed): tipo della lista di output della transazione non valido: " << transaction.toString() << endl;
    return false;
  }
  vector<TransOut>::iterator it2;
  for(it2 = transaction.transOuts.begin(); it2 != transaction.transOuts.end(); ++it2){
    if(!isTransOutWellFormed(*it2)){
      return false;
    }
  }
  return true;
}

/*Validazione logica dell'input di transazione:
Controlla se l'input fa riferimento ad un output non speso,
verifica la firma applicata all'input di transazione, essa deve corrispondere alla
chiave pubblica che è la destinazione dell'output non speso a cui questo fa riferimento*/
bool isValidTransIn(TransIn transIn, Transaction transaction, vector<UnspentTransOut> unspentTransOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  UnspentTransOut referencedUTransOut;
  bool found = false;
  vector<UnspentTransOut>::iterator it;
  //Ricerca dell'output di transazione non speso referenziato nell'input
  for(it = unspentTransOuts.begin(); it != unspentTransOuts.end(); ++it){
    if(it->transOutId == transIn.transOutId && it->transOutIndex == transIn.transOutIndex){
      referencedUTransOut = *it;
      found = true;
    }
  }
  if (!found) {
      cout << "ERRORE (isValidTransIn): output di transazione referenziato nell'input non trovato: " << transIn.toString();
      return false;
  }

  int signOk = signVerify(referencedUTransOut.address, transaction.id, transIn.signature);
  if(signOk != 1){
    return false;
  }

  return true;
}

/*Dato l'input controlla il valore dell'output non speso a cui questo fa
riferimento (la sua esistenza è gia stata verificata in isValidTransIn)*/
float getAmountFromInput(TransIn transIn, vector<UnspentTransOut> unspentTransOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<UnspentTransOut>::iterator it;
  for(it = unspentTransOuts.begin(); it != unspentTransOuts.end(); ++it){
    if(it->transOutId == transIn.transOutId && it->transOutIndex == transIn.transOutIndex){
      return it->amount;
    }
  }
  cout << endl;
  throw "EXCEPTION (getAmountFromInput): Output di transazione referenziato non trovato: " + transIn.toString();
}

/*Validazione della transazione (tipi di dati e procedure di sicurezza)*/
bool isValidTransaction(Transaction transaction, vector<UnspentTransOut> unspentTransOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  //validazione struttura
  if (!isTransactionWellFormed(transaction)) {
      return false;
  }
  //Verifica l'hash per rilevare eventuali modifiche non permesse
  if (getTransactionId(transaction).compare(transaction.id) != 0) {
      cout << "ERRORE (isValidTransaction): id della transazione non valido: " << transaction.toString() << endl;
      return false;
  }
  //Validazione inputs (verifica firma e output non spesi di provenienza)
  vector<TransIn>::iterator it;
  for(it = transaction.transIns.begin(); it != transaction.transIns.end(); ++it){
    if(!isValidTransIn(*it, transaction, unspentTransOuts)){
      cout << "ERRORE (isValidTransaction): la transazione contiene input non validi: " << it->toString() << endl;
      return false;
    }
  }
  //Calcolo del totale degli inputs
  float totalTransInValues = 0;
  for(it = transaction.transIns.begin(); it != transaction.transIns.end(); ++it){
    try{
      float singleAmount = getAmountFromInput(*it, unspentTransOuts);
      totalTransInValues = totalTransInValues + singleAmount;
    }catch(const char* msg){
      cout << msg << endl;
      return false;
    }
  }
  //Calcolo del totale degli outputs
  vector<TransOut>::iterator it2;
  float totalTransOutValues = 0;
  for(it2 = transaction.transOuts.begin(); it2 != transaction.transOuts.end(); ++it2){
    totalTransOutValues = totalTransOutValues + it2->amount;
  }
  //Le somme di outputs e inputs devono essere uguali
  if (totalTransOutValues != totalTransInValues) {
      cout << "ERRORE (isValidTransaction): la somma degli inputs non corrisponde a quella degli outputs: " + transaction.toString() << endl;
      return false;
  }
  return true;
}

/*Validazione della transazione di Coinbase*/
bool isValidCoinbaseTransaction(Transaction transaction, int blockIndex){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  //Controllo che non ci siano modifiche controllando la validità dell'id (hash)
  if (getTransactionId(transaction) != transaction.id) {
      cout << "ERRORE (isValidCoinbaseTransaction): id della transazione coinbase non valido: " << transaction.id << endl;
      return false;
  }
  //La coinbase transaction deve avere un solo input
  if (transaction.transIns.size() != 1) {
      cout << "ERRORE (isValidCoinbaseTransaction): nessun input di transazione indicato nella transazione di coinbase" << endl;
      return false;
  }
  //l'indice dell'output di provenienza per l'input è l'indice del blocco nella blockchain
  if (transaction.transIns[0].transOutIndex != blockIndex) {
      cout << "ERRORE (isValidCoinbaseTransaction): la signature nella transazione coinbase deve essere l'indice del blocco" << endl;
      return false;
  }
  //la transazione i coinbase ha un solo output
  if (transaction.transOuts.size() != 1) {
      cout << "ERRORE (isValidCoinbaseTransaction): numero di output non valido nella transazione coinbase" << endl;
      return false;
  }
  //l'output deve essere pari al coinbase amount
  if (transaction.transOuts[0].amount != COINBASE_AMOUNT) {
      cout << "ERRORE (isValidCoinbaseTransaction): importo non valido nella transazione coinbase" << endl;
      return false;
  }
  return true;
}

/*Validazione di tutte le transazioni del blocco*/
bool areValidBlockTransactions(vector<Transaction> transactions, vector<UnspentTransOut> unspentTransOuts, int blockIndex){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  Transaction coinbaseTrans = transactions[0];
  if (!isValidCoinbaseTransaction(coinbaseTrans, blockIndex)) {
      cout << "ERRORE (areValidBlockTransactions): transazione coinbase non valida: " << coinbaseTrans.toString() << endl;
      return false;
  }

  //Controllo che non ci siano input duplicati
  vector<TransIn> transIns = {};
  vector<Transaction>::iterator it;
  for(it = transactions.begin(); it != transactions.end(); ++it){
    //Concatenzazione più efficiente di un semplice insert iterato perchè si fa l'allocazione una volta sola
    transIns.reserve(transIns.size() + it->transIns.size());
    transIns.insert(transIns.end(), it->transIns.begin(), it->transIns.end());
  }
  if (hasDuplicates(transIns)) {
      return false;
  }

  // Validazione di tutte le transazioni esclusa quella coinbase
  vector<Transaction> normalTransactions = transactions;
  normalTransactions.erase (normalTransactions.begin());
  for(it = normalTransactions.begin(); it != normalTransactions.end(); ++it){
    if(!isValidTransaction(*it, unspentTransOuts)){
      return false;
    }
  }
  return true;
}

/*Cerca un UnspentTransOut dati i suoi campi*/
UnspentTransOut getUnspentTransOut(string outId, int index, vector<UnspentTransOut> unspentTransOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  UnspentTransOut ret = UnspentTransOut("", -1, "", 0);
  bool found = false;
  vector<UnspentTransOut>::iterator it;
  for(it = unspentTransOuts.begin(); it != unspentTransOuts.end(); ++it){
    if(it->transOutId == outId && it->transOutIndex == index){
      ret = *it;
      found = true;
    }
  }
  if(!found){
    cout << endl;
    throw "EXCEPTION (getUnspentTransOut): Output non speso non trovato!";
  }
    return ret;
}

/*Genera coinbase transaction per il nuovo blocco*/
Transaction getCoinbaseTransaction(string address,int blockIndex){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  Transaction t;
  TransIn transIn;
  transIn.signature = "";
  transIn.transOutId = "";
  transIn.transOutIndex = blockIndex;

  t.transIns = {transIn};
  t.transOuts = {TransOut(address, COINBASE_AMOUNT)};
  t.id = getTransactionId(t);
  return t;
}

/* Ritorna la firma digitale per un input della transazione,
esso preventivamente viene controllato in termini di validità strutturale e logica*/
string getTransInSignature(Transaction transaction, int transInIndex, string privateKey, vector<UnspentTransOut> unspentTransOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  TransIn transIn = transaction.transIns[transInIndex];
  string dataToSign = transaction.id;
  string referencedAddress;
  try{
    UnspentTransOut referencedUnspentTransOut = getUnspentTransOut(transIn.transOutId, transIn.transOutIndex, unspentTransOuts);
    referencedAddress = referencedUnspentTransOut.address;
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (getTransInSignature): Firma dell'input di transazione abortita, output non speso referenziato non trovato!";
  }

  if (getWalletPublicKey().compare(referencedAddress) != 0) {
      throw "EXCEPTION (getTransInSignature): Firma dell'input di transazione abortita, tentativo di firmare con una chiave privata che non corrisponde all'indirizzo referenziato";
  }

  string* signature = new string;
  int signCreated = createSignature(privateKey, dataToSign, *signature);
  if(signCreated == 0){
    throw "EXCEPTION (getTransInSignature): operazione di firma dell'input di transazione fallita!";
  }

  //Ritorno della signature prodotta, nel formato stringa in notazione puntata
  return *signature;
}

/*Aggiornamento della lista di output non spesi dopo un nuovo blocco di transazioni*/
vector<UnspentTransOut> updateUnspentTransOuts(vector<Transaction> transactions, vector<UnspentTransOut> unspentTransOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<UnspentTransOut> newUnspentTransOuts;
  vector<UnspentTransOut> consumedTransOuts;
  vector<Transaction>::iterator it;
  int index;

  for(it = transactions.begin(); it != transactions.end(); ++it){
    vector<TransOut>::iterator it2;
    index = 0;
    //Per ognuno degli output creo un nuovo unspentTransOut da aggiungere alla lista unspentTransOuts
    for(it2 = it->transOuts.begin(); it2 != it->transOuts.end(); ++it2){
      newUnspentTransOuts.push_back(UnspentTransOut(it->id, index, it2->address, it2->amount));
      index++;
    }

    //Colleziono tutti gli UnspentTransOut che vengon spesi in questa transazione
    vector<TransIn>::iterator it3;
    for(it3 = it->transIns.begin(); it3 != it->transIns.end(); ++it3){
      consumedTransOuts.push_back(UnspentTransOut(it3->transOutId, it3->transOutIndex, " ", 0));
    }
  }

  //Rimuovo da unspentTransOuts gli output spesi da questa transazione
  vector<UnspentTransOut>::iterator it4;
  for (it4 = unspentTransOuts.begin(); it4 != unspentTransOuts.end(); ) {
    if (isPresentUnspentTransOut(*it4, consumedTransOuts)) {
      it4 = unspentTransOuts.erase(it4);
    } else {
      ++it4;
    }
  }
  /*Aggiungo gli output non spesi creati dalla nuova transazione alla
  lista unspentTransOuts, uso un modo di concatenzazion più efficiente di un
  semplice insert iterato perchè si fa l'allocazione una volta sola*/
  unspentTransOuts.reserve(unspentTransOuts.size() + newUnspentTransOuts.size());
  unspentTransOuts.insert(unspentTransOuts.end(), newUnspentTransOuts.begin(), newUnspentTransOuts.end());
  return unspentTransOuts;
}

/*Validazione del blocco di transazioni e aggiornamento della lista di output non spesi*/
vector <UnspentTransOut> processTransactions(vector<Transaction> transactions, vector<UnspentTransOut> unspentTransOuts, int blockIndex){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if (!areValidBlockTransactions(transactions, unspentTransOuts, blockIndex)) {
      cout << endl;
      throw "EXCEPTION (processTransactions): il blocco contiene transazioni non valide";
  }
  return updateUnspentTransOuts({transactions}, unspentTransOuts);
}

/*Generazione di una nuova transazione*/
Transaction createTransaction(string receiverAddress, float amount, string privateKey, vector<UnspentTransOut> unspentTransOuts, vector<Transaction> pool){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  //Ottengo la lista degli output non spesi per il mio wallet
  string myAddress = getWalletPublicKey();
  vector<UnspentTransOut> myUnspentTransOutsA = getUnspentTransOutsOfAddress(myAddress, unspentTransOuts);

  /*Ottengo la lista dei miei output non spesi a cui sono stati togli gli output
   che vengono usati come input in una delle transazioni nel pool*/
  vector<UnspentTransOut> myUnspentTransOuts = filterUnspentTransOuts(myUnspentTransOutsA, pool);
  float leftOverAmount;

  /*Cerco tra gli output non spesi quelli necessari per il nuovo input
  (devo "raccogliere" un ammontare pari ad amount),
  gli output che sto usando saranno nel nuovo vettore includedUnspentTransOuts*/
  vector<UnspentTransOut> includedUnspentTransOuts;
  try{
    includedUnspentTransOuts= getTransOutsForAmount(amount, myUnspentTransOuts, &leftOverAmount);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (createTransaction): Impossibile creare la transazione, il mittente non possiede abbastanza skanCoin!";
  }
  /*Raccolti gli output non spesi da usare di creano i rispettivi input per la nuova transazione
   (che faranno riferimento ad essi), questi devono ancora essere firmati!*/
  vector<TransIn> unsignedTransIns;
  vector<UnspentTransOut>::iterator it2;
  for(it2 = includedUnspentTransOuts.begin(); it2 != includedUnspentTransOuts.end(); ++it2){
    unsignedTransIns.push_back(getInputFromUnspentTransOut(*it2));
  }
  //Creo la transazione, gli input non sono ancora firmati
  Transaction transaction = Transaction();
  transaction.transIns = unsignedTransIns;
  transaction.transOuts = createTransOuts(receiverAddress, myAddress, amount, leftOverAmount);
  transaction.id = getTransactionId(transaction);
  int index = 0;
  try{
    //firma di tutti gli input della nuova transazione
    vector<TransIn>::iterator it3;
    for(it3 = transaction.transIns.begin(); it3 != transaction.transIns.end(); ++it3){
      it3->signature = getTransInSignature(transaction, index, privateKey, unspentTransOuts);
      index++;
    }
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (createTransaction): Creazione della transazione fallita, alcuni input di transazione non sono stati firmati correttamente";
  }
  cout << "Nuova transazione creata!" << endl;
  return transaction;
}

/*Generazione di una nuova transazione, a partire da un array di output, per
permettere di esporre la rest per effettuare più movimenti dallo stesso wallet
in un solo blocco senza passare dal transaction pool*/
Transaction createTransactionWithMultipleOutputs (std::vector<TransOut> transOuts, std::string privateKey, std::vector<UnspentTransOut> unspentTransOuts, std::vector<Transaction> pool){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  //Ottengo la lista degli output non spesi per il mio wallet
  string myAddress = getWalletPublicKey();
  vector<UnspentTransOut> myUnspentTransOutsA = getUnspentTransOutsOfAddress(myAddress, unspentTransOuts);

  /*Ottengo la lista dei miei output non spesi a cui sono stati togli gli output
   che vengono usati come input in una delle transazioni nel pool*/
  vector<UnspentTransOut> myUnspentTransOuts = filterUnspentTransOuts(myUnspentTransOutsA, pool);
  float leftOverAmount;

  /*Cerco tra gli output non spesi quelli necessari per il nuovo input
  (devo "raccogliere" un ammontare pari ad amount),
  gli output che sto usando saranno nel nuovo vettore includedUnspentTransOuts*/
  vector<UnspentTransOut> includedUnspentTransOuts;

  //Calcolo il totale degli output
  float amount = 0;
  vector<TransOut>::iterator it;
  for(it = transOuts.begin(); it != transOuts.end(); ++it){
    amount += it->amount;
  }
  try{
    includedUnspentTransOuts= getTransOutsForAmount(amount, myUnspentTransOuts, &leftOverAmount);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (createTransaction): Impossibile creare la transazione, il mittente non possiede abbastanza skanCoin!";
  }
  /*Raccolti gli output non spesi da usare di creano i rispettivi input per la nuova transazione
   (che faranno riferimento ad essi), questi devono ancora essere firmati!*/
  vector<TransIn> unsignedTransIns;
  vector<UnspentTransOut>::iterator it2;
  for(it2 = includedUnspentTransOuts.begin(); it2 != includedUnspentTransOuts.end(); ++it2){
    unsignedTransIns.push_back(getInputFromUnspentTransOut(*it2));
  }
  //Creo la transazione, gli input non sono ancora firmati
  Transaction transaction = Transaction();
  transaction.transIns = unsignedTransIns;
  transaction.transOuts = transOuts;
  if (leftOverAmount != 0) {
      TransOut leftOverTransOut = TransOut(myAddress, leftOverAmount);
      transaction.transOuts.push_back(leftOverTransOut);
  }
  transaction.id = getTransactionId(transaction);
  int index = 0;
  try{
    //firma di tutti gli input della nuova transazione
    vector<TransIn>::iterator it3;
    for(it3 = transaction.transIns.begin(); it3 != transaction.transIns.end(); ++it3){
      it3->signature = getTransInSignature(transaction, index, privateKey, unspentTransOuts);
      index++;
    }
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (createTransaction): Creazione della transazione fallita, alcuni input di transazione non sono stati firmati correttamente";
  }
  cout << "Nuova transazione creata!" << endl;
  return transaction;
}

/*Ritorna la lista di tutti gli output non spesi, senza quelli che sono
referenziati da un qualche input in una delle transazioni presenti nella transaction pool*/
vector<UnspentTransOut> filterUnspentTransOuts(vector<UnspentTransOut> unspentTransOuts, vector<Transaction> transactionPool){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<TransIn> transIns;

  vector<Transaction>::iterator it;
  vector<TransIn>::iterator it2;
  for(it = transactionPool.begin(); it != transactionPool.end(); ++it){
    for(it2 = it->transIns.begin(); it2 != it->transIns.end(); ++it2){
      transIns.push_back(*it2);
    }
  }
  vector<UnspentTransOut> res;
  //copia del vettore per evitare side effects, faccio solo una allocazione per avere migliori prestazioni
  res.reserve(res.size() + unspentTransOuts.size());
  res.insert(res.end(), unspentTransOuts.begin(), unspentTransOuts.end());

  /*Per ogni output non speso cerco nel vettore di transIn quella che fa riferimento
  //ad esso, se la trovo rimuovo dal vettore l'output non speso*/
  vector<UnspentTransOut>::iterator it3;
  for(it3 = res.begin(); it3 != res.end();){
    if (isReferencedUnspentTransOut(*it3, transIns)) {
      it3 = res.erase(it3);
    } else {
      ++it3;
    }
  }
    return res;
}

/*Creazione di un input di transazione a partire da un output non speso
a cui questo farà riferimento*/
TransIn getInputFromUnspentTransOut(UnspentTransOut unspentTransOut){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  TransIn transIn = TransIn();
  transIn.transOutId = unspentTransOut.transOutId;
  transIn.transOutIndex = unspentTransOut.transOutIndex;
  return transIn;
}
