#ifndef __TRANSACTION_COMPONENTS_DEFINITION__
#define __TRANSACTION_COMPONENTS_DEFINITION__

#include <iostream>

class UnspentTransOut {
  public:
    std::string transOutId;
    std::string address;
    int transOutIndex;
    float amount;
    UnspentTransOut();
    UnspentTransOut(std::string transOutId, int transOutIndex, std::string address, float amount);
    bool isEqual(UnspentTransOut other);
    std::string toString();
};

class TransIn {
  public:
    std::string transOutId;
    std::string signature;
    int transOutIndex;
    TransIn();
    TransIn(std::string transOutId, std::string signature, int transOutIndex);
    bool isEqual(TransIn other);
    std::string toString();
};

class TransOut {
  public:
    std::string address;
    float amount;
    TransOut();
    TransOut(std::string address, float amount);
    std::string toString();
};

#endif
