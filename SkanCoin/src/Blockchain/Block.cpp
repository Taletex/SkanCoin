#include "Block.hpp"

using namespace std;

Block::Block (int index, string hash, string previousHash, long timestamp, vector<Transaction> data, int difficulty, int nonce) {
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  this->index = index;
  this->previousHash = previousHash;
  this->timestamp = timestamp;
  this->data.swap(data);
  this->hash = hash;
  this->difficulty = difficulty;
  this->nonce = nonce;
}

bool Block::isEqual(Block other){
  #if DEBUG_FLAG == 1
  DEBUG_INFO("");
  #endif

  return this->toString() == other.toString();
}

string Block::toString(){
  string ret = "{\"index\": " + to_string(index) + ", \"previousHash\": \"" + previousHash  + "\", \"timestamp\": " + to_string(timestamp) + ", \"hash\": \"" + hash + "\", \"difficulty\": " + to_string(difficulty)  + ", \"nonce\": " + to_string(nonce)   + ", \"data\": [";
  vector<Transaction>::iterator it;
  for(it = data.begin(); it != data.end(); ++it){
    if(it != data.begin()){
      ret = ret + ", ";
    }
    ret = ret + it->toString();
  }
  ret = ret + "]}";
  return ret;
}
