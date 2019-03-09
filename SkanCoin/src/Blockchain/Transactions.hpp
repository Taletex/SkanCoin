#ifndef __TRANSACTION_DEFINITION__
#define __TRANSACTION_DEFINITION__

#include <vector>
#include <map>
#include <typeinfo>
#include <regex>
#include "picosha2.h"
#include "config.hpp"
#include "TransactionComponents.hpp"
#include "Wallet.hpp"

const int COINBASE_AMOUNT = 10;

class Transaction {
  public:
    std::string id;
    std::vector<TransIn> transIns;
    std::vector<TransOut> transOuts;
    Transaction();
    Transaction(std::string id, std::vector<TransIn> transIns, std::vector<TransOut> transOuts);
    std::string toString();
};

/*Validazione di tutte le transazioni del blocco*/
bool areValidBlockTransactions(std::vector<Transaction> transactions, std::vector<UnspentTransOut> unspentTransOuts, int blockIndex);

/*Generazione di una nuova transazione*/
Transaction createTransaction(std::string receiverAddress, float amount, std::string privateKey, std::vector<UnspentTransOut> unspentTransOuts, std::vector<Transaction> pool);

/*Generazione di una nuova transazione, a partire da un array di output, per
permettere di esporre la rest per effettuare più movimenti dallo stesso wallet
in un solo blocco senza passare dal transaction pool*/
Transaction createTransactionWithMultipleOutputs (std::vector<TransOut> transOuts, std::string privateKey, std::vector<UnspentTransOut> unspentTransOuts, std::vector<Transaction> pool);

/*Ritorna la lista di tutti gli output non spesi, senza quelli che sono
referenziati da un qualche input in una delle transazioni presenti nella transaction pool*/
std::vector<UnspentTransOut> filterUnspentTransOuts(std::vector<UnspentTransOut> unspentTransOuts, std::vector<Transaction> transactionPool);

/*Dato l'input controlla il valore dell'output non speso a cui questo fa
riferimento (la sua esistenza è gia stata verificata in isValidTransIn)*/
float getAmountFromInput(TransIn transIn, std::vector<UnspentTransOut> unspentTransOuts);

/*Genera coinbase transaction per il nuovo blocco*/
Transaction getCoinbaseTransaction(std::string address,int blockIndex);

/*Creazione di un input di transazione a partire da un output non speso
a cui questo farà riferimento*/
TransIn getInputFromUnspentTransOut(UnspentTransOut unspentTransOut);

/* Ritorna la firma digitale per un input della transazione,
esso preventivamente viene controllato in termini di validità strutturale e logica*/
std::string getTransInSignature(Transaction transaction, int transInIndex, std::string privateKey, std::vector<UnspentTransOut> unspentTransOuts);

/*Ritorna l'id (hash) della transazione passata come parametro,
esso verrà firmato da chi invia i coin*/
std::string getTransactionId (Transaction transaction);

/*Cerca un UnspentTransOut dati i suoi campi*/
UnspentTransOut getUnspentTransOut(std::string outId, int index, std::vector<UnspentTransOut> unspentTransOuts);

/*Controlla se il vettore di input contiene dei duplicati*/
bool hasDuplicates(std::vector<TransIn> transIns);

/*Verifica se l'UnspentTransOut passato come primo parametro è presente nell'array passato come secondo*/
bool isPresentUnspentTransOut(UnspentTransOut find, std::vector<UnspentTransOut> TransOuts);

/*Validazione della struttura (type checking) dell'input di transazione*/
bool isTransInWellFormed(TransIn transIn);

/*Validazione struttura (type checking) dell'output di transazione*/
bool isTransOutWellFormed(TransOut transOut);

/*Validazione della struttura (type checking) della transazione e di tutti i suoi input e output*/
bool isTransactionWellFormed(Transaction transaction);

/*Validazione della transazione di Coinbase*/
bool isValidCoinbaseTransaction(Transaction transaction, int blockIndex);

/*Validazione della transazione (tipi di dati e procedure di sicurezza)*/
bool isValidTransaction(Transaction transaction, std::vector<UnspentTransOut> unspentTransOuts);

/*Validazione logica dell'input di transazione:
Controlla se l'input fa riferimento ad un output non speso,
verifica la firma applicata all'input di transazione, essa deve corrispondere alla
chiave pubblica che è la destinazione dell'output non speso a cui questo fa riferimento*/
bool isValidTransIn(TransIn transIn, Transaction transaction, std::vector<UnspentTransOut> unspentTransOuts);

/*Validazione del blocco di transazioni e aggiornamento della lista di output non spesi*/
std::vector <UnspentTransOut> processTransactions(std::vector<Transaction> transactions, std::vector<UnspentTransOut> unspentTransOuts, int blockIndex);

/*Aggiornamento della lista di output non spesi dopo un nuovo blocco di transazioni*/
std::vector<UnspentTransOut> updateUnspentTransOuts(std::vector<Transaction> transactions, std::vector<UnspentTransOut> unspentTransOuts);

#endif
