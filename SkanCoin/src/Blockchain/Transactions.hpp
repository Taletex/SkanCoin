#ifndef __TRANSACTION_DEFINITION__
#define __TRANSACTION_DEFINITION__

#include <vector>
#include <map>
#include <typeinfo>
#include <regex>
#include "picosha2.h"

class UnspentTxOut {
  public:
    std::string txOutId;
    std::string address;
    int txOutIndex;
    float amount;
    UnspentTxOut();
    UnspentTxOut(std::string txOutId, int txOutIndex, std::string address, float amount);
    std::string toString();
    bool isEqual(UnspentTxOut other);
};

class TxIn {
  public:
    std::string txOutId;
    std::string signature;
    int txOutIndex;
    TxIn();
    TxIn(std::string txOutId, std::string signature, int txOutIndex);
    bool isEqual(TxIn other);
    std::string toString();
};

class TxOut {
  public:
    std::string address;
    float amount;
    TxOut();
    TxOut(std::string address, float amount);
    std::string toString();
};

class Transaction {
  public:
    std::string id;
    std::vector<TxIn> txIns;
    std::vector<TxOut> txOuts;
    Transaction();
    Transaction(std::string id, std::vector<TxIn> txIns, std::vector<TxOut> txOuts);
    std::string toString();
};

/*Verifica se l'UnspentTxOut passato come primo parametro è presente nell'array passato come secondo*/
bool isPresentUnspentTxOut(UnspentTxOut find, std::vector<UnspentTxOut> TxOuts);

/*Ritorna l'id (hash) della transazione passata come parametro,
esso verrà firmato da chi invia i coin*/
std::string getTransactionId (Transaction transaction);

/*Controlla se il vettore di input contiene dei duplicati*/
bool hasDuplicates(std::vector<TxIn> txIns);

/*Validazione della struttura (type checking) dell'input di transazione*/
bool isValidTxInStructure(TxIn txIn);

/*Validazione struttura (type checking) dell'output di transazione*/
bool isValidTxOutStructure(TxOut txOut);

/*Validazione della struttura (type checking) della transazione e di tutti i suoi input e output*/
bool isValidTransactionStructure(Transaction transaction);

/*Validazione logica dell'input di transazione:
Controlla se l'input fa riferimento ad un output non speso,
verifica la firma applicata all'input di transazione, essa deve corrispondere alla
chiave pubblica che è la destinazione dell'output non speso a cui questo fa riferimento*/
bool validateTxIn(TxIn txIn, Transaction transaction, std::vector<UnspentTxOut> aUnspentTxOuts);

/*Dato l'input controlla il valore dell'output non speso a cui questo fa
riferimento (la sua esistenza è gia stata verificata in validateTxIn)*/
float getTxInAmount(TxIn txIn, std::vector<UnspentTxOut> aUnspentTxOuts);

/*Validazione della transazione (tipi di dati e procedure di sicurezza)*/
bool validateTransaction(Transaction transaction, std::vector<UnspentTxOut> aUnspentTxOuts);

/*Validazione della transazione di Coinbase*/
bool validateCoinbaseTx(Transaction transaction, int blockIndex);

/*Validazione di tutte le transazioni del blocco*/
bool validateBlockTransactions(std::vector<Transaction> aTransactions, std::vector<UnspentTxOut> aUnspentTxOuts, int blockIndex);

/*Cerca un UnspentTxOut dati i suoi campi*/
UnspentTxOut findUnspentTxOut(std::string outId, int index, std::vector<UnspentTxOut> aUnspentTxOuts);

/*Genera coinbase transaction per il nuovo blocco*/
Transaction getCoinbaseTransaction(std::string address,int blockIndex);

/*Applica la firma digitale ad un input della transazione,
esso preventivamente controllato in termini di validità strutturale e logica*/
std::string signTxIn(Transaction transaction, int txInIndex, std::string privateKey, std::vector<UnspentTxOut> aUnspentTxOuts);

/*Aggiornamento della lista di output non spesi dopo un nuovo blocco di transazioni*/
std::vector<UnspentTxOut> updateUnspentTxOuts(std::vector<Transaction> aTransactions, std::vector<UnspentTxOut> aUnspentTxOuts);

/*Validazione del blocco di transazioni e aggiornamento della lista di output non spesi*/
std::vector <UnspentTxOut> processTransactions(std::vector<Transaction> aTransactions, std::vector<UnspentTxOut> aUnspentTxOuts, int blockIndex);

#endif
