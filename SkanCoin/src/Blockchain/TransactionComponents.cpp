#include "TransactionComponents.hpp"

using namespace std;

UnspentTransOut::UnspentTransOut() {}

UnspentTransOut::UnspentTransOut(string transOutId, int transOutIndex, string address, float amount) {
  #if DEBUG_FLAG == 1
    DEBUG_INFO("");
  #endif

  this->transOutId = transOutId;
  this->transOutIndex = transOutIndex;
  this->address = address;
  this->amount = amount;
}

bool UnspentTransOut::isEqual(UnspentTransOut other){
  #if DEBUG_FLAG == 1
    DEBUG_INFO("");
  #endif

  return (this->transOutId == other.transOutId && this->transOutIndex == other.transOutIndex);
}

string UnspentTransOut::toString(){
  return "{\"transOutId\": \"" + this->transOutId + "\", \"address\": \"" + this->address + "\", \"transOutIndex\": " + to_string(this->transOutIndex) + ", \"amount\": " + to_string(this->amount) + "}";
}

TransIn::TransIn() {}

TransIn::TransIn(string transOutId, string signature, int transOutIndex) {
  #if DEBUG_FLAG == 1
    DEBUG_INFO("");
  #endif

  this->transOutId = transOutId;
  this->signature = signature;
  this->transOutIndex = transOutIndex;
}

bool TransIn::isEqual(TransIn other){
  #if DEBUG_FLAG == 1
    DEBUG_INFO("");
  #endif

  return (this->transOutId == other.transOutId && this->transOutIndex == other.transOutIndex);
}

string TransIn::toString(){
  return "{\"transOutId\": \"" + transOutId + "\", \"signature\": \"" + signature + "\", \"transOutIndex\": " + to_string(transOutIndex) + "}";
}

TransOut::TransOut(){}

TransOut::TransOut(string address, float amount) {
  #if DEBUG_FLAG == 1
    DEBUG_INFO("");
  #endif

  this->address = address;
  this->amount = amount;
}

string TransOut::toString(){
  return "{\"address\": \"" + address + "\", \"amount\": " + to_string(amount) + "}";
}
