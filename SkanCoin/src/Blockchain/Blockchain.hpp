#ifndef __BLOCKCHAIN_DEFINITION__
#define __BLOCKCHAIN_DEFINITION__

#include <list>
#include <math.h>
#include <ctime>
#include "Transactions.hpp"
#include "Wallet.hpp"
#include "picosha2.h"
#include "TransactionPool.hpp"

class Block {
  public:
    int index;
    std::string hash;
    std::string previousHash;
    long timestamp;
    std::vector<Transaction> data;
    int difficulty;
    int nonce;

    Block(){}

    Block (int index, std::string hash, std::string previousHash, long timestamp, std::vector<Transaction> data, int difficulty, int nonce);

    std::string toString();

    bool isEqual(Block block);
};

class BlockChain {
  int BLOCK_GENERATION_INTERVAL= 10;

  int DIFFICULTY_ADJUSTMENT_INTERVAL = 10;

  public:
    static BlockChain& getInstance() {
        static BlockChain bc;
        return bc;
    }
    std::list<Block> blockchain;
    std::vector<UnspentTxOut> unspentTxOuts;

    std::string toString();

    //Tabella di conversione dei caratteri esadecimali in byte
    std::string hexToBinaryLookup(char c);

    Block getGenesisBlock();

    //Converte una stringa esadecimale in una sequenza binaria
    std::string hexToBinary(std::string s);

    //ritorna la blockchain
    std::list<Block> getBlockchain();

    //ritorna il vettore di output non spesi
    std::vector<UnspentTxOut> getUnspentTxOuts();

    //Reimposta il vettore di output non spesi
    void setUnspentTxOuts(std::vector<UnspentTxOut> newUnspentTxOuts);

    //ritorna l'ultimo blocco della blockchain
    Block getLatestBlock();

    //Calcola la nuova difficoltà per i blocchi
    int getAdjustedDifficulty(Block latestBlock, std::list<Block> aBlockchain);

    //ritorna la difficoltà per il prossimo blocco
    int getDifficulty(std::list<Block> aBlockchain);

    //Data la lista di transazioni da inserire nel blocco si esegue il mining del blocco e si inserisce nella blockchain
    Block generateRawNextBlock(std::vector<Transaction> blockData);

    // Ritorna la lista degli output non spesi appartenenti al nodo
    std::vector<UnspentTxOut> getMyUnspentTransactionOutputs();

    //Colleziona le transazioni dal transaction pool, inizializza la coinbase transaction ed avvia la procedura di mining ed inserimento del blocco
    Block generateNextBlock();

    //Genera un nuovo blocco con una sola transazion (oltre alla coinbase) e lo inserisce nella blockchain
    Block generatenextBlockWithTransaction(std::string receiverAddress, float amount);

    //calcolo dell'hash del blocco
    std::string calculateHash(int index, std::string previousHash, time_t timestamp, std::vector<Transaction> data, int difficulty, int nonce);

    Block findBlock(int index, std::string previousHash, time_t timestamp, std::vector<Transaction> data, int difficulty);

    //ritorna il totale degli output non spesi nel wallet del nodo
    float getAccountBalance();

    //Crea una nuova transazione e la inserisce nel transaction pool
    Transaction sendTransaction(std::string address, float amount);

    //calcolo dell'hash del blocco
    std::string calculateHashForBlock(Block block);

    //Validazione della struttura del blocco (type checking)
    bool isValidBlockStructure(Block block);

    //Calcolo della complessità del mining del prossimo blocco
    int getAccumulatedDifficulty(std::vector<Block> aBlockchain);

    //Validazione del timestamp
    bool isValidTimestamp(Block newBlock, Block previousBlock);

    //ricalcola l'hash del blocco e lo confronta con quello proposto (per rilevare modifiche)
    bool hashMatchesBlockContent(Block block);

    //Controlla se l'hash rispetta la difficoltà minima (deve iniziare con un certo numero di zeri)
    bool hashMatchesDifficulty(std::string hash, int difficulty);

    //Controllo della validità dell'hash e del rispetto della difficoltà minima (proof of work)
    bool hasValidHash(Block block);

    //Validazione della struttura e della correttezza del blocco
    bool isValidNewBlock(Block newBlock, Block previousBlock);

    /* Checks if the given blockchain is valid. Return the unspent txOuts if the chain is valid */
    std::vector<UnspentTxOut> isValidChain(std::list<Block> blockchainToValidate);

    /* Add a block to the blockchain */
    bool addBlockToChain(Block newBlock);

    /* Replaces the blockchain with the passed blocks if possible */
    void replaceChain(std::list<Block> newBlocks);

    /* Salva in un file le statistiche della blockchain (numero di blocchi, di transazioni e di coin) */
    void saveBlockchainStats();

    /* Handles the received transaction adding it to the transaction pool */
    void handleReceivedTransaction(Transaction transaction);
    private:
      BlockChain();
      BlockChain(const BlockChain&) = delete;
      BlockChain& operator=(const BlockChain&) = delete;
};
#endif
