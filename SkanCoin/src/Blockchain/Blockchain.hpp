#ifndef __BLOCKCHAIN_DEFINITION__
#define __BLOCKCHAIN_DEFINITION__

#include <list>
#include <ctime>
#include <math.h>
#include "picosha2.h"
#include "config.hpp"
#include "Block.hpp"
#include "Transactions.hpp"
#include "TransactionComponents.hpp"
#include "TransactionPool.hpp"
#include "Wallet.hpp"
#include "../P2P/Peer.hpp"

class BlockChain {
    int BLOCK_CREATION_INTERVAL = 10;
    int DIFFICULTY_ADJUSTMENT_INTERVAL = 10;

  public:
    /*Metodo getInstance per l'implementazione del pattern Singleton*/
    static BlockChain& getInstance() {
        static BlockChain bc;
        return bc;
    }
    std::list<Block> blockchain;
    std::vector<UnspentTransOut> unspentTransOuts;

    /*Aggiunta di un blocco alla blockchain*/
    bool addBlockToBlockchain(Block newBlock);

    /*Colleziona le transazioni dal transaction pool, inizializza la coinbase
    transaction ed avvia la procedura di mining ed inserimento del blocco*/
    Block createNextBlock();

    /*Genera un nuovo blocco con la coinbase e una transazione avente transOuts creati a
    partire da un vettore di coppie [indirizzo destinazione, amount] e lo inserisce
    nella BlockChain (transazion con multipli output) */
    Block createNextBlockWithTransaction(std::vector<TransOut> transOuts);

    /*Data la lista di transazioni da inserire nel blocco si esegue il mining del
    blocco e si inserisce nella BlockChain*/
    Block createRawNextBlock(std::vector<Transaction> blockData);

    /*Ritorna il totale degli output non spesi nel wallet del nodo*/
    float getAccountBalance();

    /*Ritorna la BlockChain*/
    std::list<Block> getBlockchain();

    /*Ritorna un blocco dato il suo hash*/
    Block getBlockFromHash(std::string hash);

    /*Ritorna l'ultimo blocco della BlockChain*/
    Block getLatestBlock();

    /*Ritorna una transazione della blockchain dato il suo id*/
    Transaction getTransactionFromId(std::string id);

    /*Ritorna la lista degli output non spesi appartenenti al nodo*/
    std::vector<UnspentTransOut> getMyUnspentTransactionOutputs();

    /*Ritorna il vettore di output non spesi*/
    std::vector<UnspentTransOut> getUnspentTransOuts();

    /*Validazione della struttura (type checking) del blocco*/
    bool isWellFormedBlock(Block block);

    /*Sostituzione blockchain con i blocchi ricevuti (se questa è
    valida ed ha una difficoltà complessiva maggiore) si ricorda infatti che non
    è valida la chain più lunga ma quella con la difficoltà cumulativa maggiore*/
    void replaceChain(std::list<Block> newBlocks);

    /*Crea una nuova transazione e la inserisce nel transaction pool*/
    Transaction sendTransaction(std::string address, float amount);

    std::string toString();

  private:
    /*Il pattern singleton viene implementato rendendo il costruttore di default privato
    ed eliminando il costruttore di copia e l'operazione di assegnamento*/
    BlockChain();
    BlockChain(const BlockChain&) = delete;
    BlockChain& operator=(const BlockChain&) = delete;

    /*Ricalcola l'hash del blocco e lo confronta con quello proposto (per rilevare modifiche)*/
    bool blockHashCheck(Block block);

    /*Calcola la nuova difficoltà per i blocchi*/
    int computeNewDifficulty(Block latestBlock, std::list<Block> lBlockchain);

    /*Calcolo dell'hash per un blocco*/
    std::string calculateHash(int index, std::string previousHash, time_t timestamp, std::vector<Transaction> data, int difficulty, int nonce);

    /*Calcolo della complessità del mining del prossimo blocco (numero di
    zeri iniziali necessari nell'hash) della blockchain data*/
    int getAccumulatedDifficulty(std::vector<Block> lBlockchain);

    /*Ritorna la difficoltà per il prossimo blocco*/
    int getDifficulty(std::list<Block> lBlockchain);

    /*Generazione blocco di genesi (il primo blocco della blockchain)*/
    Block getGenesisBlock();

    /*Controlla se l'hash rispetta la difficoltà minima (deve iniziare con un certo numero di zeri)*/
    bool hashDifficultyCheck(std::string hash, int difficulty);

    /*Controllo della validità dell'hash e del rispetto della difficoltà minima (proof of work)*/
    bool hasValidHash(Block block);

    /*Converte una stringa esadecimale in una sequenza binaria, in modo da poter
    verificare il numero di zeri iniziali (difficoltà) durante la validazione del blocco*/
    std::string hexToBinary(std::string s);

    /*Tabella di conversione dei caratteri esadecimali in byte,
    usata per verificare la correttezza (difficoltà) degli hash*/
    std::string hexToBinaryLookup(char c);

    /*Elimina i file delle statistiche se esistono */
    void initStatFiles();

    /*Validazione della struttura e della correttezza logica del blocco*/
    bool isBlockValid(Block newBlock, Block previousBlock);

    /*Verifica la validità della blockchain ricevutaa, ritorna la lista aggiornata degli
    output non spesi se questa è valida*/
    std::vector<UnspentTransOut> isBlockchainValid(std::list<Block> blockchainToValidate);

    /*Validazione del timestamp, per evitare che venga introdotto un timestamp falso in modo da
    rendere valido un blocco con difficoltà inferiore a quella attuale vengono accettati solo i blocchi per
    cui il mining è iniziato al massimo un minuto prima del mining dell'ultimo blocco ed al
    massimo un minuto prima del tempo percepito dal nodo che effettua la validazione*/
    bool isValidTimestamp(Block newBlock, Block previousBlock);

    /*Questo metodo effettua il mining di un nuovo blocco, viengono generati
    (e scartati) nuovi blocchi finche l'hash ottenuto non rispetta la difficoltà richiesta*/
    Block mineBlock(int index, std::string previousHash, time_t timestamp, std::vector<Transaction> data, int difficulty, double *time);

    /*Salva in un file le statistiche della blockchain (numero di blocchi,
    di transazioni e di coin)*/
    void saveBlockchainStats();

    /*Reimposta il vettore di output non spesi*/
    void setUnspentTransOuts(std::vector<UnspentTransOut> newUnspentTransOuts);
};

#endif


/* Definizione della macro BOOST_SYSTEM_NO_DEPRECATED per risolvere il bug
   boost::system::throws della libreria Boost
   http://boost.2283326.n4.nabble.com/Re-Boost-build-System-Link-issues-using-BOOST-ERROR-CODE-HEADER-ONLY-td4688963.html
   https://www.boost.org/doc/libs/1_56_0/libs/system/doc/reference.html#Header-
*/
#ifndef BOOST_SYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED
#endif
