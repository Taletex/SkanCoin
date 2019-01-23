#include "P2PServer.hpp"

using namespace std;
const int P2P_PORT= 6001;

void P2PServer::initP2PServer(){
  crow::SimpleApp app;
  CROW_ROUTE(app, "/ws").websocket()
  .onopen([&](crow::websocket::connection& conn){
    CROW_LOG_INFO << "new websocket connection";
    std::lock_guard<std::mutex> _(mtx);
    users.insert(&conn);
  })
  .onclose([&](crow::websocket::connection& conn, const std::string& reason){
    CROW_LOG_INFO << "websocket connection closed: " << reason;
    std::lock_guard<std::mutex> _(mtx);
    users.erase(&conn);
  })
  .onmessage([&](crow::websocket::connection& /*conn*/, const std::string& data, bool is_binary){
    std::lock_guard<std::mutex> _(mtx);
    for(auto u:users)
        if (is_binary)
            u->send_binary(data);
        else
            u->send_text(data);
  });
  cout << "Starting P2PServer..." << endl;
  app.port(P2P_PORT).run();
}

void P2PServer::connectToPeers(string peer){
  cout << "Connecting to new peer..." << endl;
}

string P2PServer::printPeers(){
  //TODO: restituire una lista di stringhe <indirizzo remoto>:<porta remota>
  return "[\"ciao\",\"ciao\"]";
}
