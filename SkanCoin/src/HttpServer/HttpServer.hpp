#ifndef __HTTP_SERVER_DEFINITION__
#define __HTTP_SERVER_DEFINITION__

#include <ctime>
#include <sstream>
#include "document.h"
#include "crow.h"
#include "../Blockchain/Transactions.hpp"
#include "../Blockchain/Wallet.hpp"
#include "../Blockchain/TransactionPool.hpp"
#include "../Blockchain/Blockchain.hpp"
#include "../P2P/Peer.hpp"
#include "../debug.hpp"

void initHttpServer(int port);

crow::response optionsResponse();

crow::response createResponse(std::string data, int code);

std::vector<TxIn> parseTxInVector(const rapidjson::Value &txIns);

std::vector<TxOut> parseTxOutVector(const rapidjson::Value &txOuts);

std::vector<Transaction> parseTransactionVector(const rapidjson::Value &transactions);

std::string printUnspentTxOuts(std::vector<UnspentTxOut> unspentTxOuts);

std::string printTransactions(std::vector<Transaction> transactions);

#endif
