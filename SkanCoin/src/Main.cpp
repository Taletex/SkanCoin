//Standard libraries//
#include <iostream>

//header files//
#include "Blockchain/Blockchain.hpp"
#include "Blockchain/Wallet.hpp"
#include "HttpServer/HttpServer.hpp"
#include "P2P/Peer.hpp"
#include "config.hpp"

using namespace std;

void initP2PServer(int port){
  Peer::getInstance().initP2PServer(port);
}

void startP2PClient(){
  Peer::getInstance().startClientPoll();
}

int main(){
  cout << "######################################################" << endl;
  cout << "############### " << PROJECT_NAME << " - version " << VERSION_NUMBER << " ###############" << endl;
  cout << "######################################################" << endl << endl;

  try{
    cout <<"Avvio del nodo..." << endl << "Generazione del Wallet..." << endl;
    initWallet();
  }catch(const char* msg){
    cout << msg << endl;
    cout << "ECCEZIONE: Creazione del wallet fallita!" << endl;
    return 0;
  }
  cout << "Creazione della Blockchain..." << endl;
  try{
    BlockChain::getInstance();
    cout << "BlockChain creata!" << endl;
  }catch(const char* msg){
    cout << msg << endl;
    cout << "ECCEZIONE: Creazione della Blockchain fallita!" << endl;
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


/* Definizione della macro BOOST_SYSTEM_NO_DEPRECATED per risolvere il bug
   boost::system::throws della libreria Boost
   http://boost.2283326.n4.nabble.com/Re-Boost-build-System-Link-issues-using-BOOST-ERROR-CODE-HEADER-ONLY-td4688963.html
   https://www.boost.org/doc/libs/1_56_0/libs/system/doc/reference.html#Header-
*/
#ifndef BOOST_SYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED
#endif
