#include "Blockchain.hpp"

using namespace std;

//BLOCK FUNCTIONS//
Block::Block (int index, string hash, string previousHash, long timestamp, vector<Transaction> data, int difficulty, int nonce) {
  this->index = index;
  this->previousHash = previousHash;
  this->timestamp = timestamp;
  this->data.swap(data);
  this->hash = hash;
  this->difficulty = difficulty;
  this->nonce = nonce;
}

string Block::toString(){
  string ret = "{\"index\": " + to_string(index) + ", \"previousHash\": \"" + previousHash  + "\", \"timestamp\": " + to_string(timestamp) + ", \"hash\": \"" + hash + "\", \"difficulty\": " + to_string(difficulty)  + ", \"nonce\": " + to_string(nonce)   + ", \"data\": [";
  vector<Transaction>::iterator it;
  for(it = data.begin(); it != data.end(); ++it){
    if(it != data.begin()){
      ret = ret + ", ";
    }
    ret = ret + it->toString();
  }
  ret = ret + "]}";
  return ret;
}

bool Block::isEqual(Block other){
  return this->toString() == other.toString();
}

//BLOCKCHAIN FUNCTIONS//
BlockChain::BlockChain(){
  Block genesisBlock = getGenesisBlock();
  blockchain = {genesisBlock};
  vector<UnspentTxOut> unspentTxOuts = {};
  try{
    unspentTxOuts = processTransactions(blockchain.front().data, unspentTxOuts, 0);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION: Blockchain intialization failed";
  }
}

//generazione blocco di genesi
Block BlockChain::getGenesisBlock(){
  vector<TxIn> txInsVector = {TxIn("","",0)};
  vector<TxOut> txOutsVector = {TxOut(getPublicFromWallet(), COINBASE_AMOUNT)};
  Transaction genesisTransaction("", txInsVector, txOutsVector);
  genesisTransaction.id = getTransactionId(genesisTransaction);
  vector<Transaction> transactionsVector = {genesisTransaction};
  Block ret(0, "", "", time(nullptr), transactionsVector, 0, 0);
  ret.hash = calculateHash(ret.index, ret.previousHash, ret.timestamp , ret.data, ret.difficulty, ret.nonce);
  return ret;
}

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

//Tabella di conversione dei caratteri esadecimali in byte
string BlockChain::hexToBinaryLookup(char c){
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

//Converte una stringa esadecimale in una sequenza binaria
string BlockChain::hexToBinary(string s){
    string ret = "";
    for(char& c : s) {
      ret = ret + hexToBinaryLookup(c);
    }
    return ret;
}

//ritorna la BlockChain::blockchain
list<Block> BlockChain::getBlockchain(){
  return blockchain;
}

//ritorna il vettore di output non spesi
vector<UnspentTxOut> BlockChain::getUnspentTxOuts(){
  return unspentTxOuts;
}

//Reimposta il vettore di output non spesi
void BlockChain::setUnspentTxOuts(vector<UnspentTxOut> newUnspentTxOuts){
  vector<UnspentTxOut>::iterator it;
  for(it = newUnspentTxOuts.begin(); it != newUnspentTxOuts.end(); ++it){
    cout << it->toString();
  }
  unspentTxOuts.swap(newUnspentTxOuts);
}

//ritorna l'ultimo blocco della BlockChain::blockchain
Block BlockChain::getLatestBlock(){
  return blockchain.back();
}

//Calcola la nuova difficoltà per i blocchi
int BlockChain::getAdjustedDifficulty(Block latestBlock, list<Block> aBlockchain){
  list<Block>::iterator it = aBlockchain.begin();
  //Avanza l'iteratore fino al blocco in cui è cambiata l'ultima volta la difficoltà
  advance(it, (blockchain.size() - DIFFICULTY_ADJUSTMENT_INTERVAL));
  Block prevAdjustmentBlock = *it;
  long timeExpected = long(BLOCK_GENERATION_INTERVAL * DIFFICULTY_ADJUSTMENT_INTERVAL); //tempo atteso tra un aumento di diffoltà e l'altro
  long timeTaken = latestBlock.timestamp - prevAdjustmentBlock.timestamp; //tempo trascorso dall'ultimo aumento di difficoltà

  //Si cerca sempre di fare in modo che i blocchi vengano aggiunti regolarmente con un intervallo pari a timeExpected
  if (timeTaken < timeExpected / 2) {
      return prevAdjustmentBlock.difficulty + 1;
  } else if (timeTaken > timeExpected * 2) {
      return prevAdjustmentBlock.difficulty - 1;
  } else {
      return prevAdjustmentBlock.difficulty;
  }
}

//ritorna la difficoltà per il prossimo blocco
int BlockChain::getDifficulty(list<Block> aBlockchain){
    Block latestBlock = aBlockchain.back();
    if ((latestBlock.index % DIFFICULTY_ADJUSTMENT_INTERVAL) == 0 && latestBlock.index != 0) {
        return getAdjustedDifficulty(latestBlock, aBlockchain);
    } else {
        return latestBlock.difficulty;
    }
}

//Data la lista di transazioni da inserire nel blocco si esegue il mining del blocco e si inserisce nella BlockChain::blockchain
Block BlockChain::generateRawNextBlock(vector<Transaction> blockData){
  Block previousBlock = getLatestBlock();
  int difficulty = getDifficulty(getBlockchain());
  int nextIndex = previousBlock.index + 1;
  long nextTimestamp =  time(nullptr);
  Block newBlock;
  try{
    newBlock = findBlock(nextIndex, previousBlock.hash, nextTimestamp, blockData, difficulty);
  }catch(const char* msg){
    cout << endl;
    throw "EXCEPTION: error occurred during new block mining";
  }
  if (addBlockToChain(newBlock)) {
    string data = "";
    try{
      ifstream inFile;
      inFile.open("blocksminingtime.txt");
      string line;
      if(inFile) {
        while (getline(inFile, line)) {
          data = line;
        }
      } else {
        throw "Errore: non è stato possibile aprire il file per salvare il tempo di mining del blocco!";
      }
    }catch(const char* msg){
      cout << msg << endl << endl;
      cout << "ERROR: Could not open file containing mining time!";
    }
      Peer::getInstance().broadcastLatest(data);
      return newBlock;
  } else {
      cout << endl;
      throw "EXCEPTION: Error while inserting new Block in the BlockChain!";
  }
  return newBlock;
}

// Ritorna la lista degli output non spesi appartenenti al nodo
vector<UnspentTxOut> BlockChain::getMyUnspentTransactionOutputs(){
    return findUnspentTxOutsOfAddress(getPublicFromWallet(), getUnspentTxOuts());
}

//Colleziona le transazioni dal transaction pool, inizializza la coinbase transaction ed avvia la procedura di mining ed inserimento del blocco
Block BlockChain::generateNextBlock(){
    Transaction coinbaseTx = getCoinbaseTransaction(getPublicFromWallet(), getLatestBlock().index + 1);
    vector<Transaction> blockData = TransactionPool::getInstance().getTransactionPool();
    blockData.insert (blockData.begin() , coinbaseTx);
    try{
      Block ret = generateRawNextBlock(blockData);
      Peer::getInstance().broadcastTxPoolStat(TransactionPool::getInstance().getStatStrings());
      return ret;
    }catch(const char* msg){
      cout << msg << endl;
      cout << endl;
      throw "EXCEPTION: Error while generating the block";
    }
}

//Genera un nuovo blocco con una sola transazion (oltre alla coinbase) e lo inserisce nella BlockChain::blockchain
Block BlockChain::generatenextBlockWithTransaction(string receiverAddress, float amount){
    if (!isValidAddress(receiverAddress)) {
        cout << endl;
        throw "EXCEPTION: Unvalid Address!";
    }
    if (typeid(amount) != typeid(float)) {
        cout << endl;
        throw "EXCEPTION: Unvalid amount!";
    }
    Transaction coinbaseTx = getCoinbaseTransaction(getPublicFromWallet(), getLatestBlock().index + 1);
    Transaction tx;
    try{
      tx = createTransaction(receiverAddress, amount, getPrivateFromWallet(), getUnspentTxOuts(), TransactionPool::getInstance().getTransactionPool());
        cout << coinbaseTx.toString();
    }catch(const char* msg){
      cout << msg << endl;
      cout << endl;
      throw "EXCEPTION: Creation of the transaction failed";
    }
    vector<Transaction> blockData = {coinbaseTx, tx};
    try{
      return generateRawNextBlock(blockData);
    }catch(const char* msg){
      cout << msg << endl;
      cout << endl;
      throw "EXCEPTION: Error while generating next block";
    }
}

// Mining del blocco
Block BlockChain::findBlock(int index, string previousHash, long timestamp, vector<Transaction> data, int difficulty) {
  int nonce = 0;
  string hash;
  double duration;
  clock_t start = clock();

  // calcolo dell'hash (mining)
  while (true) {
    hash = calculateHash(index, previousHash, timestamp, data, difficulty, nonce);
    if(hashMatchesDifficulty(hash, difficulty)) {
      // calcolo e salvataggio del tempo di mining del blocco in secondi (per le stat)
      duration = (std::clock() - start)/(double)CLOCKS_PER_SEC;
      ofstream myfile;
      myfile.open ("blocksminingtime.txt", ios::out | ios::app);
      if(myfile) {
        myfile << "{\"block\": " <<  index << ", \"miningtime\": " << duration << "}";
      } else {
        throw "Errore: non è stato possibile aprire il file per salvare il tempo di mining del blocco!";
      }
      myfile.close();
      return Block(index, hash, previousHash, timestamp, data, difficulty, nonce);
    }
    nonce++;
  }
}

//ritorna il totale degli output non spesi nel wallet del nodo
float BlockChain::getAccountBalance(){
    return getBalance(getPublicFromWallet(), getUnspentTxOuts());
}

//Crea una nuova transazione e la inserisce nel transaction pool
Transaction BlockChain::sendTransaction(string address, float amount){
  Transaction tx;
  try{
    tx = createTransaction(address, amount, getPrivateFromWallet(), getUnspentTxOuts(), TransactionPool::getInstance().getTransactionPool());
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION: Creation of the transaction to be sent failed";
  }
  try{
    TransactionPool::getInstance().addToTransactionPool(tx, getUnspentTxOuts());
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION: Creation of the transaction to be sent failed";
  }
  Peer::getInstance().broadCastTransactionPool();
  return tx;
}

//calcolo dell'hash del blocco
string BlockChain::calculateHashForBlock(Block block){
  return calculateHash(block.index, block.previousHash, block.timestamp, block.data, block.difficulty, block.nonce);
}

//calcolo dell'hash del blocco
string BlockChain::calculateHash (int index, string previousHash, long timestamp, vector<Transaction> data, int difficulty, int nonce){
  string ret = to_string(index) + previousHash + to_string(timestamp);
  vector<Transaction>::iterator it;
  for(it = data.begin(); it != data.end(); ++it){
    ret = ret + it->toString();
  }
  ret = ret + to_string(difficulty) + to_string(nonce);
  return picosha2::hash256_hex_string(ret);
}

//Validazione della struttura del blocco (type checking)
bool BlockChain::isValidBlockStructure(Block block){
  return (
    typeid(block.index) != typeid(int) &&
    typeid(block.hash) != typeid(string) &&
    typeid(block.previousHash) != typeid(string) &&
    typeid(block.timestamp) != typeid(int) &&
    typeid(block.data) != typeid(vector<Transaction>) );
}

//Calcolo della complessità del mining del prossimo blocco
int BlockChain::getAccumulatedDifficulty(vector<Block> aBlockchain){
  int res = 0;
  vector<Block>::iterator it;
  for(it = aBlockchain.begin(); it != aBlockchain.end(); ++it){
    res = res + pow(2, it->difficulty);
  }
  return res;
}

//Validazione del timestamp, per evitare che venga introdotto un timestamp falso in modo da
//rendere valido un blocco con difficoltà inferiore a quella attuale vengono accettati solo i blocchi per
//cui il mining è iniziato al massimo un minuto prima del mining dell'ultimo blocco ed al
//massimo un minuto prima del tempo percepito dal nodo che effettua la validazione
bool BlockChain::isValidTimestamp(Block newBlock, Block previousBlock){
  return (( previousBlock.timestamp - 60 < newBlock.timestamp ) && ( newBlock.timestamp - 60 < time(nullptr) ) );
}

//ricalcola l'hash del blocco e lo confronta con quello proposto (per rilevare modifiche)
bool BlockChain::hashMatchesBlockContent(Block block){
    string blockHash = calculateHashForBlock(block);
    return blockHash == block.hash;
}

//Controlla se l'hash rispetta la difficoltà minima (deve iniziare con un certo numero di zeri)
bool BlockChain::hashMatchesDifficulty(string hash, int difficulty){
    string hashInBinary = hexToBinary(hash);
    int n = hashInBinary.length();
    // declaring character array
    char charArray[n+1];
    // copying the contents of the string to char array
    strcpy(charArray, hashInBinary.c_str());

    for(int i = 0; i < difficulty; i++){
      if(charArray[i] != '0'){
        return false;
      }
    }
    return true;
}

//Controllo della validità dell'hash e del rispetto della difficoltà minima (proof of work)
bool BlockChain::hasValidHash(Block block){
    if (!hashMatchesBlockContent(block)) {
        cout << "invalid hash, got:" << endl << block.hash << endl;
        return false;
    }
    if (!hashMatchesDifficulty(block.hash, block.difficulty)) {
        cout << "block difficulty not satisfied. Expected: " << block.difficulty << ", got: " << block.hash << endl;
    }
    return true;
}

//Validazione della struttura e della correttezza del blocco
bool BlockChain::isValidNewBlock(Block newBlock, Block previousBlock){
    if (!isValidBlockStructure(newBlock)) {
        cout << "invalid block structure: " <<  newBlock.toString();
        return false;
    }
    if (previousBlock.index + 1 != newBlock.index) {
        cout << "invalid index";
        return false;
    } else if (previousBlock.hash != newBlock.previousHash) {
        cout << "invalid previoushash";
        return false;
    } else if (!isValidTimestamp(newBlock, previousBlock)) {
        cout << "invalid timestamp";
        return false;
    } else if (!hasValidHash(newBlock)) {
        return false;
    }
    return true;
}

/* Checks if the given BlockChain::blockchain is valid. Return the unspent txOuts if the chain is valid */
vector<UnspentTxOut> BlockChain::isValidChain(list<Block> blockchainToValidate) {
  vector<UnspentTxOut> aUnspentTxOuts;
  list<Block>::iterator it1;

  /* Validate the genesis block */
  if(blockchainToValidate.front().isEqual(getGenesisBlock())) {
    cout << endl;
    throw "EXCEPTION: genesis block is invalid!";
  }

  /* Validate each block in the chain. The block is valid if the block structure is valid and the transaction are valid */
  for(it1 = blockchainToValidate.begin(); it1 != blockchainToValidate.end(); ++it1) {
    if(it1 != blockchainToValidate.begin() && !isValidNewBlock(*it1, *(--it1))) {
      cout << endl;
      throw "EXCEPTION: some block is invalid!";
    }
    it1++;
    try{
      aUnspentTxOuts = processTransactions(it1->data, aUnspentTxOuts, it1->index);
    }catch(const char* msg){
      cout << msg << endl;
      cout << endl;
      throw "EXCEPTION: Invalid transactions in this blockchain!";
    }
  }
  return aUnspentTxOuts;
}

/* Add a block to the blockchain */
bool BlockChain::addBlockToChain(Block newBlock) {
  if(isValidNewBlock(newBlock, getLatestBlock())) {
    vector<UnspentTxOut> ret;
    try{
      ret = processTransactions(newBlock.data, getUnspentTxOuts(), newBlock.index);
    }catch(const char* msg){
      cout << msg << endl;
      return false;
    }
    BlockChain::blockchain.push_back(newBlock);
    setUnspentTxOuts(ret);
    TransactionPool::getInstance().updateTransactionPool(getUnspentTxOuts());
    try{
        saveBlockchainStats();
    }catch(const char* msg){
      cout << msg << endl;
      cout << endl;
    }

    return true;
  }else{
    return false;
  }
}

/* Replaces the BlockChain::blockchain with the passed blocks if possible */
void BlockChain::replaceChain(list<Block> newBlocks) {
  vector<UnspentTxOut> aUnspentTxOuts;
  try{
    aUnspentTxOuts = isValidChain(newBlocks);
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION: Received blockchain is invalid!";
  }
  if(getAccumulatedDifficulty({ begin(newBlocks), end(newBlocks) }) > getAccumulatedDifficulty({ begin(newBlocks), end(newBlocks) })) {
    cout << "Received blockchain is valid: replacing current blockchain with the received one!" << endl;
    BlockChain::blockchain = newBlocks;
    setUnspentTxOuts(aUnspentTxOuts);
    TransactionPool::getInstance().updateTransactionPool(getUnspentTxOuts());
    Peer::getInstance().broadcastLatest("");
    try{
        saveBlockchainStats();
    }catch(const char* msg){
      cout << msg << endl;
      cout << endl;
    }
  }
}

/* Handles the received transaction adding it to the transaction pool */
void BlockChain::handleReceivedTransaction(Transaction transaction) {
  try{
    TransactionPool::getInstance().addToTransactionPool(transaction, getUnspentTxOuts());
  }catch(const char* msg){
    cout << msg << endl;
    cout << endl;
    throw "EXCEPTION: Received invalid transaction!";
    }
}

/* Salva in un file le statistiche della blockchain (numero di blocchi, di transazioni e di coin) */
void BlockChain::saveBlockchainStats() {
  list<Block>::iterator it;
  int transactionNumber = 0;
  vector<UnspentTxOut>::iterator it2;
  float coins = 0;
  // prendo il numero di transazioni
  for(it = blockchain.begin(); it != blockchain.end(); ++it) {
    transactionNumber += it->data.size();
  }

  for(it2 = unspentTxOuts.begin(); it2 != unspentTxOuts.end(); ++it2){
    coins += it2->amount;
  }

  // prendo il tempo corrente (relativo al sistema corrente)
  time_t now = time(0);
  tm *ltm = localtime(&now);
  string time = to_string(1 + ltm->tm_hour) + ":" + to_string(1 + ltm->tm_min) + ":" + to_string(1 + ltm->tm_sec) + " " + to_string(ltm->tm_mday) + "/" + to_string(1 + ltm->tm_mon) + "/" + to_string(1900 + ltm->tm_year);

  // salvataggio su file
  ofstream myfile;
  myfile.open ("blockchainstats.txt", ios::out | ios::app);
  if(myfile) {
    myfile << "{\"time\": " << time << ", \"blocks\": " << blockchain.size() << ", \"transactions\": " << transactionNumber << ", \"coins\": " << coins << "}";
  } else {
    throw "Errore: non è stato possibile aprire il file per salvare il tempo di mining del blocco!";
  }
  myfile.close();
}
