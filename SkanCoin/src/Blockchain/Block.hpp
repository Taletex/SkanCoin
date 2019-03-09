#ifndef __BLOCK_DEFINITION__
#define __BLOCK_DEFINITION__

#include <iostream>
#include "Transactions.hpp"

class Block {
  public:
    int index;
    int difficulty;
    int nonce;
    long timestamp;
    std::string hash;
    std::string previousHash;
    std::vector<Transaction> data;
    Block(){}
    Block (int index, std::string hash, std::string previousHash, long timestamp, std::vector<Transaction> data, int difficulty, int nonce);
    bool isEqual(Block block);
    std::string toString();
};

#endif
