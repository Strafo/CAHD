//
// Created by Andrea Straforini on 16/01/20.
//
#ifndef TRANSACTIONGROUP_H
#define TRANSACTIONGROUP_H
#include "TransactionDataAnonymizationTypes.h"
#include "TransactionWrapper.h"
#include "TransactionsTable.h"
#include <forward_list>

class TransactionGroup {
private:
    const TransactionTable &transaction_table;
    std::list<std::list<TransactionWrapper>::iterator> transactions={};
public:
    std::list<TransactionWrapper> published{};

    TransactionGroup(const TransactionTable &tb, std::list<TransactionWrapper>::iterator originalEl);

    explicit TransactionGroup(const TransactionTable &tb);

    bool isGroupConflicting(const std::list<TransactionWrapper>::iterator &t) const;

    void addTransaction(std::list<TransactionWrapper>::iterator t);

    /**
     * This method filter 'privacyDegree' transactions from the alpha*p*2 transactions.
     * @param privacyDegree
     */
    void filterForMostSimilarTransactions(short privacyDegree);

    /**
     * This method creates a list containing all the sensitive items contained
     * in the TransactionGroup
     * @return a single linked list
     */
    std::forward_list<tda::product_t> getSensitiveProductsList() const;


    /**
     * This method return the list of sensitive items contained in this PUBLISHED transaction group.
     * This method should be used after 'publishAndRemive' invocation.
     * @return a single linked-list
     */
    std::forward_list<tda::product_t> getSensitiveProductsListPublished() const;

    /**
     * This method publishes group transactions by saving them within the class.
     * Published transactions are then removed from the list 'listToRemove'.
     * @param listToRemove is updated
     */
    void publishAndRemove(std::list<TransactionWrapper> &listToRemove);
};
#endif //TRANSACTIONGROUP_H
