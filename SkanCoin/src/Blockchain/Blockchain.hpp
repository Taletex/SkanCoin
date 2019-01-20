#include <list>
#include <math.h>
#include "Transactions.hpp"
#include "Wallet.hpp"
#include "picosha2.h"
#include "TransactionPool.hpp"

#ifndef __BLOCKCHAIN_DEFINITION__
#define __BLOCKCHAIN_DEFINITION__

using namespace std;

class Block {
  public:
    int index;
    string hash;
    string previousHash;
    int timestamp;
    vector<Transaction> data;
    int difficulty;
    int nonce;

    Block (int index, string hash, string previousHash, int timestamp, vector<Transaction> data, int difficulty, int nonce);

    string toString();

    bool isEqual(Block block);
};

class BlockChain {

  // in seconds
  int BLOCK_GENERATION_INTERVAL= 10;

  // in blocks
  int DIFFICULTY_ADJUSTMENT_INTERVAL = 10;

  public:
    BlockChain();
    list<Block> blockchain;
    vector<UnspentTxOut> unspentTxOuts;
    TransactionPool transactionPool;


    //Tabella di conversione dei caratteri esadecimali in byte
    string hexToBinaryLookup(char c);

    Block getGenesisBlock();

    //Converte una stringa esadecimale in una sequenza binaria
    string hexToBinary(string s);

    //ritorna la blockchain
    list<Block> getBlockchain();

    //ritorna il vettore di output non spesi
    vector<UnspentTxOut> getUnspentTxOuts();

    //Reimposta il vettore di output non spesi
    void setUnspentTxOuts(vector<UnspentTxOut> newUnspentTxOuts);

    //ritorna l'ultimo blocco della blockchain
    Block getLatestBlock();

    //Calcola la nuova difficoltà per i blocchi
    int getAdjustedDifficulty(Block latestBlock, list<Block> aBlockchain);

    //ritorna la difficoltà per il prossimo blocco
    int getDifficulty(list<Block> aBlockchain);

    //Data la lista di transazioni da inserire nel blocco si esegue il mining del blocco e si inserisce nella blockchain
    Block generateRawNextBlock(vector<Transaction> blockData);

    // Ritorna la lista degli output non spesi appartenenti al nodo
    vector<UnspentTxOut> getMyUnspentTransactionOutputs();

    //Colleziona le transazioni dal transaction pool, inizializza la coinbase transaction ed avvia la procedura di mining ed inserimento del blocco
    Block generateNextBlock();

    //Genera un nuovo blocco con una sola transazion (oltre alla coinbase) e lo inserisce nella blockchain
    Block generatenextBlockWithTransaction(string receiverAddress, float amount);

    //calcolo dell'hash del blocco
    string calculateHash(int index, string previousHash, int timestamp, vector<Transaction> data, int difficulty, int nonce);

    Block findBlock(int index, string previousHash, int timestamp, vector<Transaction> data, int difficulty);

    //ritorna il totale degli output non spesi nel wallet del nodo
    float getAccountBalance();

    //Crea una nuova transazione e la inserisce nel transaction pool
    Transaction sendTransaction(string address, float amount);

    //calcolo dell'hash del blocco
    string calculateHashForBlock(Block block);

    //Validazione della struttura del blocco (type checking)
    bool isValidBlockStructure(Block block);

    //Calcolo della complessità del mining del prossimo blocco
    int getAccumulatedDifficulty(vector<Block> aBlockchain);

    //Validazione del timestamp, TODO:rivedere la guida in merito a questo controllo
    bool isValidTimestamp(Block newBlock, Block previousBlock);

    //ricalcola l'hash del blocco e lo confronta con quello proposto (per rilevare modifiche)
    bool hashMatchesBlockContent(Block block);

    //Controlla se l'hash rispetta la difficoltà minima (deve iniziare con un certo numero di zeri)
    bool hashMatchesDifficulty(string hash, int difficulty);

    //Controllo della validità dell'hash e del rispetto della difficoltà minima (proof of work)
    bool hasValidHash(Block block);

    //Validazione della struttura e della correttezza del blocco
    bool isValidNewBlock(Block newBlock, Block previousBlock);

    /* Checks if the given blockchain is valid. Return the unspent txOuts if the chain is valid */
    vector<UnspentTxOut> isValidChain(list<Block> blockchainToValidate);

    /* Add a block to the blockchain */
    bool addBlockToChain(Block newBlock);

    /* Replaces the blockchain with the passed blocks if possible */
    void replaceChain(list<Block> newBlocks);

    /* Handles the received transaction adding it to the transaction pool */
    void handleReceivedTransaction(Transaction transaction);
};
#endif
