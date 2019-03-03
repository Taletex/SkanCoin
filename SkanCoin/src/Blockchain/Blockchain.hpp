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

    /*Ritorna la BlockChain*/
    std::list<Block> getBlockchain();

    /*Ritorna il vettore di output non spesi*/
    std::vector<UnspentTxOut> getUnspentTxOuts();

    /*Ritorna l'ultimo blocco della BlockChain*/
    Block getLatestBlock();

    /*Ritorna un blocco dato il suo hash*/
    Block getBlockFromHash(std::string hash);

    /*Data la lista di transazioni da inserire nel blocco si esegue il mining del
    blocco e si inserisce nella BlockChain*/
    Block generateRawNextBlock(std::vector<Transaction> blockData);

    /*Ritorna la lista degli output non spesi appartenenti al nodo*/
    std::vector<UnspentTxOut> getMyUnspentTransactionOutputs();

    /*Colleziona le transazioni dal transaction pool, inizializza la coinbase
    transaction ed avvia la procedura di mining ed inserimento del blocco*/
    Block generateNextBlock();

    /*Genera un nuovo blocco con una sola transazion (oltre alla coinbase)
    e lo inserisce nella BlockChain*/
    Block generateNextBlockWithTransaction(std::string receiverAddress, float amount);

    /*Ritorna il totale degli output non spesi nel wallet del nodo*/
    float getAccountBalance();

    /*Crea una nuova transazione e la inserisce nel transaction pool*/
    Transaction sendTransaction(std::string address, float amount);

    /*Ritorna una transazione della blockchain dato il suo id*/
    Transaction getTransactionFromId(std::string id);

    /*Validazione della struttura (type checking) del blocco*/
    bool isValidBlockStructure(Block block);

    /*Aggiunta di un blocco alla blockchain*/
    bool addBlockToChain(Block newBlock);

    /*Sostituzione blockchain con i blocchi ricevuti (se questa è
    valida ed ha una difficoltà complessiva maggiore) si ricorda infatti che non
    è valida la chain più lunga ma quella con la difficoltà cumulativa maggiore*/
    void replaceChain(std::list<Block> newBlocks);

    /*Gestione per la ricezione di una nuova transazione, questa deve essere
    aggiunta al transaction pool*/
    void handleReceivedTransaction(Transaction transaction);
  private:
    /*Il pattern singleton viene implementato rendendo il costruttore di default privato
    ed eliminando il costruttore di copia e l'operazione di assegnamento*/
    BlockChain();
    BlockChain(const BlockChain&) = delete;
    BlockChain& operator=(const BlockChain&) = delete;

    /*Tabella di conversione dei caratteri esadecimali in byte,
    usata per verificare la correttezza (difficoltà) degli hash*/
    std::string hexToBinaryLookup(char c);

    /*Generazione blocco di genesi (il primo blocco della blockchain)*/
    Block getGenesisBlock();

    /*Converte una stringa esadecimale in una sequenza binaria, in modo da poter
     verificare il numero di zeri iniziali (difficoltà) durante la validazione del blocco*/
    std::string hexToBinary(std::string s);

    /*Reimposta il vettore di output non spesi*/
    void setUnspentTxOuts(std::vector<UnspentTxOut> newUnspentTxOuts);

    /*Calcola la nuova difficoltà per i blocchi*/
    int getAdjustedDifficulty(Block latestBlock, std::list<Block> aBlockchain);

    /*Ritorna la difficoltà per il prossimo blocco*/
    int getDifficulty(std::list<Block> aBlockchain);

    /*Calcolo dell'hash per un blocco*/
    std::string calculateHash(int index, std::string previousHash, time_t timestamp, std::vector<Transaction> data, int difficulty, int nonce);

    /*Questo metodo effettua il mining di un nuovo blocco, viengono generati
    (e scartati) nuovi blocchi finche l'hash ottenuto non rispetta la difficoltà richiesta*/
    Block findBlock(int index, std::string previousHash, time_t timestamp, std::vector<Transaction> data, int difficulty);

    /*Calcolo della complessità del mining del prossimo blocco (numero di
    zeri iniziali necessari nell'hash) della blockchain data*/
    int getAccumulatedDifficulty(std::vector<Block> aBlockchain);

    /*Validazione del timestamp, per evitare che venga introdotto un timestamp falso in modo da
    rendere valido un blocco con difficoltà inferiore a quella attuale vengono accettati solo i blocchi per
    cui il mining è iniziato al massimo un minuto prima del mining dell'ultimo blocco ed al
    massimo un minuto prima del tempo percepito dal nodo che effettua la validazione*/
    bool isValidTimestamp(Block newBlock, Block previousBlock);

    /*Ricalcola l'hash del blocco e lo confronta con quello proposto (per rilevare modifiche)*/
    bool hashMatchesBlockContent(Block block);

    /*Controlla se l'hash rispetta la difficoltà minima (deve iniziare con un certo numero di zeri)*/
    bool hashMatchesDifficulty(std::string hash, int difficulty);

    /*Controllo della validità dell'hash e del rispetto della difficoltà minima (proof of work)*/
    bool hasValidHash(Block block);

    /*Validazione della struttura e della correttezza logica del blocco*/
    bool isValidNewBlock(Block newBlock, Block previousBlock);

    /*Verifica la validità della blockchain ricevutaa, ritorna la lista aggiornata degli
    output non spesi se questa è valida*/
    std::vector<UnspentTxOut> isValidChain(std::list<Block> blockchainToValidate);

    /*Salva in un file le statistiche della blockchain (numero di blocchi,
    di transazioni e di coin)*/
    void saveBlockchainStats();
};
#endif
