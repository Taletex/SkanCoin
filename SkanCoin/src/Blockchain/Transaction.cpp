#include <vector>
#include <typeinfo>
#include "ecc.h"
#include "picosha2.h"
#include <regex>

#ifndef __TRANSACTION_DEFINITION__
#define __TRANSACTION_DEFINITION__

using namespace std;

const int COINBASE_AMOUNT = 10;

class UnspentTxOut {
  public:
    string txOutId;
    string address;
    int txOutIndex;
    float amount;

    UnspentTxOut(string txOutId, int txOutIndex, string address, float amount) {
      this->txOutId = txOutId;
      this->txOutIndex = txOutIndex;
      this->address = address;
      this->amount = amount;
    }
};

class TxIn {
  public:
    string txOutId;
    string signature;
    int txOutIndex;

    TxIn(string txOutId, string signature, int txOutIndex) {
        this->txOutId = txOutId;
        this->signature = signature;
        this->txOutIndex = txOutIndex;
    }

    string toString(){
      return "{'txOutId': " + this->txOutId + ", 'signature': " + this->signature + ", 'txOutIndex': " + to_string(this->txOutIndex) + "}";
    }
};

class TxOut {
  public:
    string address;
    float amount;

    TxOut(string address, float amount) {
        this->address = address;
        this->amount = amount;
    }
};

class Transaction {
  public:
    string id;
    vector<TxIn> txIns;
    vector<TxOut> txOuts;

    Transaction(string id, vector<TxIn> txIns, vector<TxOut> txOuts){
      this->id = id;
      this->txIns.swap(txIns);
      this->txOuts.swap(txOuts);
    }
};

string hexToBinaryLookup(char c){
  string ret= "";
  switch(c){
      case '0': ret = "0000";
      break;
      case '1': ret = "0001";
      break;
      case '2': ret = "0010";
      break;
      case '3': ret = "0011";
      break;
      case '4': ret = "0100";
      break;
      case '5': ret = "0101";
      break;
      case '6': ret = "0110";
      break;
      case '7': ret = "0111";
      break;
      case '8': ret = "1000";
      break;
      case '9': ret = "1001";
      break;
      case 'a': ret = "1010";
      break;
      case 'b': ret = "1011";
      break;
      case 'c': ret = "1100";
      break;
      case 'd': ret = "1101";
      break;
      case 'e': ret = "1110";
      break;
      case 'f': ret = "1111";
      break;
      default: ret = "";
  }
  return ret;
}

string hexToBinary(string s){
    string ret = "";
    for(char& c : s) {
      ret = ret + hexToBinaryLookup(c);
    }
    return ret;
};

string getTransactionId (Transaction transaction){
  string txInContent = "";
  string txOutContent = "";
  vector<TxIn>::iterator it;
  for(it = transaction.txIns.begin(); it != transaction.txIns.end(); ++it){
    txInContent = txInContent + it->txOutId + to_string(it->txOutIndex);
  }
  vector<TxOut>::iterator it2;
  for(it2 = transaction.txOuts.begin(); it2 != transaction.txOuts.end(); ++it2){
    txOutContent = txOutContent + it2->address + to_string(it2->amount);
  }
  return picosha2::hash256_hex_string(txInContent + txOutContent);;
};

// valid address is a valid ecdsa public key in the 04 + X-coordinate + Y-coordinate format
bool isValidAddress(string address){
  if(!regex_match (address, regex("^[0][4][a-fA-F0-9]{128}$") )){
    cout << "public key must contain only 130 hex characters and must begin with '04': " << address << endl;
    return false;
  }
  return true;
};

bool validateTxIn(TxIn txIn, Transaction transaction, vector<UnspentTxOut> aUnspentTxOuts){
    bool flag = false;
    UnspentTxOut referencedUTxOut = UnspentTxOut("", -1, "", 0);
    vector<UnspentTxOut>::iterator it;
    for(it = aUnspentTxOuts.begin(); it != aUnspentTxOuts.end(); ++it){
      if(it->txOutId == txIn.txOutId && it->txOutIndex == txIn.txOutIndex){
        referencedUTxOut = *it;
        flag = true;
      }
    }
    if (flag == false) {
        cout << "referenced txOut not found: " << txIn.toString();
        return false;
    }

    string address = referencedUTxOut.address;
    /*vector<uint8_t> addr(address.begin(), address.end());
    vector<uint8_t> idd(transaction.id.begin(), transaction.id.end());
    vector<uint8_t> sigg(txIn.signature.begin(), txIn.signature.end());

    if (ecdsa_verify(addr, idd, sigg) == 0) {
        cout << "invalid txIn signature: " + txIn.signature + ", txId: " + transaction.id + ", address: " + referencedUTxOut.address;
        return false;
    }*/
    return true;
};

float getTxInAmount(TxIn txIn, vector<UnspentTxOut> aUnspentTxOuts){
  vector<UnspentTxOut>::iterator it;
  for(it = aUnspentTxOuts.begin(); it != aUnspentTxOuts.end(); ++it){
    if(it->txOutId == txIn.txOutId && it->txOutIndex == txIn.txOutIndex){
      return it->amount;
    }
  }
  return 0;
};

bool isValidTxInStructure(TxIn txIn){
    if (typeid(txIn.signature) != typeid(string)) {
        cout << "invalid signature type in txIn" << endl;
        return false;
    } else if (typeid(txIn.txOutId) != typeid(string)) {
        cout << "invalid txOutId type in txIn" << endl;
        return false;
    } else if (typeid(txIn.txOutIndex) != typeid(int)) {
        cout << "invalid txOutIndex type in txIn" << endl;
        return false;
    } else {
        return true;
    }
};

bool isValidTxOutStructure(TxOut txOut){
    if (typeid(txOut.address) != typeid(string)) {
        cout << "invalid address type in txOut" << endl;
        return false;
    } else if (!isValidAddress(txOut.address)) {
        cout << "invalid TxOut address" << endl;
        return false;
    } else if (typeid(txOut.amount) != typeid(float)) {
        cout << "invalid amount type in txOut" << endl;
        return false;
    } else {
        return true;
    }
};

bool isValidTransactionStructure(Transaction transaction){
    if (typeid(transaction.id) != typeid(string)) {
        cout << "transactionId missing" << endl;
        return false;
    }
    if (!(typeid(transaction.txIns) ==  (typeid(vector<TxIn>)))) {
        cout << "invalid txIns type in transaction" << endl;
        return false;
    }
    vector<TxIn>::iterator it;
    for(it = transaction.txIns.begin(); it != transaction.txIns.end(); ++it){
      if(!isValidTxInStructure(*it)){
        return false;
      }
    }

    if (!(typeid(transaction.txOuts) ==  (typeid(vector<TxOut>)))) {
        cout << "invalid txIns type in transaction" << endl;
        return false;
    }
    vector<TxOut>::iterator it2;
    for(it2 = transaction.txOuts.begin(); it2 != transaction.txOuts.end(); ++it2){
      if(!isValidTxOutStructure(*it2)){
        return false;
      }
    }
    return true;
};

bool validateTransaction(Transaction transaction, vector<UnspentTxOut> aUnspentTxOuts){

    if (!isValidTransactionStructure(transaction)) {
        return false;
    }

    if (getTransactionId(transaction).compare(transaction.id) != 0) {
        cout << "invalid tx id: " << transaction.id << endl;
        return false;
    }
    vector<TxIn>::iterator it;
    for(it = transaction.txIns.begin(); it != transaction.txIns.end(); ++it){
      if(!validateTxIn(*it, transaction, aUnspentTxOuts)){
        cout << "some of the txIns are invalid in tx: " << transaction.id << endl;
        return false;
      }
    }

    float totalTxInValues = 0;
    for(it = transaction.txIns.begin(); it != transaction.txIns.end(); ++it){
      float singleAmount = getTxInAmount(*it, aUnspentTxOuts);
      if(singleAmount == 0){
        return false;
      }
      totalTxInValues = totalTxInValues + singleAmount;
    }

    vector<TxOut>::iterator it2;
    float totalTxOutValues = 0;
    for(it2 = transaction.txOuts.begin(); it2 != transaction.txOuts.end(); ++it2){
      totalTxOutValues = totalTxOutValues + it2->amount;
    }

    if (totalTxOutValues != totalTxInValues) {
        cout << "totalTxOutValues != totalTxInValues in tx: " + transaction.id << endl;
        return false;
    }

    return true;
};

#endif
