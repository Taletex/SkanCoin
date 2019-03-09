#include "Wallet.hpp"

using namespace std;

/*Carica la chiave (pubblica o privata) dall'apposito file*/
string loadKey(bool isPrivate){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

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
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

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
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

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
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

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
string getWalletPrivateKey(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  try{
    return loadKey(true);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (getWalletPrivateKey): Errore durante il caricamento della chiave privata!";
  }
}

/*Legge la chiave pubblica del nodo dal file*/
string getWalletPublicKey(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  try{
    return loadKey(false);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION (getWalletPublicKey): Errore durante il caricamento della chiave pubblica!";
  }
}

/*Generazione di una nuova coppia di chiavi e salvataggio negi appositi file*/
void generateKeys(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

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
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

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
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

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
vector<UnspentTransOut> getUnspentTransOutsOfAddress(string ownerAddress, vector<UnspentTransOut> unspentTransOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<UnspentTransOut> res;
  //Per fare una copia del vettore faccio solo una allocazione per avere migliori prestazioni
  res.reserve(res.size() + unspentTransOuts.size());
  res.insert(res.end(), unspentTransOuts.begin(), unspentTransOuts.end());

  vector<UnspentTransOut>::iterator it;
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
float getBalance(string address, vector<UnspentTransOut> unspentTransOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  float total = 0;
  vector<UnspentTransOut> unspentTransOutsOfAddress = getUnspentTransOutsOfAddress(address, unspentTransOuts);
  vector<UnspentTransOut>::iterator it;
  for(it = unspentTransOutsOfAddress.begin(); it != unspentTransOutsOfAddress.end(); ++it){
    total = total + it->amount;
  }
    return total;
}

/*Calcola il totale data una lista di output non spesi*/
float getTotalFromOutputVector(vector<UnspentTransOut> unspentTransOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  float total = 0;
  vector<UnspentTransOut>::iterator it;
  for(it = unspentTransOuts.begin(); it != unspentTransOuts.end(); ++it){
    total = total + it->amount;
  }
    return total;
}

/*Trova gli output non spesi necessari ad eseguire un pagamento dato il totale
 e la lista degli output non spesi dell'utente che sta effettuando il pagamento.
Viene ritornata la lista degli output non spesi che saranno utilizzati,
inoltre si assegna il valore di un puntatore contenente il valore superfluo
che dovra essere indirizzato al mittente stesso*/
vector<UnspentTransOut> getTransOutsForAmount(float amount, vector<UnspentTransOut> myUnspentTransOuts, float *leftOverAmount){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  float currentAmount = 0;
  vector<UnspentTransOut> includedUnspentTransOuts;
  vector<UnspentTransOut>::iterator it;
  for (it = myUnspentTransOuts.begin(); it != myUnspentTransOuts.end(); ++it) {
    includedUnspentTransOuts.push_back(*it);
    currentAmount = currentAmount + it->amount;
    if (currentAmount >= amount) {
        *leftOverAmount = currentAmount - amount;
        return includedUnspentTransOuts;
    }
  }
  throw "EXCEPTION (getTransOutsForAmount): Impossibile creare la transazione, il wallet sorgente non contiene abbastanza skancoin!";
}

/*Generazione degli output di transazione dati l'importo e la
differenza che deve tornare al mittente*/
vector<TransOut> createTransOuts(string receiverAddress, string myAddress, float amount, float leftOverAmount){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  TransOut transOut1 = TransOut(receiverAddress, amount);
  if (leftOverAmount == 0) {
      return {transOut1};
  } else {
      TransOut leftOverTx = TransOut(myAddress, leftOverAmount);
      return {transOut1, leftOverTx};
  }
}

/*Verifica se l'output non speso (1° parametro) è referenziato da un
input nella lista transIns (2° parametro)*/
bool isReferencedUnspentTransOut(UnspentTransOut uTransOut, vector<TransIn> transIns){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<TransIn>::iterator it;
  for(it = transIns.begin(); it != transIns.end(); ++it){
    if(it->transOutIndex == uTransOut.transOutIndex && it->transOutId == uTransOut.transOutId){
      return true;
    }
  }
  return false;
}

/*Verifica della firma, effettuo le conversioni ad array di byte, per ottenere
un formato compatibile con l'interfaccia della libreria per ECDSA
Funzione utilizzata per verificare la firma posta su un input di Transazione
che deve corrispondere alla firma dell'id della transazione effettuata con la
chiave privata del proprietario dell'output di transazione referenziato */
int signVerify(string key, string hash, string signature) {
  //Chiave pubblica del proprietario dell'output non speso
  uint8_t p_public[ECC_BYTES+1];
  byteArrayFromString(key, p_public);

  /*Id della transazione (hash su cui è stata fatta la firma). Su questo elemento
  non applichiamo la conversione implementata in wallet, poiche questo hash
  è una normale stringa che non rispetta la notazione puntata utilizzata nelle
  nostre conversioni, dunque si converte semplicemente ogni carattere in un byte*/
  uint8_t p_hash[ECC_BYTES];
  memcpy (p_hash, hash.c_str(), ECC_BYTES);

  //Signature creata con la chiave privata del proprietario dell'output non speso
  uint8_t p_signature[ECC_BYTES*2];
  byteArrayFromString(signature, p_signature);

  return ecdsa_verify(p_public, p_hash , p_signature);
}

/* Genera una signature a partire da una chiave privata e un hash da firmare.
Viene utilizzato in transactions durante la firma degli input degli input di
transazione. Se la firma va a buon fine il metodo ritorna 1 e il puntatore a
stringa passato come terzo parametro viene riempito con la signature prodotta */
int createSignature(string key, string hash, string& signature) {
  //Generazione della firma a partire dall'hash (id della transazione)
  uint8_t p_hash[ECC_BYTES];
  memcpy (p_hash, hash.c_str(), ECC_BYTES);
  uint8_t p_private[ECC_BYTES];
  byteArrayFromString(key, p_private);
  uint8_t p_signature[ECC_BYTES*2];
  int signCreated = ecdsa_sign(p_private, p_hash, p_signature);
  if(signCreated == 1) {
    signature = stringFromByteArray(p_signature, ECC_BYTES*2);
  }

  return signCreated;
}
