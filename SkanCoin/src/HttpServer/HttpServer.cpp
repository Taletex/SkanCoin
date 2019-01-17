#include "crow.h"

int main()
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

    app.port(18080).run();
}
