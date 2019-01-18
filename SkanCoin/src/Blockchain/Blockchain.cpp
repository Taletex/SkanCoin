#include "Transaction.cpp"

#ifndef __BLOCKCHAIN_DEFINITION__
#define __BLOCKCHAIN_DEFINITION__

class Block {
  public:
    int index;
    std::string hash;
    std::string previousHash;
    int timestamp;
    std::vector<Transaction> data;
    int difficulty;
    int nonce;

    Block (int index, std::string hash, std::string previousHash, int timestamp, std::vector<Transaction> data, int difficulty, int nonce) {
      this->index = index;
      this->previousHash = previousHash;
      this->timestamp = timestamp;
      this->data.swap(data);
      this->hash = hash;
      this->difficulty = difficulty;
      this->nonce = nonce;
    }
};

std::vector<TxIn> txInsVector = {TxIn("","",0)};
//txInsVector.push_back();

std::vector<TxOut> txOutsVector = {TxOut("04bfcab8722991ae774db48f934ca79cfb7dd991229153b9f732ba5334aafcd8e7266e47076996b55a14bf9913ee3145ce0cfc1372ada8ada74bd287450313534a", 50)};
//txOutsVector.push_back();

const Transaction genesisTransaction("e655f6a5f26dc9b4cac6e46f52336428287759cf81ef5ff10854f69d68f43fa3", txInsVector, txOutsVector);

std::vector<Transaction> transactionsVector = {genesisTransaction};

const Block genesisBlock(0, "91a73664bc84c0baa1fc75ea6e4aa6d1d20c5df664c724e3159aefc2e1186627", "", std::time(0), transactionsVector, 0, 0);

std::list<Block> blockchain = {genesisBlock};



std::list<Block> getBlockchain(){
  return blockchain;
}

#endif
