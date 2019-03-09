#ifndef __WALLET_DEFINITION__
#define __WALLET_DEFINITION__

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <fstream>
#include <streambuf>
#include <vector>
#include "ecc.h"
#include "config.hpp"
#include "TransactionComponents.hpp"

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

/*Conversione stringa -> array di byte*/
void byteArrayFromString(std::string str, uint8_t *dest);

/* Genera una signature a partire da una chiave privata e un hash da firmare.
Viene utilizzato in transactions durante la firma degli input degli input di
transazione. Se la firma va a buon fine il metodo ritorna 1 e il puntatore a
stringa passato come terzo parametro viene riempito con la signature prodotta */
int createSignature(std::string key, std::string hash, std::string& signature);

/*Generazione degli output di transazione dati l'importo e la differenza che deve
  tornare al mittente */
std::vector<TransOut> createTransOuts(std::string receiverAddress, std::string myAddress, float amount, float leftOverAmount);

/*Eliminazione del wallet (file contenenti le chiavi)*/
void deleteWallet();

/*Generazione di una nuova coppia di chiavi e salvataggio negi appositi file*/
void generateKeys();

/*Calcolo del totale degli output non spesi appartenenti ad un certo indirizzo*/
float getBalance(std::string address, std::vector<UnspentTransOut> unspentTransOuts);

/*Calcola il totale data una lista di output non spesi*/
float getTotalFromOutputVector(std::vector<UnspentTransOut> unspentTransOuts);

/*Trova gli output non spesi necessari ad eseguire un pagamento dato il totale
 e la lista degli output non spesi dell'utente che sta effettuando il pagamento.
Viene ritornata la lista degli output non spesi che saranno utilizzati,
inoltre si assegna il valore di un puntatore contenente il valore superfluo
che dovra essere indirizzato al mittente stesso*/
std::vector<UnspentTransOut> getTransOutsForAmount(float amount, std::vector<UnspentTransOut> myUnspentTransOuts, float *leftOverAmount);

/*Ritorna la lista degli output non spesi appartenenti ad un certo indirizzo (wallet)*/
std::vector<UnspentTransOut> getUnspentTransOutsOfAddress(std::string ownerAddress, std::vector<UnspentTransOut> unspentTransOuts);

/*Legge la chiave privata del nodo dal file*/
std::string getWalletPrivateKey();

/*Legge la chiave pubblica del nodo dal file*/
std::string getWalletPublicKey();

/*Inizializzazione del wallet (chiave privata), se il file non esiste si
genera una nuova chiave e si salva su file*/
void initWallet();

/*Verifica se l'output non speso (1° parametro) è referenziato da un
input nella lista transIns (2° parametro)*/
bool isReferencedUnspentTransOut(UnspentTransOut uTransOut, std::vector<TransIn> transIns);

/*Carica la chiave (pubblica o privata) dall'apposito file*/
std::string loadKey(bool isPrivate);

/*Salvataggio della chiave (pubblica o privata) nell'apposito file*/
void saveKey(bool isPrivate);

/*Verifica della firma, effettuo le conversioni ad array di byte, per ottenere
un formato compatibile con l'interfaccia della libreria per ECDSA
Funzione utilizzata per verificare la firma posta su un input di Transazione
che deve corrispondere alla firma dell'id della transazione effettuata con la
chiave privata del proprietario dell'output di transazione referenziato */
int signVerify(std::string key, std::string hash, std::string signature);

/*Conversione array di byte -> stringa*/
std::string stringFromByteArray(uint8_t *array, int len);

#endif
