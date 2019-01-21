#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>

#ifndef __WALLET_DEFINITION__
#define __WALLET_DEFINITION__

//path del file contenente la chiave privata del nodo
const std::string privateKeyLocation = "../NodeWallet/private_key";

//Legge la chiave privata (wallet) del nodo dal file
std::string getPrivateFromWallet();

//Legge la chiave privata del nodo dal file, da essa genera la chiave pubblica e la ritorna
std::string getPublicFromWallet();

//Generazione di una nuova chiave privata
std::string generatePrivateKey();

//inizializzazione del wallet (chiave privata), se il file non esiste si genera una nuova chiave e si salva su file
void initWallet();

//delete the file containing the wallet if it exists
void deleteWallet();

//ritorna la lista degli output non spesi appartenenti ad un certo indirizzo (wallet)
std::vector<UnspentTxOut> findUnspentTxOutsOfAddress(std::string ownerAddress, std::vector<UnspentTxOut> unspentTxOuts);

//calcola il totale degli output non spesi appartenenti ad un certo indirizzo
float getBalance(std::string address, std::vector<UnspentTxOut> unspentTxOuts);

//calcola il totale data una lista di output non spesi
float getTotalFromOutputVector(std::vector<UnspentTxOut> unspentTxOuts);

//Trova gli output non spesi necessari ad eseguire un pagamento dato il totale e la lista degli output non spesi dell'utente che sta effettuando il pagamento
//Viene ritornata la lista degli output non spesi che saranno utilizzati, inoltre si assegna il valore di un puntatore contenente il valore superfluo che dovra essere indirizzato al mittente stesso
std::vector<UnspentTxOut> findTxOutsForAmount(float amount, std::vector<UnspentTxOut> myUnspentTxOuts, float *leftOverAmount);

//Generazione degli output di transazione dati l'amount e la differenza che deve tornare al mittente
std::vector<TxOut> createTxOuts(std::string receiverAddress, std::string myAddress, float amount, float leftOverAmount);

//Verifica se l'output non speso (1° parametro) è referenziato da un input nella lista txIns (2° parametro)
bool isReferencedUnspentTxOut(UnspentTxOut uTxOut, std::vector<TxIn> txIns);

//Ritorna la lista di tutti gli output non spesi, senza quelli che sono referenziati da un qualche input in una delle transazioni presenti nella transaction pool
std::vector<UnspentTxOut> filterTxPoolTxs(std::vector<UnspentTxOut> unspentTxOuts, std::vector<Transaction> transactionPool);

TxIn toUnsignedTxIn(UnspentTxOut unspentTxOut);

Transaction createTransaction(std::string receiverAddress, float amount, std::string privateKey, std::vector<UnspentTxOut> unspentTxOuts, std::vector<Transaction> txPool);

#endif
