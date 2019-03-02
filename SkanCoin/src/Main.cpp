//Standard libraries//
#include <iostream>
#include <thread>
//header files//
#include "Blockchain/Blockchain.hpp"
#include "Blockchain/Transactions.hpp"
#include "Blockchain/TransactionPool.hpp"
#include "Blockchain/Wallet.hpp"
#include "HttpServer/HttpServer.hpp"
#include "P2P/Peer.hpp"
#include "easywsclient.hpp"
#include "easywsclient.cpp"
#include "ecc.h"

//source files//
#include "Blockchain/TransactionPool.cpp"
#include "Blockchain/Transactions.cpp"
#include "Blockchain/Blockchain.cpp"
#include "HttpServer/HttpServer.cpp"
#include "Blockchain/Wallet.cpp"
#include "P2P/Peer.cpp"
#include "ecc.c"

using namespace std;

void initP2PServer(int port){
  Peer::getInstance().initP2PServer(port);
}

void startP2PClient(){
  Peer::getInstance().startClientPoll();
}

int main(){
  try{
    cout <<"Starting node..." << endl << "Generating wallet..." << endl;
    initWallet();
  }catch(const char* msg){
    cout << msg << endl;
    cout << "EXCEPTION: Wallet creation failed!" << endl;
    return 0;
  }
  cout << "Generating blockchain..." << endl;
  try{
    cout << "BLOCKCHAIN: " << endl <<  BlockChain::getInstance().toString() << endl;
  }catch(const char* msg){
    cout << msg << endl;
    cout << "EXCEPTION: Blockchain generation failed!" << endl;
    return 0;
  }

  thread httpServer (initHttpServer,3001);
  thread p2pServer (initP2PServer,6001);
  thread p2pClient (startP2PClient);
  httpServer.join();
  p2pServer.join();
  p2pClient.join();
  return 0;
}

/*
uint8_t p_publicKey[ECC_BYTES+1];
uint8_t p_privateKey[ECC_BYTES];
int a = ecc_make_key(p_publicKey, p_privateKey);
for(int i = 0; i < ECC_BYTES+1; i++){
  cout << p_publicKey[i] << endl;
}
cout << endl << endl << endl;

ofstream myfile, myfile2;
myfile.open ("public_key", ios::out | ios::trunc);
myfile2.open ("private_key", ios::out | ios::trunc);
if(myfile && myfile2) {
  for(int i = 0; i < ECC_BYTES+1; i++){
    myfile << p_publicKey[i] << endl;
  }
} else {
  throw "EXCEPTION: Errore durante il salvataggio delle chiavi su file!";
}



//Vogliamo ottenere una conversione a stringa che sia reversibile (qua proviamo sull'array p_publicKey)
string temp = "";
for (int a = 0; a < ECC_BYTES+1; a++) {
  if(a != 0){
    temp += ".";
  }
  temp +=  to_string((int)p_publicKey[a]);
}

cout << endl << endl << endl << temp << endl << endl ;
stringstream tempstream(temp);
string segment;
int x;
int i = 0;
while(std::getline(tempstream, segment, '.')){
  x = stoi(segment);
  p_publicKey[i] = x;

  cout << i << " : " << p_publicKey[i] << endl;
   i++;
}





//Recupero new_public da file

uint8_t new_public[ECC_BYTES+1];
ifstream inFile;
inFile.open("public_key");
if(inFile) {
  int i = 0;
    while (!inFile.eof()) {
        inFile >> new_public[i];
        i++;
    }
} else {
  throw "Errore: non è stato possibile aprire il file!";
}

//confronto new_public (caricata da file) con p_publicKey, ottenuta dalla stringa
//Per semplificare la gestione delle chiavi queste vengono convertite in stinghe
//per evitare errori di conversione accetto solo chiavi che non contengono
//caratteri senza una corrispondenza univoca con valori interi
bool flag = 1;
for(int i = 0; i < ECC_BYTES+1; i++){
  if(p_publicKey[i] != new_public[i]){
    flag = 0;
  }
  cout << p_publicKey[i] << " " << new_public[i] << endl;
}
cout << "flag: " << flag << endl;
return 0;


///FIRMA E VERIFICA

uint8_t p_hash[ECC_BYTES];
uint8_t p_signature[ECC_BYTES*2];
string myString = "alalalalalalal";
myString = picosha2::hash256_hex_string(myString);
memcpy (p_hash, myString.c_str(), ECC_BYTES);

//int signCreated = ecdsa_sign(new_private, p_hash, p_signature);
//cout << signCreated << endl << endl;
for(int i = 0; i < ECC_BYTES+1; i++){
  cout << p_signature[i] << endl;
}

int signOk = ecdsa_verify(p_publicKey, p_hash , p_signature);
cout << signOk << endl;
*/
