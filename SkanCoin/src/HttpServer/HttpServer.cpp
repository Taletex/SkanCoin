#include "crow.h"
#include "../Blockchain/Blockchain.hpp"

const int httpPort= 3001;

void initHttpServer()
{
    crow::SimpleApp app;
    CROW_ROUTE(app, "/")
    ([]() {
        return "Hello world!";
    });

    CROW_ROUTE(app, "/bella")
    ([]() {
        return "<h1>THIS HTTP SERVER WORKS! ;)</h1>";
    });

    app.port(3001).run();
    std::cout << "Http Server started..." << std::endl;
}
