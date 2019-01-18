#include <vector>

/* STUFF TO BE PORTED

import * as CryptoJS from 'crypto-js';
import * as ecdsa from 'elliptic';
import * as _ from 'lodash';

const ec = new ecdsa.ec('secp256k1');

const COINBASE_AMOUNT: number = 50;
*/
#ifndef __TRANSACTION_DEFINITION__
#define __TRANSACTION_DEFINITION__
class UnspentTxOut {
  public:
    std::string txOutId;
    std::string address;
    int txOutIndex;
    float amount;

    UnspentTxOut(std::string txOutId, int txOutIndex, std::string address, float amount) {
      this->txOutId = txOutId;
      this->txOutIndex = txOutIndex;
      this->address = address;
      this->amount = amount;
    }
};

class TxIn {
  public:
    std::string txOutId;
    std::string signature;
    int txOutIndex;

    TxIn(std::string txOutId, std::string signature, int txOutIndex) {
        this->txOutId = txOutId;
        this->signature = signature;
        this->txOutIndex = txOutIndex;
    }
};

class TxOut {
  public:
    std::string address;
    float amount;

    TxOut(std::string address, float amount) {
        this->address = address;
        this->amount = amount;
    }
};

class Transaction {
  public:
    std::string id;
    std::vector<TxIn> txIns;
    std::vector<TxOut> txOuts;

    Transaction(std::string id, std::vector<TxIn> txIns, std::vector<TxOut> txOuts){
      this->id = id;
      this->txIns.swap(txIns);
      this->txOuts.swap(txOuts);
    }
};



#endif
