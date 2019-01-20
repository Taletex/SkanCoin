#include <vector>
#include <map>
#include <typeinfo>
#include <regex>
#include "picosha2.h"

#ifndef __TRANSACTION_DEFINITION__
#define __TRANSACTION_DEFINITION__

using namespace std;

const int COINBASE_AMOUNT = 10;

class UnspentTxOut {
  public:
    string txOutId;
    string address;
    int txOutIndex;
    float amount;
    UnspentTxOut();
    UnspentTxOut(string txOutId, int txOutIndex, string address, float amount);
    string toString();
    bool isEqual(UnspentTxOut other);
};

class TxIn {
  public:
    string txOutId;
    string signature;
    int txOutIndex;
    TxIn();
    TxIn(string txOutId, string signature, int txOutIndex);
    bool isEqual(TxIn other);
    string toString();
};

class TxOut {
  public:
    string address;
    float amount;
    TxOut();
    TxOut(string address, float amount);
    string toString();
};

class Transaction {
  public:
    string id;
    vector<TxIn> txIns;
    vector<TxOut> txOuts;
    Transaction();
    Transaction(string id, vector<TxIn> txIns, vector<TxOut> txOuts);
    string toString();
};

//verifica se l'UnspentTxOut passato come primo parametro è presente nell'array passato come secondo
bool isPresentUnspentTxOut(UnspentTxOut find, vector<UnspentTxOut> TxOuts);

//ritorna l'id (hash) della transazione, che verrà firmato da chi invia i coin
string getTransactionId (Transaction transaction);

//controlla se il vettore di input contiene dei duplicati
bool hasDuplicates(vector<TxIn> txIns);

string getPublicKey(string privateKey);

// valid address is a valid ecdsa public key in the 04 + X-coordinate + Y-coordinate format
bool isValidAddress(string address);

bool isValidTxInStructure(TxIn txIn);

//validazione struttura dell'output (type check)
//NOTE: cercare un modo migliore per il type checking
bool isValidTxOutStructure(TxOut txOut);

//validazione della struttura (type check) della transazione e di tutti i suoi input e output
//NOTE: cercare un modo migliore per il type checking
bool isValidTransactionStructure(Transaction transaction);

//Controlla se l'input fa riferimento ad un output non speso
//verifica la firma della nuova transazione che deve corrispondere alla chiave pubblica che è la destinazione di quell'output
bool validateTxIn(TxIn txIn, Transaction transaction, vector<UnspentTxOut> aUnspentTxOuts);

//Dato l'input controlla il valore dell'output non speso a cui questo fa riferimento (la sua esistenza è gia stata verificata in validateTxIn)
float getTxInAmount(TxIn txIn, vector<UnspentTxOut> aUnspentTxOuts);

//Validazione della transazione (tipi di dati e procedure di sicurezza)
bool validateTransaction(Transaction transaction, vector<UnspentTxOut> aUnspentTxOuts);

//Validazione della transazione di Coinbase
bool validateCoinbaseTx(Transaction transaction, int blockIndex);

//validazione di tutte le transazioni del blocco
bool validateBlockTransactions(vector<Transaction> aTransactions, vector<UnspentTxOut> aUnspentTxOuts, int blockIndex);

//Cerca un UnspentTxOut dati i suoi campi
UnspentTxOut findUnspentTxOut(string outId, int index, vector<UnspentTxOut> aUnspentTxOuts);

//genera coinbase transaction per il nuovo blocco
Transaction getCoinbaseTransaction(string address,int blockIndex);

//Applica la firma digitale ad un input della transazione, che viene preventivamente validato
string signTxIn(Transaction transaction, int txInIndex, string privateKey, vector<UnspentTxOut> aUnspentTxOuts);

//Aggiornamento della lista di output non spesi dopo un nuovo blocco di transazioni
vector<UnspentTxOut> updateUnspentTxOuts(vector<Transaction> aTransactions, vector<UnspentTxOut> aUnspentTxOuts);

//validazione del blocco di transazioni e aggiornamento della lista di output non spesi
vector <UnspentTxOut> processTransactions(vector<Transaction> aTransactions, vector<UnspentTxOut> aUnspentTxOuts, int blockIndex);

#endif
