#ifndef __HTTP_SERVER_DEFINITION__
#define __HTTP_SERVER_DEFINITION__

#include <ctime>
#include <sstream>
#include "document.h"
#include "crow.h"
#include "config.hpp"
#include "../Blockchain/Block.hpp"
#include "../Blockchain/Transactions.hpp"
#include "../Blockchain/Wallet.hpp"
#include "../Blockchain/TransactionPool.hpp"
#include "../Blockchain/Blockchain.hpp"
#include "../P2P/Peer.hpp"

void initHttpServer(int port);

/* Ritorna una response crow con un body contenente data e un codice code */
crow::response createResponse(std::string data, int code);

/* Ritorna una response crow per le richiste di tipo OPTIONS (per gestire il CORS) */
crow::response optionsResponse();

/* Effettua il parsing di una lista di blocchi: JSON (rapidjson) -> list<Block> */
std::list<Block> parseBlockList(const rapidjson::Value &blocks);

/* Effettua il parsing di un vettore di transazioni: JSON (rapidjson) -> vector<Transaction> */
std::vector<Transaction> parseTransactionVector(const rapidjson::Value &transactions);

/* Effettua il parsing di un vettore di transaction input: JSON (rapidjson) -> vector<TransIn> */
std::vector<TransIn> parseTransInVector(const rapidjson::Value &transIns);

/* Effettua il parsing di un vettore di transaction output: JSON (rapidjson) -> vector<TransOut> */
std::vector<TransOut> parseTransOutVector(const rapidjson::Value &transOuts);

/* Stampa un vector di transazioni: vector<Transaction> -> String (da inserire in un JSON) */
std::string printTransactions(std::vector<Transaction> transactions);

/* Stampa un vector di unspentTransOuts: vector<UnspentTransOut> -> String (da inserire in un JSON) */
std::string printUnspentTransOuts(std::vector<UnspentTransOut> unspentTransOuts);

#endif


/* Definizione della macro BOOST_SYSTEM_NO_DEPRECATED per risolvere il bug
   boost::system::throws della libreria Boost
   http://boost.2283326.n4.nabble.com/Re-Boost-build-System-Link-issues-using-BOOST-ERROR-CODE-HEADER-ONLY-td4688963.html
   https://www.boost.org/doc/libs/1_56_0/libs/system/doc/reference.html#Header-
*/
#ifndef BOOST_SYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED
#endif
