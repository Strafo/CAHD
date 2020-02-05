#ifndef TRANSACTIONWRAPPER_H
#define TRANSACTIONWRAPPER_H
#include <utility>
#include "TransactionDataAnonymizationTypes.h"

class TransactionWrapper {
public:
    const tda::Transaction_t transaction;
    const bool sensitive;
    const std::list<tda::product_t>sensProds;
    TransactionWrapper(tda::Transaction_t t,std::list<tda::product_t> l):
            transaction(std::move(t)),sensitive(true),sensProds(std::move(l)){};

    explicit TransactionWrapper(tda::Transaction_t t):transaction(std::move(t)),sensitive(false),sensProds(){};

    TransactionWrapper(const TransactionWrapper &t):sensitive(t.sensitive),transaction(t.transaction),sensProds(t.sensProds){};

    static bool areConflicting(const TransactionWrapper &t1,const TransactionWrapper &t2){
        if(!t1.sensitive||!t2.sensitive)return false;
        for(const auto &i:t1.sensProds){
            for(const auto &j:t2.sensProds){
                if(i==j)return true;
            }
        }
    }
};
#endif //TRANSACTIONWRAPPER_H