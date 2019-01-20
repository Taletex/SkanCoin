#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>

#ifndef __WALLET_DEFINITION__
#define __WALLET_DEFINITION__

using namespace std;

//path del file contenente la chiave privata del nodo
const string privateKeyLocation = "../NodeWallet/private_key";

//Legge la chiave privata (wallet) del nodo dal file
string getPrivateFromWallet();

//Legge la chiave privata del nodo dal file, da essa genera la chiave pubblica e la ritorna
string getPublicFromWallet();

//Generazione di una nuova chiave privata
string generatePrivateKey();

//inizializzazione del wallet (chiave privata), se il file non esiste si genera una nuova chiave e si salva su file
void initWallet();

//delete the file containing the wallet if it exists
void deleteWallet();

//ritorna la lista degli output non spesi appartenenti ad un certo indirizzo (wallet)
vector<UnspentTxOut> findUnspentTxOutsOfAddress(string ownerAddress, vector<UnspentTxOut> unspentTxOuts);

//calcola il totale degli output non spesi appartenenti ad un certo indirizzo
float getBalance(string address, vector<UnspentTxOut> unspentTxOuts);

//calcola il totale data una lista di output non spesi
float getTotalFromOutputVector(vector<UnspentTxOut> unspentTxOuts);

//Trova gli output non spesi necessari ad eseguire un pagamento dato il totale e la lista degli output non spesi dell'utente che sta effettuando il pagamento
//Viene ritornata la lista degli output non spesi che saranno utilizzati, inoltre si assegna il valore di un puntatore contenente il valore superfluo che dovra essere indirizzato al mittente stesso
vector<UnspentTxOut> findTxOutsForAmount(float amount, vector<UnspentTxOut> myUnspentTxOuts, float *leftOverAmount);

//Generazione degli output di transazione dati l'amount e la differenza che deve tornare al mittente
vector<TxOut> createTxOuts(string receiverAddress, string myAddress, float amount, float leftOverAmount);

//Verifica se l'output non speso (1° parametro) è referenziato da un input nella lista txIns (2° parametro)
bool isReferencedUnspentTxOut(UnspentTxOut uTxOut, vector<TxIn> txIns);

//Ritorna la lista di tutti gli output non spesi, senza quelli che sono referenziati da un qualche input in una delle transazioni presenti nella transaction pool
vector<UnspentTxOut> filterTxPoolTxs(vector<UnspentTxOut> unspentTxOuts, vector<Transaction> transactionPool);

TxIn toUnsignedTxIn(UnspentTxOut unspentTxOut);

Transaction createTransaction(string receiverAddress, float amount, string privateKey, vector<UnspentTxOut> unspentTxOuts, vector<Transaction> txPool);

#endif
