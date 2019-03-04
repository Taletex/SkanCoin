#include "Wallet.hpp"

using namespace std;

/*Carica la chiave (pubblica o privata) dall'apposito file*/
string loadKey(bool isPrivate){
  if(debug == 1){
    cout << endl << "Wallet - loadKey" << endl;
  }
  string location;
  string key;
  if(isPrivate == false){
    location = publicKeyLocation;
  }else{
    location = privateKeyLocation;
  }
  ifstream inFile;
  inFile.open(location);
  if(inFile.is_open()) {
    getline(inFile, key);
    inFile.close();
    return key;
  } else {
    throw "EXCEPTION (loadKey): non è stato possibile aprire il file per leggere la chiave!";
  }
}

/*Salvataggio della chiave (pubblica o privata) nell'apposito file*/
void saveKey(string key, bool isPrivate){
  if(debug == 1){
    cout << endl << "Wallet - saveKey" << endl;
  }
  string location;
  if(isPrivate == false){
    location = publicKeyLocation;
  }else{
    location = privateKeyLocation;
  }
  ofstream myfile;
  myfile.open (location, ios::out | ios::trunc);
  if(myfile.is_open()) {
    myfile << key;
  } else {
    throw "EXCEPTION (saveKey): non è stato possibile aprire il file per salvare la chiave!";
  }
}

/*La libreria utilizzata per la firma digitale basata su curve ellittiche utilizza
chiavi (e produce signature) in forma di array di byte (uint8_t), tuttavia
effettuiamo una conversione (reversibile) a stringa sulle chiavi e sulle signature
per semplificarne la gestione nelle classi esterne.
In particolare questo ci permette di avere un formato
stampabile e più facilmente interpretabile nelle interazioni tra i peer e
e nei payload delle richieste/risposte HTTP
Il formato usato per le stringhe prevede di stampare il valore numerico di ogni byteArrayFromString
dell'array, utilizzando il punto come separatore (es: 33.243.453.334. ....)*/

/*Conversione array di byte -> stringa*/
string stringFromByteArray(uint8_t *array, int len){
  if(debug == 1){
    cout << endl << "Wallet - stringFromByteArray" << endl;
  }
  string ret = "";
  for (int a = 0; a < len; a++) {
    if(a != 0){
      ret += ".";
    }
    ret +=  to_string((int)array[a]);
  }
  return ret;
}

/*Conversione stringa -> array di byte*/
void byteArrayFromString(string str, uint8_t *dest){
  if(debug == 1){
    cout << endl << "Wallet - byteArrayFromString" << endl;
  }
  stringstream tempstream(str);
  string segment;
  int x;
  int i = 0;
  while(std::getline(tempstream, segment, '.')){
    x = stoi(segment);
    dest[i] = x;
    i++;
  }
}

/*Legge la chiave privata del nodo dal file*/
string getPrivateFromWallet(){
  if(debug == 1){
    cout << endl << "Wallet - getPrivateFromWallet" << endl;
  }
  try{
    return loadKey(true);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (getPrivateFromWallet): Errore durante il caricamento della chiave privata!";
  }
}

/*Legge la chiave pubblica del nodo dal file*/
string getPublicFromWallet(){
  if(debug == 1){
    cout << endl << "Wallet - getPublicFromWallet" << endl;
  }
  try{
    return loadKey(false);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (getPublicFromWallet): Errore durante il caricamento della chiave pubblica!";
  }
}

/*Generazione di una nuova coppia di chiavi e salvataggio negi appositi file*/
void generateKeys(){
  if(debug == 1){
    cout << endl << "Wallet - generateKeys" << endl;
  }
  uint8_t p_publicKey[ECC_BYTES+1];
  uint8_t p_privateKey[ECC_BYTES];
  int count = 0;
  //Generazione nuova coppia di chiavi
  int createdKeys = ecc_make_key(p_publicKey, p_privateKey);

  //La generazione non è andata a buon fine
  while(createdKeys != 1){
    if(count > 5){
      throw "EXCEPTION (generateKeys): Errore durante la generazione delle chiavi, troppi tentativi falliti!";
    }
    cout << "ERRORE (generateKeys): Errore durante la generazione delle chiavi, nuovo tentativo in corso (" << count << ")..." << endl;
    createdKeys = ecc_make_key(p_publicKey, p_privateKey);
  }

  //Salvataggio delle chiavi negli appositi files
  try{
    saveKey(stringFromByteArray(p_publicKey,ECC_BYTES+1), false);
    saveKey(stringFromByteArray(p_privateKey,ECC_BYTES), true);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION: Errore durante il salvataggio delle chiavi";
    }
}

/*Inizializzazione del wallet (chiave privata), se il file non esiste si
genera una nuova chiave e si salva su file*/
void initWallet(){
  if(debug == 1){
    cout << endl << "Wallet - initWallet" << endl;
  }
  //controllo se esiste il file con la chiave privata
  if (FILE* file = fopen(privateKeyLocation.c_str(), "r")) {
    fclose(file);
    cout << "Trovato wallet esistente..." << endl;
    return;
  }

  //genero nuova coppia di chiavi ed effettuo il salvataggio su file
  try{
      generateKeys();
      cout << "Nuovo wallet creato!" << endl;
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (initWallet): Errore durante la generazione delle chiavi!";
  }
}

/*Eliminazione del wallet (file contenenti le chiavi)*/
void deleteWallet(){
  if(debug == 1){
    cout << endl << "Wallet - deleteWallet" << endl;
  }
  if (FILE* file = fopen(privateKeyLocation.c_str(), "r")) {
    fclose(file);

    if( remove( privateKeyLocation.c_str() ) != 0 || remove( publicKeyLocation.c_str() ) != 0){
      cout << "ERRORE (deleteWallet):errore durante l'eliminazione del file: " << privateKeyLocation << endl;
    }
    else{
      cout << "I file contenenti le chiavi sono stati eliminati con successo!" << endl;
    }
    return;
  }
}

/*Ritorna la lista degli output non spesi appartenenti ad un certo indirizzo (wallet)*/
vector<UnspentTxOut> findUnspentTxOutsOfAddress(string ownerAddress, vector<UnspentTxOut> unspentTxOuts){
  if(debug == 1){
    cout << endl << "Wallet - findUnspentTxOutsOfAddress" << endl;
  }
  vector<UnspentTxOut> res;
  //Per fare una copia del vettore faccio solo una allocazione per avere migliori prestazioni
  res.reserve(res.size() + unspentTxOuts.size());
  res.insert(res.end(), unspentTxOuts.begin(), unspentTxOuts.end());

  vector<UnspentTxOut>::iterator it;
  for (it = res.begin(); it != res.end(); ) {
    if (it->address != ownerAddress) {
      it = res.erase(it);
    } else {
      ++it;
    }
  }
  return res;
}

/*Calcolo del totale degli output non spesi appartenenti ad un certo indirizzo*/
float getBalance(string address, vector<UnspentTxOut> unspentTxOuts){
  if(debug == 1){
    cout << endl << "Wallet - getBalance" << endl;
  }
  float total = 0;
  vector<UnspentTxOut> unspentTxOutsOfAddress = findUnspentTxOutsOfAddress(address, unspentTxOuts);
  vector<UnspentTxOut>::iterator it;
  for(it = unspentTxOutsOfAddress.begin(); it != unspentTxOutsOfAddress.end(); ++it){
    total = total + it->amount;
  }
    return total;
}

/*Calcola il totale data una lista di output non spesi*/
float getTotalFromOutputVector(vector<UnspentTxOut> unspentTxOuts){
  if(debug == 1){
    cout << endl << "Wallet - getTotalFromOutputVector" << endl;
  }
  float total = 0;
  vector<UnspentTxOut>::iterator it;
  for(it = unspentTxOuts.begin(); it != unspentTxOuts.end(); ++it){
    total = total + it->amount;
  }
    return total;
}

/*Trova gli output non spesi necessari ad eseguire un pagamento dato il totale
 e la lista degli output non spesi dell'utente che sta effettuando il pagamento.
Viene ritornata la lista degli output non spesi che saranno utilizzati,
inoltre si assegna il valore di un puntatore contenente il valore superfluo
che dovra essere indirizzato al mittente stesso*/
vector<UnspentTxOut> findTxOutsForAmount(float amount, vector<UnspentTxOut> myUnspentTxOuts, float *leftOverAmount){
  if(debug == 1){
    cout << endl << "Wallet - findTxOutsForAmount" << endl;
  }
  float currentAmount = 0;
  vector<UnspentTxOut> includedUnspentTxOuts;
  vector<UnspentTxOut>::iterator it;
  for (it = myUnspentTxOuts.begin(); it != myUnspentTxOuts.end(); ++it) {
    includedUnspentTxOuts.push_back(*it);
    currentAmount = currentAmount + it->amount;
    if (currentAmount >= amount) {
        *leftOverAmount = currentAmount - amount;
        return includedUnspentTxOuts;
    }
  }
  throw "EXCEPTION (findTxOutsForAmount): Impossibile creare la transazione, il wallet sorgente non contiene abbastanza skancoin!";
}

/*Generazione degli output di transazione dati l'importo e la
differenza che deve tornare al mittente*/
vector<TxOut> createTxOuts(string receiverAddress, string myAddress, float amount, float leftOverAmount){
  if(debug == 1){
    cout << endl << "Wallet - createTxOuts" << endl;
  }
  TxOut txOut1 = TxOut(receiverAddress, amount);
  if (leftOverAmount == 0) {
      return {txOut1};
  } else {
      TxOut leftOverTx = TxOut(myAddress, leftOverAmount);
      return {txOut1, leftOverTx};
  }
}

/*Verifica se l'output non speso (1° parametro) è referenziato da un
input nella lista txIns (2° parametro)*/
bool isReferencedUnspentTxOut(UnspentTxOut uTxOut, vector<TxIn> txIns){
  if(debug == 1){
    cout << endl << "Wallet - isReferencedUnspentTxOut" << endl;
  }
  vector<TxIn>::iterator it;
  for(it = txIns.begin(); it != txIns.end(); ++it){
    if(it->txOutIndex == uTxOut.txOutIndex && it->txOutId == uTxOut.txOutId){
      return true;
    }
  }
  return false;
}

/*Ritorna la lista di tutti gli output non spesi, senza quelli che sono
referenziati da un qualche input in una delle transazioni presenti nella transaction pool*/
vector<UnspentTxOut> filterTxPoolTxs(vector<UnspentTxOut> unspentTxOuts, vector<Transaction> transactionPool){
  if(debug == 1){
    cout << endl << "Wallet - filterTxPoolTxs" << endl;
  }
  vector<TxIn> txIns;

  vector<Transaction>::iterator it;
  vector<TxIn>::iterator it2;
  for(it = transactionPool.begin(); it != transactionPool.end(); ++it){
    for(it2 = it->txIns.begin(); it2 != it->txIns.end(); ++it2){
      txIns.push_back(*it2);
    }
  }
  vector<UnspentTxOut> res;
  //copia del vettore per evitare side effects, faccio solo una allocazione per avere migliori prestazioni
  res.reserve(res.size() + unspentTxOuts.size());
  res.insert(res.end(), unspentTxOuts.begin(), unspentTxOuts.end());

  /*Per ogni output non speso cerco nel vettore di txIn quella che fa riferimento
  //ad esso, se la trovo rimuovo dal vettore l'output non speso*/
  vector<UnspentTxOut>::iterator it3;
  for(it3 = res.begin(); it3 != res.end();){
    if (isReferencedUnspentTxOut(*it3, txIns)) {
      it3 = res.erase(it3);
    } else {
      ++it3;
    }
  }
    return res;
}

/*Creazione di un input di transazione a partire da un output non speso
a cui questo farà riferimento*/
TxIn toUnsignedTxIn(UnspentTxOut unspentTxOut){
  if(debug == 1){
    cout << endl << "Wallet - toUnsignedTxIn" << endl;
  }
  TxIn txIn = TxIn();
  txIn.txOutId = unspentTxOut.txOutId;
  txIn.txOutIndex = unspentTxOut.txOutIndex;
  return txIn;
}

/*Generazione di una nuova transazione*/
Transaction createTransaction(string receiverAddress, float amount, string privateKey, vector<UnspentTxOut> unspentTxOuts, vector<Transaction> txPool){
  if(debug == 1){
    cout << endl << "Wallet - createTransaction" << endl;
  }
  //Ottengo la lista degli output non spesi per il mio wallet
  string myAddress = getPublicFromWallet();
  vector<UnspentTxOut> myUnspentTxOutsA = findUnspentTxOutsOfAddress(myAddress, unspentTxOuts);

  /*Ottengo la lista dei miei output non spesi a cui sono stati togli gli output
   che vengono usati come input in una delle transazioni nel pool*/
  vector<UnspentTxOut> myUnspentTxOuts = filterTxPoolTxs(myUnspentTxOutsA, txPool);
  float leftOverAmount;

  /*Cerco tra gli output non spesi quelli necessari per il nuovo input
  (devo "raccogliere" un ammontare pari ad amount),
  gli output che sto usando saranno nel nuovo vettore includedUnspentTxOuts*/
  vector<UnspentTxOut> includedUnspentTxOuts;
  try{
    includedUnspentTxOuts= findTxOutsForAmount(amount, myUnspentTxOuts, &leftOverAmount);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (createTransaction): Impossibile creare la transazione, il mittente non possiede abbastanza skanCoin!";
  }
  /*Raccolti gli output non spesi da usare di creano i rispettivi input per la nuova transazione
   (che faranno riferimento ad essi), questi devono ancora essere firmati!*/
  vector<TxIn> unsignedTxIns;
  vector<UnspentTxOut>::iterator it2;
  for(it2 = includedUnspentTxOuts.begin(); it2 != includedUnspentTxOuts.end(); ++it2){
    unsignedTxIns.push_back(toUnsignedTxIn(*it2));
  }
  //Creo la transazione, gli input non sono ancora firmati
  Transaction tx = Transaction();
  tx.txIns = unsignedTxIns;
  tx.txOuts = createTxOuts(receiverAddress, myAddress, amount, leftOverAmount);
  tx.id = getTransactionId(tx);
  int index = 0;
  try{
    //firma di tutti gli input della nuova transazione
    vector<TxIn>::iterator it3;
    for(it3 = tx.txIns.begin(); it3 != tx.txIns.end(); ++it3){
      it3->signature = signTxIn(tx, index, privateKey, unspentTxOuts);
      index++;
    }
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (createTransaction): Creazione della transazione fallita, alcuni input di transazione non sono stati firmati correttamente";
  }
  return tx;
}

/*Generazione di una nuova transazione, a partire da un array di output, per
permettere di esporre la rest per effettuare più movimenti dallo stesso wallet
in un solo blocco senza passare dal transaction pool*/
Transaction createTransactionWithMultipleOutputs (std::vector<TxOut> txOuts, std::string privateKey, std::vector<UnspentTxOut> unspentTxOuts, std::vector<Transaction> txPool){
  if(debug == 1){
    cout << endl << "Wallet - createTransactionWithMultipleOutputs" << endl;
  }
  //Ottengo la lista degli output non spesi per il mio wallet
  string myAddress = getPublicFromWallet();
  vector<UnspentTxOut> myUnspentTxOutsA = findUnspentTxOutsOfAddress(myAddress, unspentTxOuts);

  /*Ottengo la lista dei miei output non spesi a cui sono stati togli gli output
   che vengono usati come input in una delle transazioni nel pool*/
  vector<UnspentTxOut> myUnspentTxOuts = filterTxPoolTxs(myUnspentTxOutsA, txPool);
  float leftOverAmount;

  /*Cerco tra gli output non spesi quelli necessari per il nuovo input
  (devo "raccogliere" un ammontare pari ad amount),
  gli output che sto usando saranno nel nuovo vettore includedUnspentTxOuts*/
  vector<UnspentTxOut> includedUnspentTxOuts;

  //Calcolo il totale degli output
  float amount = 0;
  vector<TxOut>::iterator it;
  for(it = txOuts.begin(); it != txOuts.end(); ++it){
    amount += it->amount;
  }
  try{
    includedUnspentTxOuts= findTxOutsForAmount(amount, myUnspentTxOuts, &leftOverAmount);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (createTransaction): Impossibile creare la transazione, il mittente non possiede abbastanza skanCoin!";
  }
  /*Raccolti gli output non spesi da usare di creano i rispettivi input per la nuova transazione
   (che faranno riferimento ad essi), questi devono ancora essere firmati!*/
  vector<TxIn> unsignedTxIns;
  vector<UnspentTxOut>::iterator it2;
  for(it2 = includedUnspentTxOuts.begin(); it2 != includedUnspentTxOuts.end(); ++it2){
    unsignedTxIns.push_back(toUnsignedTxIn(*it2));
  }
  //Creo la transazione, gli input non sono ancora firmati
  Transaction tx = Transaction();
  tx.txIns = unsignedTxIns;
  tx.txOuts = txOuts;
  if (leftOverAmount != 0) {
      TxOut leftOverTx = TxOut(myAddress, leftOverAmount);
      tx.txOuts.push_back(leftOverTx);
  }
  tx.id = getTransactionId(tx);
  int index = 0;
  try{
    //firma di tutti gli input della nuova transazione
    vector<TxIn>::iterator it3;
    for(it3 = tx.txIns.begin(); it3 != tx.txIns.end(); ++it3){
      it3->signature = signTxIn(tx, index, privateKey, unspentTxOuts);
      index++;
    }
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (createTransaction): Creazione della transazione fallita, alcuni input di transazione non sono stati firmati correttamente";
  }
  return tx;
}
