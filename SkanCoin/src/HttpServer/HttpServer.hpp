#ifndef __HTTP_SERVER_DEFINITION__
#define __HTTP_SERVER_DEFINITION__

#include "document.h"
#include "crow.h"
#include "../Blockchain/Transactions.hpp"
#include "../Blockchain/Wallet.hpp"
#include "../Blockchain/TransactionPool.hpp"
#include "../Blockchain/Blockchain.hpp"
#include "../P2P/P2PServer.hpp"

std::vector<TxIn> parseTxInVector(const rapidjson::Value &txIns);

std::vector<TxOut> parseTxOutVector(const rapidjson::Value &txOuts);

std::vector<Transaction> parseTransactionVector(const rapidjson::Value &transactions);

std::string printUnspentTxOuts(std::vector<UnspentTxOut> unspentTxOuts);

std::string printTransactions(std::vector<Transaction> transactions);

Block getBlockFromHash(std::list<Block> blockchain, std::string hash);

Transaction getTransactionFromId(std::list<Block> blockchain, std::string id);

void initHttpServer(int port);
#endif
