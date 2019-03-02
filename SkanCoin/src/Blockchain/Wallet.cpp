#include "Wallet.hpp"

using namespace std;

//Carica la chiave (pubblica o privata) dall'apposito file
void loadKey(bool isPrivate, uint8_t *dest){
  string location;
  if(isPrivate == false){
    location = publicKeyLocation;
  }else{
    location = privateKeyLocation;
  }
  ifstream inFile;
  inFile.open(location);
  if(inFile.is_open()) {
    int i = 0;
      while (!inFile.eof()) {
          inFile >> dest[i];
          i++;
      }
  } else {
    throw "EXCEPTION: non è stato possibile aprire il file per leggere la chiave!";
  }
}

//Salvataggio della chiave (pubblica o privata) nell'apposito file
void saveKey(uint8_t *key, bool isPrivate){
  int len;
  string location;
  if(isPrivate == false){
    len = ECC_BYTES+1;
    location = publicKeyLocation;
  }else{
    len = ECC_BYTES;
    location = privateKeyLocation;
  }
  ofstream myfile;
  myfile.open (location, ios::out | ios::trunc);
  if(myfile.is_open()) {
    for(int i = 0; i < len; i++){
      myfile << key[i] << endl;
    }
  } else {
    throw "EXCEPTION: non è stato possibile aprire il file per salvare la chiave!";
  }
}

//Effettuiamo una conversione a stringa sulle chiavi che sia reversibile
//per semplificare la gestione delle chiavi nelle classi esterne

//Una conversione analoga viene applicata alle signature, per avere un formato
//stampabile e più facilmente interpretabile nelle interazioni tra i peer e
//e nei payload delle richieste/risposte HTTP
string stringFromByteArray(uint8_t *array, int len){
  string ret = "";
  for (int a = 0; a < len; a++) {
    if(a != 0){
      ret += ".";
    }
    ret +=  to_string((int)array[a]);
  }
  return ret;
}

void byteArrayFromString(string str, uint8_t *dest){
  stringstream tempstream(str);
  string segment;
  int x;

  //Conversione all'indietro da stringa ad array di uint8_t della chiave pubblica
  int i = 0;
  while(std::getline(tempstream, segment, '.')){
    x = stoi(segment);
    dest[i] = x;
    i++;
  }
}



//Legge la chiave privata del nodo dal file
string getPrivateFromWallet(){
  try{
    uint8_t privateKey[ECC_BYTES];
    loadKey(true, privateKey);
    //Effettuiamo una conversione a stringa che sia reversibile per semplificare
    // la gestione delle chiavi nelle classi esterne
    return stringFromByteArray(privateKey, ECC_BYTES);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION: Errore durante il caricamento della chiave privata!";
  }
}

//Legge la chiave pubblica del nodo dal file
string getPublicFromWallet(){
  try{
    uint8_t publicKey[ECC_BYTES+1];
    loadKey(false, publicKey);
    //Effettuiamo una conversione a stringa che sia reversibile per semplificare
    // la gestione delle chiavi nelle classi esterne
    return stringFromByteArray(publicKey, ECC_BYTES+1);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION: Errore durante il caricamento della chiave pubblica!";
  }
}

//Controllo per eventuali chiavi già esistenti, in caso non esistano già si effettua
//la generazione di una nuova coppia di chiavi e il salvataggio negi appositi file
void generateKeys(){
  uint8_t p_publicKey[ECC_BYTES+1];
  uint8_t p_privateKey[ECC_BYTES];
  bool valid = false;

  while(valid == false){
    //Generazione nuova coppia di chiavi
    int createdKeys = ecc_make_key(p_publicKey, p_privateKey);

    //La generazione non è andata a buon fine
    if(createdKeys != 1){
      continue;
    }

    //Salvataggio delle chiavi negli appositi files
    try{
      saveKey(p_publicKey, false);
      saveKey(p_privateKey, true);
    }catch(const char* msg){
      cout << msg << endl;
      cout << endl;
      throw "EXCEPTION: Errore durante il salvataggio delle chiavi";
    }

    //Vogliamo ottenere una conversione a stringa che sia reversibile
    //In modo da semplificare la gestione all'esterno, in particolare utilizziamo
    //una notazione puntata in cui ogni byte viene convertito in un intero e successivamente
    //tutti gli interi vengono concatenati, usando come carattere di separazione un punto.
    string tempPublic = stringFromByteArray(p_publicKey, ECC_BYTES+1);
    string tempPrivate = stringFromByteArray(p_privateKey, ECC_BYTES);

    //Effettuo una conversione all'indietro per poi verificare che sulla la coppia di
    //chiavi generate la conversione sia fattibile senza perdite di informazioni.
    //Per evitare errori di conversione accetto solo chiavi che non contengono
    //caratteri senza una corrispondenza biunivoca con i valori stampabili su file
    //NOTA: ovviamente ogni possibile valore di uint8_t ha una corrispondenza biunivoca
    //con un intero, tuttavia la presenza di un salvataggio/recupero da file restringe
    //il set di valori accettabili a quelli stampabili su file
    byteArrayFromString(tempPublic, p_publicKey);
    byteArrayFromString(tempPrivate, p_privateKey);

    //Recupero delle chiavi da file per la verifica di reversibilità della conversione
    try{
      uint8_t new_private[ECC_BYTES];
      loadKey(true, new_private);
      uint8_t new_public[ECC_BYTES+1];
      loadKey(false, new_public);
      //Verifica della correttezza della conversione effettuata
      valid = true;
      if(p_publicKey[0] != new_public[0]){
        valid = false;
        continue;
      }
      for(int i = 0; i < ECC_BYTES; i++){
        if(p_publicKey[i+1] != new_public[i+1] || p_privateKey[i] != new_private[i]){
          valid = false;
          continue;
        }
      }
    }catch(const char* msg){
      cout << msg << endl;
      cout << endl;
      throw "EXCEPTION: Errore durante il caricamento delle chiavi!";
    }
  }
}


//Inizializzazione del wallet (chiave privata), se il file non esiste si genera una nuova chiave e si salva su file
void initWallet(){
    //controllo se esiste il file con la chiave privata
    if (FILE* file = fopen(privateKeyLocation.c_str(), "r")) {
      fclose(file);
      cout << "Foud existing wallet" << endl;
      return;
    }

    //genero nuova coppia di chiavi ed effettuo il salvataggio su file
    try{
        generateKeys();
        cout << "New wallet created!" << endl;
    }catch(const char* msg){
      cout << msg << endl;
      cout << endl;
      throw "EXCEPTION: Transaction creation failed, sender does not own enough coins!";
    }
}

//Eliminazione del wallet (file contenenti le chiavi)
void deleteWallet(){
  if (FILE* file = fopen(privateKeyLocation.c_str(), "r")) {
    fclose(file);

    if( remove( privateKeyLocation.c_str() ) != 0 || remove( publicKeyLocation.c_str() ) != 0){
      cout << "Error deleting file" << endl;
    }
    else{
      cout << "Files successfully deleted" << endl;
    }
    return;
  }
}

//Ritorna la lista degli output non spesi appartenenti ad un certo indirizzo (wallet)
vector<UnspentTxOut> findUnspentTxOutsOfAddress(string ownerAddress, vector<UnspentTxOut> unspentTxOuts){
  vector<UnspentTxOut> res;
  //per fare una copia del vettore faccio solo una allocazione per avere migliori prestazioni
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

//Totale degli output non spesi appartenenti ad un certo indirizzo
float getBalance(string address, vector<UnspentTxOut> unspentTxOuts){
  float total = 0;
  vector<UnspentTxOut> unspentTxOutsOfAddress = findUnspentTxOutsOfAddress(address, unspentTxOuts);
  vector<UnspentTxOut>::iterator it;
  for(it = unspentTxOutsOfAddress.begin(); it != unspentTxOutsOfAddress.end(); ++it){
    total = total + it->amount;
  }
    return total;
}

//Calcola il totale data una lista di output non spesi
float getTotalFromOutputVector(vector<UnspentTxOut> unspentTxOuts){
  float total = 0;
  vector<UnspentTxOut>::iterator it;
  for(it = unspentTxOuts.begin(); it != unspentTxOuts.end(); ++it){
    total = total + it->amount;
  }
    return total;
}

//Trova gli output non spesi necessari ad eseguire un pagamento dato il totale e la lista degli output non spesi dell'utente che sta effettuando il pagamento
//Viene ritornata la lista degli output non spesi che saranno utilizzati, inoltre si assegna il valore di un puntatore contenente il valore superfluo che dovra essere indirizzato al mittente stesso
vector<UnspentTxOut> findTxOutsForAmount(float amount, vector<UnspentTxOut> myUnspentTxOuts, float *leftOverAmount){
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
  throw "EXCEPTION: Impossibile creare la transazione, il wallet sorgente non contiene abbastanza skancoin!";
}

//Generazione degli output di transazione dati l'amount e la differenza che deve tornare al mittente
vector<TxOut> createTxOuts(string receiverAddress, string myAddress, float amount, float leftOverAmount){
    TxOut txOut1 = TxOut(receiverAddress, amount);
    if (leftOverAmount == 0) {
        return {txOut1};
    } else {
        TxOut leftOverTx = TxOut(myAddress, leftOverAmount);
        return {txOut1, leftOverTx};
    }
}

//Verifica se l'output non speso (1° parametro) è referenziato da un input nella lista txIns (2° parametro)
bool isReferencedUnspentTxOut(UnspentTxOut uTxOut, vector<TxIn> txIns){
  vector<TxIn>::iterator it;
  for(it = txIns.begin(); it != txIns.end(); ++it){
    if(it->txOutIndex == uTxOut.txOutIndex && it->txOutId == uTxOut.txOutId){
      return true;
    }
  }
  return false;
}

//Ritorna la lista di tutti gli output non spesi, senza quelli che sono
//referenziati da un qualche input in una delle transazioni presenti nella transaction pool
vector<UnspentTxOut> filterTxPoolTxs(vector<UnspentTxOut> unspentTxOuts, vector<Transaction> transactionPool){
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

  //per ogni output non speso cerco nel vettore di txIn quella che fa riferimento
  //ad esso, se la trovo rimuovo dal vettore l'output non speso
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

TxIn toUnsignedTxIn(UnspentTxOut unspentTxOut){
    TxIn txIn = TxIn();
    txIn.txOutId = unspentTxOut.txOutId;
    txIn.txOutIndex = unspentTxOut.txOutIndex;
    return txIn;
}

//Generazione di una nuova transazione
Transaction createTransaction(string receiverAddress, float amount, string privateKey, vector<UnspentTxOut> unspentTxOuts, vector<Transaction> txPool){
  //Stampa tutte le transazioni nel pool
  cout << "txPool: " << endl;
  vector<Transaction>::iterator it;
  for(it = txPool.begin(); it != txPool.end(); ++it){
    cout << it->toString();
  }
  //Ottengo la lista degli output non spesi per il mio wallet
  string myAddress = getPublicKey();
  vector<UnspentTxOut> myUnspentTxOutsA = findUnspentTxOutsOfAddress(myAddress, unspentTxOuts);
  //Ottengo la lista dei miei output non spesi a cui sono stati togli gli outpu che vengono usati come input in una delle transazioni nel pool
  vector<UnspentTxOut> myUnspentTxOuts = filterTxPoolTxs(myUnspentTxOutsA, txPool);
  float leftOverAmount;
  //Cerco tra gli output non spesi quelli necessari per il nuovo input (devo "raccogliere" un ammontare pari ad amount), gli output che sto usando saranno nel nuovo vettore includedUnspentTxOuts
  vector<UnspentTxOut> includedUnspentTxOuts;
  try{
    includedUnspentTxOuts= findTxOutsForAmount(amount, myUnspentTxOuts, &leftOverAmount);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION: Impossibile creare la transazione, il mittente nonpossiede abbastanza skanCoin!";
  }
  //raccolti gli outpud da usare di creano i rispettivi input per la nuova transazione (che faranno riferimento ad essi), ANCORA NON SONO FIRMATI!
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
    throw "EXCEPTION: Creazione della transazione fallita, alcuni input di transazione non sono stati firmati correttamente";
  }
  return tx;
}
