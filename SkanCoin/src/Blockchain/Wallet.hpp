#ifndef __WALLET_DEFINITION__
#define __WALLET_DEFINITION__

#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include "../debug.hpp"

//path del file contenente la chiave privata del nodo
const std::string privateKeyLocation = "private_key";
const std::string publicKeyLocation = "public_key";

/*La libreria utilizzata per la firma digitale basata su curve ellittiche utilizza
chiavi (e produce signature) in forma di array di byte (uint8_t), tuttavia
effettuiamo una conversione (reversibile) a stringa sulle chiavi e sulle signature
per semplificarne la gestione nelle classi esterne.
In particolare questo ci permette di avere un formato
stampabile e più facilmente interpretabile nelle interazioni tra i peer e
e nei payload delle richieste/risposte HTTP
Il formato usato per le stringhe prevede di stampare il valore numerico di ogni byteArrayFromString
dell'array, utilizzando il punto come separatore (es: 33.243.453.334. ....)*/

/*Conversione array di byte -> stringa*/
std::string stringFromByteArray(uint8_t *array, int len);

/*Conversione stringa -> array di byte*/
void byteArrayFromString(std::string str, uint8_t *dest);

/*Carica la chiave (pubblica o privata) dall'apposito file*/
std::string loadKey(bool isPrivate);

/*Salvataggio della chiave (pubblica o privata) nell'apposito file*/
void saveKey(bool isPrivate);

/*Legge la chiave privata del nodo dal file*/
std::string getPrivateFromWallet();

/*Legge la chiave pubblica del nodo dal file*/
std::string getPublicFromWallet();

/*Generazione di una nuova coppia di chiavi e salvataggio negi appositi file*/
void generateKeys();

/*Inizializzazione del wallet (chiave privata), se il file non esiste si
genera una nuova chiave e si salva su file*/
void initWallet();

/*Eliminazione del wallet (file contenenti le chiavi)*/
void deleteWallet();

/*Ritorna la lista degli output non spesi appartenenti ad un certo indirizzo (wallet)*/
std::vector<UnspentTxOut> findUnspentTxOutsOfAddress(std::string ownerAddress, std::vector<UnspentTxOut> unspentTxOuts);

/*Calcolo del totale degli output non spesi appartenenti ad un certo indirizzo*/
float getBalance(std::string address, std::vector<UnspentTxOut> unspentTxOuts);

/*Calcola il totale data una lista di output non spesi*/
float getTotalFromOutputVector(std::vector<UnspentTxOut> unspentTxOuts);

/*Trova gli output non spesi necessari ad eseguire un pagamento dato il totale
 e la lista degli output non spesi dell'utente che sta effettuando il pagamento.
Viene ritornata la lista degli output non spesi che saranno utilizzati,
inoltre si assegna il valore di un puntatore contenente il valore superfluo
che dovra essere indirizzato al mittente stesso*/
std::vector<UnspentTxOut> findTxOutsForAmount(float amount, std::vector<UnspentTxOut> myUnspentTxOuts, float *leftOverAmount);

/*Generazione degli output di transazione dati l'importo e la
differenza che deve tornare al mittente*/
std::vector<TxOut> createTxOuts(std::string receiverAddress, std::string myAddress, float amount, float leftOverAmount);

/*Verifica se l'output non speso (1° parametro) è referenziato da un
input nella lista txIns (2° parametro)*/
bool isReferencedUnspentTxOut(UnspentTxOut uTxOut, std::vector<TxIn> txIns);

/*Ritorna la lista di tutti gli output non spesi, senza quelli che sono
referenziati da un qualche input in una delle transazioni presenti nella transaction pool*/
std::vector<UnspentTxOut> filterTxPoolTxs(std::vector<UnspentTxOut> unspentTxOuts, std::vector<Transaction> transactionPool);

/*Creazione di un input di transazione a partire da un output non speso
a cui questo farà riferimento*/
TxIn toUnsignedTxIn(UnspentTxOut unspentTxOut);

/*Generazione di una nuova transazione*/
Transaction createTransaction(std::string receiverAddress, float amount, std::string privateKey, std::vector<UnspentTxOut> unspentTxOuts, std::vector<Transaction> txPool);

/*Generazione di una nuova transazione, a partire da un array di output, per
permettere di esporre la rest per effettuare più movimenti dallo stesso wallet
in un solo blocco senza passare dal transaction pool*/
Transaction createTransactionWithMultipleOutputs (std::vector<TxOut> txOuts, std::string privateKey, std::vector<UnspentTxOut> unspentTxOuts, std::vector<Transaction> txPool);

#endif
