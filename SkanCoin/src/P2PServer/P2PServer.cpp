#include "P2PServer.hpp"

using namespace std;
void P2PServer::initP2PServer(int port){
  crow::SimpleApp app;

  CROW_ROUTE(app, "/").websocket()
    .onopen([&](crow::websocket::connection& connection){
      //initConnection(connection);
      CROW_LOG_INFO << "new websocket connection";
      std::lock_guard<std::mutex> _(mtx);
      users.insert(&connection);
      //write(ws, queryChainLengthMsg());
      //broadcast(queryTransactionPoolMsg());
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason){
      CROW_LOG_INFO << "websocket connection closed: " << reason;
      std::lock_guard<std::mutex> _(mtx);
      users.erase(&conn);
    })
    .onmessage([&](crow::websocket::connection& connection, const std::string& data, bool is_binary){
      std::lock_guard<std::mutex> _(mtx);
      for(auto u:users)
          if (is_binary)
              u->send_binary(data);
          else
              u->send_text("world");
    });

  cout << "Starting P2PServer on port " << port << "..." << endl;
  app.port(port).run();
}

void write(crow::websocket::connection* connection, Message message){
  connection->send_text("ciao");
}

void P2PServer::connectToPeers(string peer){
  cout << "Connecting to new peer..." << endl;
}

string P2PServer::printPeers(){
  //TODO: restituire una lista di stringhe <indirizzo remoto>:<porta remota>
  return "[\"ciao\",\"ciao\"]";
}
