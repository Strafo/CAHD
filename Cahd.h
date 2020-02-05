#ifndef CAHD_H
#define CAHD_H
#include "TransactionsTable.h"
#include "Histogram.h"
#include "TransactionGroup.h"
#include "TransactionWrapper.h"
#include <utility>
#include <stdexcept>

class Cahd {
private:
    Histogram *histogram;
    const TransactionTable &transactionTable;
    std::list<TransactionWrapper> transaction_list;
    std::list<TransactionWrapper>::iterator get_next_sensitive_transaction(std::list<TransactionWrapper>::iterator it ) ;
public:
    Cahd(const TransactionTable &tb, const std::list<tda::Transaction_t>& transactionList, const std::list<tda::product_t>& sensitiveProducts);
    /**
     *
     * @param privacy the number of transaction for each group
     * @param alpha search width parameter
     * @return a list of transaction groups
     */
    std::list<TransactionGroup> cahdGroupFormationEuristic(short privacy,short alpha);
    ~Cahd();
};
#endif //CAHD_H