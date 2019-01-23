#include "Wallet.hpp"

using namespace std;

//TODO: DEFINIZIONE CURVA ELLITTICA, trovare libreria adatta per le funzioni ECDSA

//Legge la chiave privata (wallet) del nodo dal file
string getPrivateFromWallet(){
  ifstream infile { privateKeyLocation };
  string fileContent { istreambuf_iterator<char>(infile), istreambuf_iterator<char>() };
  return fileContent;
}

//Legge la chiave privata del nodo dal file, da essa genera la chiave pubblica e la ritorna
string getPublicFromWallet(){
    string privateKey = getPrivateFromWallet();
    //TODO usare funzione per ottenere la chiave pubblica da quella privata appena ottenuta, ritornare la chiave pubblica
    return "04bfcab8722991ae774db48f934ca79cfb7dd991229153b9f732ba5334aafcd8e7266e47076996b55a14bf9913ee3145ce0cfc1372ada8ada74bd287450313534a";
}

//Generazione di una nuova chiave privata
string generatePrivateKey(){
    //TODO funzione per generare chiave privata, ritornare il risultato
    return "privata";
}

//inizializzazione del wallet (chiave privata), se il file non esiste si genera una nuova chiave e si salva su file
void initWallet(){
    //controllo se esiste il file con la chiave privata
    if (FILE* file = fopen(privateKeyLocation.c_str(), "r")) {
      fclose(file);
      return;
    }

    //genero nuova chiave
    string newPrivateKey = generatePrivateKey();

    //salvataggio nuova chiave su file
    ofstream out(privateKeyLocation);
    out << newPrivateKey;
    out.close();
    cout << "new wallet with private key created" << endl;;
}

//delete the file containing the wallet if it exists
void deleteWallet(){
  if (FILE* file = fopen(privateKeyLocation.c_str(), "r")) {
    fclose(file);

    if( remove( privateKeyLocation.c_str() ) != 0 ){
      cout << "Error deleting file" << endl;
    }
    else{
      cout << "File successfully deleted" << endl;
    }
    return;
  }
}

//ritorna la lista degli output non spesi appartenenti ad un certo indirizzo (wallet)
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

//calcola il totale degli output non spesi appartenenti ad un certo indirizzo
float getBalance(string address, vector<UnspentTxOut> unspentTxOuts){
  float total = 0;
  vector<UnspentTxOut> unspentTxOutsOfAddress = findUnspentTxOutsOfAddress(address, unspentTxOuts);
  vector<UnspentTxOut>::iterator it;
  for(it = unspentTxOutsOfAddress.begin(); it != unspentTxOutsOfAddress.end(); ++it){
    total = total + it->amount;
  }
    return total;
}

//calcola il totale data una lista di output non spesi
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
  throw "EXCEPTION: Cannot create transaction from the available unspent transaction outputs (not enough coins)";
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

//Ritorna la lista di tutti gli output non spesi, senza quelli che sono referenziati da un qualche input in una delle transazioni presenti nella transaction pool
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

  //per ogni output non speso cerco nel vettore di txIn quella che fa riferimento ad esso, se la trovo rimuovo dal vettore l'output non speso
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

Transaction createTransaction(string receiverAddress, float amount, string privateKey, vector<UnspentTxOut> unspentTxOuts, vector<Transaction> txPool){
  //Stampa tutte le transazioni nel pool
  cout << "txPool: " << endl;
  vector<Transaction>::iterator it;
  for(it = txPool.begin(); it != txPool.end(); ++it){
    cout << it->toString();
  }
  //Ottengo la lista degli output non spesi per il mio wallet
  string myAddress = getPublicKey(privateKey);
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
    throw "EXCEPTION: Transaction creation failed, sender does not own enough coins!";
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
    throw "EXCEPTION: Transaction creation failed, could not sign some of the TxIns";
  }
  return tx;
}
