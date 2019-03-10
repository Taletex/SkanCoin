#include "Blockchain.hpp"

using namespace std;

BlockChain::BlockChain(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  Block genesisBlock = getGenesisBlock();
  blockchain = {genesisBlock};
  try{
    this->unspentTransOuts = processTransactions(blockchain.front().data, unspentTransOuts, 0);
    saveBlockchainStats();
    initStatFiles();
  }catch(const char* msg){
    cout << msg << endl << endl;
    throw "ECCEZIONE (BlockChain): Creazione della blockchain fallita!";
  }
}

/*Generazione blocco di genesi (il primo blocco della blockchain)*/
Block BlockChain::getGenesisBlock(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<TransIn> transInsVector = {TransIn("","",0)};
  vector<TransOut> transOutsVector = {TransOut(getWalletPublicKey(), COINBASE_AMOUNT)};
  Transaction genesisTransaction("", transInsVector, transOutsVector);
  genesisTransaction.id = getTransactionId(genesisTransaction);
  vector<Transaction> transactionsVector = {genesisTransaction};
  Block ret(0, "", "", time(nullptr), transactionsVector, 0, 0);
  ret.hash = calculateHash(ret.index, ret.previousHash, ret.timestamp , ret.data, ret.difficulty, ret.nonce);
  return ret;
}

/*Rappresentazione in stringa della blockchain*/
string BlockChain::toString(){
  string ret = "[";
  list<Block>::iterator it;
  for(it = blockchain.begin(); it != blockchain.end(); ++it){
    if(it != blockchain.begin()){
      ret = ret + ", ";
    }
    ret = ret + it->toString();
  }
  ret = ret + "]";
  return ret;
}

/*Tabella di conversione dei caratteri esadecimali in byte,
usata per verificare la correttezza (difficoltà) degli hash*/
string BlockChain::hexToBinaryLookup(char c){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  string ret= "";
  switch(c){
      case '0': ret = "0000";
      break;
      case '1': ret = "0001";
      break;
      case '2': ret = "0010";
      break;
      case '3': ret = "0011";
      break;
      case '4': ret = "0100";
      break;
      case '5': ret = "0101";
      break;
      case '6': ret = "0110";
      break;
      case '7': ret = "0111";
      break;
      case '8': ret = "1000";
      break;
      case '9': ret = "1001";
      break;
      case 'a': ret = "1010";
      break;
      case 'b': ret = "1011";
      break;
      case 'c': ret = "1100";
      break;
      case 'd': ret = "1101";
      break;
      case 'e': ret = "1110";
      break;
      case 'f': ret = "1111";
      break;
      default: ret = "";
  }
  return ret;
}

/*Converte una stringa esadecimale in una sequenza binaria, in modo da poter
 verificare il numero di zeri iniziali (difficoltà) durante la validazione del blocco*/
string BlockChain::hexToBinary(string s){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  string ret = "";
  for(char& c : s) {
    ret = ret + hexToBinaryLookup(c);
  }
  return ret;
}

/*Ritorna la BlockChain*/
list<Block> BlockChain::getBlockchain(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return blockchain;
}

/*Ritorna il vettore di output non spesi*/
vector<UnspentTransOut> BlockChain::getUnspentTransOuts(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return unspentTransOuts;
}

/*Reimposta il vettore di output non spesi*/
void BlockChain::setUnspentTransOuts(vector<UnspentTransOut> newUnspentTransOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif
  unspentTransOuts.swap(newUnspentTransOuts);
}

/*Ritorna l'ultimo blocco della BlockChain*/
Block BlockChain::getLatestBlock(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return blockchain.back();
}

/*Ritorna un blocco dato il suo hash*/
Block BlockChain::getBlockFromHash(string hash){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  list<Block>::iterator it;
  for(it = blockchain.begin(); it != blockchain.end(); ++it){
    if(it->hash == hash){
      return *it;
    }
  }
  cout << endl;
  throw "ECCEZIONE (getBlockFromHash): Blocco non trovato!";
}

/*Calcola la nuova difficoltà per i blocchi*/
int BlockChain::computeNewDifficulty(Block latestBlock, list<Block> lBlockchain){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  list<Block>::iterator it = lBlockchain.begin();
  //Avanza l'iteratore fino al blocco in cui è cambiata l'ultima volta la difficoltà
  advance(it, (blockchain.size() - DIFFICULTY_ADJUSTMENT_INTERVAL));
  Block prevAdjustmentBlock = *it;
  //tempo atteso tra un aumento di diffoltà e l'altro
  long timeExpected = long(BLOCK_CREATION_INTERVAL * DIFFICULTY_ADJUSTMENT_INTERVAL);
  //tempo trascorso dall'ultimo aumento di difficoltà
  long timeTaken = latestBlock.timestamp - prevAdjustmentBlock.timestamp;

  //Si cerca sempre di fare in modo che i blocchi vengano aggiunti regolarmente con un intervallo pari a timeExpected
  if (timeTaken < timeExpected / 2) {
      return prevAdjustmentBlock.difficulty + 1;
  } else if (timeTaken > timeExpected * 2) {
      return prevAdjustmentBlock.difficulty - 1;
  } else {
      return prevAdjustmentBlock.difficulty;
  }
}

/*Ritorna la difficoltà per il prossimo blocco*/
int BlockChain::getDifficulty(list<Block> lBlockchain){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  Block latestBlock = lBlockchain.back();
  if ((latestBlock.index % DIFFICULTY_ADJUSTMENT_INTERVAL) == 0 && latestBlock.index != 0) {
      return computeNewDifficulty(latestBlock, lBlockchain);
  } else {
      return latestBlock.difficulty;
  }
}

/*Data la lista di transazioni da inserire nel blocco si esegue il mining del
blocco e si inserisce nella BlockChain*/
Block BlockChain::createRawNextBlock(vector<Transaction> blockData){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  Block previousBlock = getLatestBlock();
  int difficulty = getDifficulty(getBlockchain());
  int nextIndex = previousBlock.index + 1;
  long nextTimestamp =  time(nullptr);
  Block newBlock;
  double duration;
  try{
    //Mining del nuovo blocco
    newBlock = mineBlock(nextIndex, previousBlock.hash, nextTimestamp, blockData, difficulty, &duration);
  }catch(const char* msg){
    cout << endl;
    throw "ECCEZIONE (createRawNextBlock): Errore durante il mining del nuovo blocco";
  }
  if (addBlockToBlockchain(newBlock)) {
    cout << "Nuovo Blocco aggiunto alla blockchain! (indice " << newBlock.index << ")" << endl;
    string data = "";
    try{
      //Aggiornamento dell'apposito file per la raccolta di statistiche sul tempo di mining
      ifstream inFile;
      inFile.open("blocksminingtime.txt");
      string line;
      if(inFile.is_open()) {
        cout << "Aggiornamento del file contenente i tempi di mining..." << endl;
        while (getline(inFile, line)) {
          data = line;
        }
        inFile.close();
      } else {
        throw "ERRORE (createRawNextBlock): non è stato possibile aprire il file per salvare il tempo di mining del blocco!";
      }
    }catch(const char* msg){
      cout << msg << endl << endl;
      cout << "ERRORE (createRawNextBlock): Apertura del file contenente i tempi di mining fallita!";
    }
      Peer::getInstance().connectionsMtx.lock();
      cout << "Broadcast del blocco creato..." << endl;
      Peer::getInstance().broadcastLatestBlock(nextIndex, duration);
      Peer::getInstance().connectionsMtx.unlock();
      return newBlock;
  } else {
      cout << endl;
      throw "ECCEZIONE (createRawNextBlock): Errore durante l'inserimento del nuovo blocco nella BlockChain!";
  }
}

/*Ritorna la lista degli output non spesi appartenenti al nodo*/
vector<UnspentTransOut> BlockChain::getMyUnspentTransactionOutputs(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return getUnspentTransOutsOfAddress(getWalletPublicKey(), getUnspentTransOuts());
}

/*Colleziona le transazioni dal transaction pool, inizializza la coinbase
transaction ed avvia la procedura di mining ed inserimento del blocco*/
Block BlockChain::createNextBlock(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  Transaction coinbaseTrans = getCoinbaseTransaction(getWalletPublicKey(), getLatestBlock().index + 1);
  vector<Transaction> blockData = TransactionPool::getInstance().getPool();
  blockData.insert (blockData.begin() , coinbaseTrans);
  try{
    vector<string> stats = TransactionPool::getInstance().getStatStrings();
    Block ret = createRawNextBlock(blockData);
    /*Aggiornamento locale e verso tutti i peer delle statistiche relative ai tempi di attesa
    delle transazioni nel pool, vengono comunicati i tempi di attesa delle transazioni appena prelevate*/
    vector<string>::iterator it;
    if(stats.size() > 0){
      Peer::getInstance().connectionsMtx.lock();
      Peer::getInstance().broadcastPoolStat(stats);
      cout << "Broadcast dei tempi di attesa nel pool per la transazioni prelevate..." << endl;
      Peer::getInstance().connectionsMtx.unlock();
    }
    ofstream myfile;
    myfile.open ("transactionwaitingtime.txt", ios::out | ios::app);
    if(myfile.is_open()) {
      cout << "Aggiornamento del file contenente i tempi di attesa delle transazioni nel pool..." << endl;
      string msg = "";
      for(it = stats.begin(); it != stats.end(); ++it){
        msg = msg + *it + "\n";
      }
      myfile << msg;
      myfile.close();
    } else {
      throw "ECCEZIONE (clientMessageHandler - POOL_STATS): Blocco generato, ma non è stato possibile aprire il file per salvare le statistiche di attesa delle transazioni!";
    }
    return ret;
  }catch(const char* msg){
    cout << msg << endl << endl;
    throw "ECCEZIONE (createNextBlock): Errore durante la generazione del nuovo blocco";
  }
}

/*Genera un nuovo blocco con la coinbase e una transazione avente transOuts creato a
partire da un vettore di coppie [indirizzo destinazione, amount] e lo inserisce
nella BlockChain (transazion con multipli output) */
Block BlockChain::createNextBlockWithTransaction(vector<TransOut> transOuts){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<Transaction> blockData;
  vector<TransOut>::iterator it;
  blockData.push_back(getCoinbaseTransaction(getWalletPublicKey(), getLatestBlock().index + 1));

  for(it = transOuts.begin(); it != transOuts.end(); ++it) {
    if(typeid(it->amount) != typeid(float)) {
      cout << endl;
      throw "ECCEZIONE (createNextBlockWithTransaction): Importo non valido!";
    }
  }
  try{
    Transaction transaction = createTransactionWithMultipleOutputs (transOuts, getWalletPrivateKey(), getUnspentTransOuts(), TransactionPool::getInstance().getPool());
    blockData.push_back(transaction);
  }catch(const char* msg){
    cout << msg << endl << endl;
    throw "ECCEZIONE (createNextBlockWithTransaction): Creazione della transazione fallita";
  }
  try{
    return createRawNextBlock(blockData);
  }catch(const char* msg){
    cout << msg << endl << endl;
    throw "ECCEZIONE (createNextBlockWithTransaction): Errore durante la generazione del nuovo blocco";
  }
}

/*Questo metodo effettua il mining di un nuovo blocco, viengono generati
(e scartati) nuovi blocchi finche l'hash ottenuto non rispetta la difficoltà richiesta*/
Block BlockChain::mineBlock(int index, string previousHash, long timestamp, vector<Transaction> data, int difficulty, double *time) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif
  cout << "Mining di un nuovo blocco in corso..." << endl;
  int nonce = 0;
  string hash;
  double duration;
  clock_t start = clock();

  /*Calcolo dell'hash (mining), l'operazione viene ripetuta finchè l'hash non
  rispetta la difficoltà (numero di zeri iniziali) richiesta*/
  while (true) {
    hash = calculateHash(index, previousHash, timestamp, data, difficulty, nonce);
    if(hashDifficultyCheck(hash, difficulty)) {
      cout << "Mining concluso! Il nuovo blocco è stato creato..." << endl;
      /*Il blocco generato ha un hash valido, si effetua il calcolo ed il
       salvataggio del tempo di mining del blocco in secondi (per le statistiche)*/
      duration = (std::clock() - start)/((double)CLOCKS_PER_SEC / 1000);
      *time = duration;
      ofstream myfile;
      myfile.open ("blocksminingtime.txt", ios::out | ios::app);
      if(myfile.is_open()) {
        cout << "Aggiornamento del file contenente i tempi di mining in corso...";
        myfile << "{\"block\": " <<  index << ", \"miningtime\": " << duration << "}\n";
      } else {
        cout << "ERRORE (mineBlock): non è stato possibile aprire il file per salvare il tempo di mining del blocco!";
      }
      myfile.close();
      return Block(index, hash, previousHash, timestamp, data, difficulty, nonce);
    }
    nonce++;
  }
}

/*Ritorna il totale degli output non spesi nel wallet del nodo*/
float BlockChain::getAccountBalance(){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return getBalance(getWalletPublicKey(), getUnspentTransOuts());
}

/*Crea una nuova transazione e la inserisce nel transaction pool*/
Transaction BlockChain::sendTransaction(string address, float amount){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  Transaction transaction;
  try{
    transaction = createTransaction(address, amount, getWalletPrivateKey(), getUnspentTransOuts(), TransactionPool::getInstance().getPool());
  }catch(const char* msg){
    cout << msg << endl << endl;
    throw "ECCEZIONE (sendTransaction): Creazione delle transazione fallita";
  }
  try{
    //Aggiunta della nuova transazione al transaction pool
    TransactionPool::getInstance().addToPool(transaction, getUnspentTransOuts());
  }catch(const char* msg){
    cout << msg << endl << endl;
    throw "ECCEZIONE (sendTransaction):Inserimento della transazione nel pool fallito!";
  }
  //Broadcast a tutti gli altri peer della transactionpool aggiornata
  Peer::getInstance().connectionsMtx.lock();
  cout << "Broadcast del transaction pool dopo l'inserimento della nuova transazione..." << endl;
  Peer::getInstance().broadCastPool();
  Peer::getInstance().connectionsMtx.unlock();
  return transaction;
}

/*Ritorna una transazione della blockchain dato il suo id*/
Transaction BlockChain::getTransactionFromId(string transactionId){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  list<Block>::iterator it;
  vector<Transaction>::iterator it2;
  for(it = blockchain.begin(); it != blockchain.end(); ++it){
    for(it2 = it->data.begin(); it2 != it->data.end(); ++it2){
      if(it2->id == transactionId){
        return *it2;
      }
    }
  }
  cout << endl;
  throw "ECCEZIONE (getTransactionFromId): Transazione non presente nella blockchain!";
}

/*Calcolo dell'hash per un blocco*/
string BlockChain::calculateHash (int index, string previousHash, long timestamp, vector<Transaction> data, int difficulty, int nonce){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  string ret = to_string(index) + previousHash + to_string(timestamp);
  vector<Transaction>::iterator it;
  for(it = data.begin(); it != data.end(); ++it){
    ret = ret + it->toString();
  }
  ret = ret + to_string(difficulty) + to_string(nonce);
  return picosha2::hash256_hex_string(ret);
}

/*Validazione della struttura (type checking) del blocco*/
bool BlockChain::isWellFormedBlock(Block block){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return (
    typeid(block.index).name() == typeid(int).name() &&
    typeid(block.hash).name() == typeid(string).name() &&
    typeid(block.previousHash).name() == typeid(string).name() &&
    typeid(block.timestamp).name() == typeid(long).name() &&
    typeid(block.data).name() == typeid(vector<Transaction>).name() );
}

/*Calcolo della complessità del mining del prossimo blocco (numero di
zeri iniziali necessari nell'hash) della blockchain data*/
int BlockChain::getAccumulatedDifficulty(vector<Block> lBlockchain){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  int res = 0;
  vector<Block>::iterator it;
  for(it = lBlockchain.begin(); it != lBlockchain.end(); ++it){
    res = res + pow(2, it->difficulty);
  }
  return res;
}

/*Validazione del timestamp, per evitare che venga introdotto un timestamp falso in modo da
rendere valido un blocco con difficoltà inferiore a quella attuale vengono accettati solo i blocchi per
cui il mining è iniziato al massimo un minuto prima del mining dell'ultimo blocco ed al
massimo un minuto prima del tempo percepito dal nodo che effettua la validazione*/
bool BlockChain::isValidTimestamp(Block newBlock, Block previousBlock){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return (( previousBlock.timestamp - 60 < newBlock.timestamp ) && ( newBlock.timestamp - 60 < time(nullptr) ) );
}

/*Ricalcola l'hash del blocco e lo confronta con quello proposto (per rilevare modifiche)*/
bool BlockChain::blockHashCheck(Block block){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  string blockHash = calculateHash(block.index, block.previousHash, block.timestamp, block.data, block.difficulty, block.nonce);
  return blockHash == block.hash;
}

/*Controlla se l'hash rispetta la difficoltà minima (deve iniziare con un certo numero di zeri)*/
bool BlockChain::hashDifficultyCheck(string hash, int difficulty){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  string prefix = "";
  string hashInBinary = hexToBinary(hash);
  for(int i = 0; i < difficulty; i++){
    prefix += "0";
  }

  /*Controllo che i primi byte dell'hash siano zeri, fino a raggiungere
   un numero pari alla difficoltà attuale*/
  return (hashInBinary.substr(0, difficulty)).compare(prefix) == 0;
}

/*Controllo della validità dell'hash e del rispetto della difficoltà minima (proof of work)*/
bool BlockChain::hasValidHash(Block block){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if (!blockHashCheck(block)) {
      cout << "ERRORE (hasValidHash): Hash non valido:" << endl << block.hash << endl;
      return false;
  }
  if (!hashDifficultyCheck(block.hash, block.difficulty)) {
      cout << "ERRORE (hasValidHash): il blocco non soddisfa la difficoltà attesa(" << block.difficulty << "): " << block.hash << endl;
  }
  return true;
}

/*Validazione della struttura e della correttezza logica del blocco*/
bool BlockChain::isBlockValid(Block newBlock, Block previousBlock){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if (!isWellFormedBlock(newBlock)) { //Validazione struttura
    cout << "isBlockValid - invalid block structure: " <<  newBlock.toString();
    return false;
  }
  //Correttezza indice del blocco precedente
  if (previousBlock.index + 1 != newBlock.index) {
    cout << "isBlockValid - invalid index" << endl;
    return false;
  } else if (previousBlock.hash != newBlock.previousHash) {
    //Correttezza hash del blocco precedente
    cout << "isBlockValid - invalid previoushash";
    return false;
  } else if (!isValidTimestamp(newBlock, previousBlock)) {
    //Validità timestamp
    cout << "isBlockValid - invalid timestamp";
    return false;
  } else if (!hasValidHash(newBlock)) {
    //Validità hash
    return false;
  }
  return true;
}

/*Verifica la validità della blockchain ricevutaa, ritorna la lista aggiornata degli
output non spesi se questa è valida*/
vector<UnspentTransOut> BlockChain::isBlockchainValid(list<Block> blockchainToValidate) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  vector<UnspentTransOut> lUnspentTransOuts;
  list<Block>::iterator it1;
  list<Block>::iterator it2;

  //Validità del blocco di genesi
  if(blockchainToValidate.front().isEqual(getGenesisBlock())) {
    cout << endl;
    throw "ECCEZIONE (isBlockchainValid): il blocco di genesi non è valido!";
  }

  //Controllo validità di ogni blocco della blockchain (struttura e transazioni contenute)
  for(it1 = blockchainToValidate.begin(); it1 != blockchainToValidate.end(); ++it1) {

    if(it1 != blockchainToValidate.begin()) {
      it2 = it1;
      it2--;
      if(!isBlockValid(*it1, *it2)){
        cout << endl;
        throw "ECCEZIONE (isBlockchainValid): la blockchain ricevuta contiene blocchi non validi!";
      }
    }


    try{
      //Check e aggiornamento lista output non spesi in base alle
      //transazioni presenti nel blocco
      lUnspentTransOuts = processTransactions(it1->data, lUnspentTransOuts, it1->index);
    }catch(const char* msg){
      cout << msg << endl << endl;
      throw "ECCEZIONE (isBlockchainValid): la blockchain ricevuta contiene delle transazioni non valide!";
    }
  }
  return lUnspentTransOuts;
}

/*Aggiunta di un blocco alla blockchain*/
bool BlockChain::addBlockToBlockchain(Block newBlock) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  //Controllo validità del blocco
  if(isBlockValid(newBlock, getLatestBlock())) {
    vector<UnspentTransOut> ret;
    try{
      /*Check e aggiornamento lista output non spesi in base alle
      transazioni presenti nel blocco*/
      ret = processTransactions(newBlock.data, getUnspentTransOuts(), newBlock.index);
    }catch(const char* msg){
      cout << msg << endl;
      return false;
    }
    BlockChain::blockchain.push_back(newBlock);
    setUnspentTransOuts(ret);
    /*Check e aggiornamento del transactionPool (rimozione delle
    transazioni minate nel blocco ricevuto)*/
    cout << "Aggiornamento del transaction pool dopo l'inserimento del nuovo blocco...";
    TransactionPool::getInstance().updatePool(getUnspentTransOuts());
    try{
        cout << "Salvataggio delle statistiche della Blockchain..." << endl;
        saveBlockchainStats();
    }catch(const char* msg){
      cout << msg << endl << endl;
    }
    return true;
  }else{
    return false;
  }
}

/*Sostituzione blockchain con i blocchi ricevuti (se questa è
valida ed ha una difficoltà complessiva maggiore) si ricorda infatti che non
è valida la chain più lunga ma quella con la difficoltà cumulativa maggiore*/
void BlockChain::replaceChain(list<Block> newBlocks) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif
  int difficultyApproved = 0;


  if(getBlockchain().size() == 1 && newBlocks.size() == 1 && getBlockchain().back().timestamp > newBlocks.back().timestamp){
      //Si sta facendo un confronto tra due blocchi di genesi, si seleziona quello con timestamp minore
      difficultyApproved = 1;
  }else{
    /*Confronto della difficoltà cumulativa della blockchain attuale con quella ricevuta, a parità di difficoltà si
     * seleziona la blockchain per cui l'ultimo blocco ha timestamp mionre. Viene effettuata
     * una conversione da liste a vettori per migliorare le prestazioni del calcolo della difficoltà*/
    int newDiff = getAccumulatedDifficulty({ begin(newBlocks), end(newBlocks) });
    int localDiff = getAccumulatedDifficulty({begin(blockchain), end(blockchain)});
    if(newDiff > localDiff || (newDiff == localDiff && getBlockchain().back().timestamp > newBlocks.back().timestamp)) {
      difficultyApproved = 1;
    }
  }

  if(difficultyApproved == 0){
    cout << "INFO (replaceChain): La blockchain ricevuta è indietro rispetto a quella locale e verrà scartata...";
    return;
  }
  vector<UnspentTransOut> lUnspentTransOuts;
  try{
    //Verifica della validità dei blocchi ricevuti
    lUnspentTransOuts = isBlockchainValid(newBlocks);
  }catch(const char* msg){
    cout << msg << endl << endl;
    throw "ECCEZIONE (replaceChain): La blockchain ricevuta non è valida!";
  }
  BlockChain::blockchain = newBlocks; //Sostituzione blockchain
  cout << "Blockchain sostituita! La Nuova BlockChain ha " << endl << BlockChain::getInstance().getBlockchain().size() << " blocchi..." << endl;
  cout << "Aggiornamento output non spesi dopo l'aggiornamento della Blockchain..." << endl;
  setUnspentTransOuts(lUnspentTransOuts); //Aggiornamento output non spesi
  //Aggiornamento transactionPool in base agli output non spesi aggiornati
  cout << "Aggiornamento della transaction pool dopo l'aggiornamento della Blockchain..." << endl;
  TransactionPool::getInstance().updatePool(getUnspentTransOuts());
  /*Broadcast della nuova blockchain a tutti i nodi (non indichiamo alcuna
   statistica perchè non si tratta di un nuovo blocco per cui si vuole il tempo di mining)
   in questo caso non effettuiamo il lock del mutex perchè questo metodo viene usato solo
   dai thread peer durante l'handling dei messaggi, dunque il lock è già acquisito
  */
  cout << "Broadcast dell'ultimo blocco..." << endl;
  Peer::getInstance().broadcastLatestBlock(-1,0);
  try{
    //Salvataggio dei nuovi dati relativi alla blockchain nell'apposito file
    cout << "Salvataggio statistiche della Blockchain..." << endl;
    saveBlockchainStats();
  }catch(const char* msg){
    cout << msg << endl << endl;
  }
}

/*Salva in un file le statistiche della blockchain (numero di blocchi,
di transazioni e di coin)*/
void BlockChain::saveBlockchainStats() {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  list<Block>::iterator it;
  int transactionNumber = 0;
  vector<UnspentTransOut>::iterator it2;
  float coins = 0;
  //Conteggio numero di transazioni effettuate
  for(it = blockchain.begin(); it != blockchain.end(); ++it) {
    transactionNumber += it->data.size();
  }

  //Conteggio dei coin in circolazione
  for(it2 = unspentTransOuts.begin(); it2 != unspentTransOuts.end(); ++it2){
    coins += it2->amount;
  }

  //Prendo il tempo corrente per abbinarlo ai dati prelevati
  time_t now = time(0);
  tm *ltm = localtime(&now);
  string time = to_string(1 + ltm->tm_hour) + ":" + to_string(1 + ltm->tm_min) + ":" + to_string(1 + ltm->tm_sec) + " " + to_string(ltm->tm_mday) + "/" + to_string(1 + ltm->tm_mon) + "/" + to_string(1900 + ltm->tm_year);

  // salvataggio su file
  ofstream myfile;
  myfile.open ("blockchainstats.txt", ios::out | ios::app);
  if(myfile.is_open()) {
    //Scrittura della nuova entry su file
    myfile << "{\"time\": \"" << time << "\", \"timestamp\": " << (now - blockchain.front().timestamp) << ", \"blocks\": " << blockchain.size() << ", \"transactions\": " << transactionNumber << ", \"coins\": " << coins << "}\n";
  } else {
    cout << "ERRORE (saveBlockchainStats): non è stato possibile aprire il file per salvare il tempo di mining del blocco!";
  }
  myfile.close();
}

/*Elimina i file delle statistiche se esistono */
void BlockChain::initStatFiles() {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  if (FILE* file = fopen("blockchainstats.txt", "r")) {
    fclose(file);
    remove("blockchainstats.txt");
  }
  if (FILE* file = fopen("blocksminingtime.txt", "r")) {
    fclose(file);
    remove("blocksminingtime.txt");
  }
  if (FILE* file = fopen("transactionwaitingtime.txt", "r")) {
    fclose(file);
    remove("transactionwaitingtime.txt");
  }
}
