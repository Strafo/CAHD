#ifndef TRANSACTIONTABLE_H
#define TRANSACTIONTABLE_H
#include "matplotlib-cpp/matplotlibcpp.h"
#include "fast-cpp-csv-parser/csv.h"
#include <eigen3/Eigen/Sparse>
#include <string>
#include <set>
#include <iostream>
#include <utility>
#include "TransactionDataAnonymizationTypes.h"
#include <forward_list>
#include <fstream>

/**
 * This class incapsulate the behaviour of transaction sparse matrix.
 */
class TransactionTable {
private:
    tda::RowMatrix *matrix;
    std::map<tda::product_t , tda::position_t> products_position_map;
    std::map<tda::invoice_t , tda::position_t> invoice_position_map;
    unsigned int max_transaction_size;
    unsigned int n_fake_entries;
    std::set<tda::product_t> sensitive_items_set;

private:
    void init_transaction_matrix(const std::string &file_name, std::istream &in,bool squared);

    void init_products_invoices_maps(const std::string &file_name, std::istream &in);

    void create_fake_entries();

    void init_sensitive_item_set(const std::list<tda::product_t> &sensitive_items);

    void permuteMaps(const std::vector<tda::position_t> &permutation);

    void permuteMatrix(const std::vector<tda::position_t> &permutation);

public:
    /**CONSTRUCTORS**/
    TransactionTable();

    //TODO add destructor fuck it Os ll take care of it

    TransactionTable(const std::string &file_name, std::istream &in,
                        const std::list<tda::product_t> &sensitive_items,bool squared);

    void permuteTransactionMatrix(const std::vector<tda::position_t> &permutation);

    TransactionTable createSymmetricMatrix() const;

    bool isSensitive(const tda::Transaction_t &transaction) const;// todo use iterator on outer  for better performances

    bool isSensitive(const tda::Transaction_t &transaction,const tda::product_t &sensProd) const;// todo use iterator on outer

    bool areConflicting(const tda::Transaction_t &t1, const tda::Transaction_t &t2) const; //todo use iterator on outer  to compare

    double compareTransactionSimilarity(const tda::Transaction_t &original,const tda::Transaction_t &t2) const;

    void addTransactionToAnonymizedFile(const tda::Transaction_t& toAnonymize,
            const std::forward_list<tda::product_t> &sensItems,std::ofstream &outputFile) const;

    static std::ofstream createAnonymizedTransactionFile(const std::string &fileName);

    bool mask(const std::set<tda::product_t> &productsMask,const tda::Transaction_t &transaction) const;

    std::list<tda::Transaction_t> subTableFromMask(const std::set<tda::product_t> &productsMask) const;

    /**GETTERS**/
    tda::position_t getNumberOfRows() const;

    tda::position_t getNumberOfColumns() const;

    unsigned int getNumberOfFakeEntries() const;

    unsigned int getMaxTransactionSize() const;

    tda::Transaction_t getNextSensitiveTransaction(const tda::Transaction_t &offset) const;

    tda::Transaction_t getTransaction(const tda::position_t &position) const;

    tda::Transaction_t getTransaction(const tda::invoice_t &invoice) const;

    const std::set<tda::product_t> &getSensitiveItemsSet() const;

    std::vector<std::pair</*row*/tda::position_t,/*col*/tda::position_t>> getNonZeroEntriesCordinates() const;

    std::list<tda::Transaction_t> getTransactionList() const;

    std::list<tda::product_t> getSensitiveItemInTransaction(const tda::Transaction_t &transaction) const;

    bool isPresent(const tda::invoice_t &invo,const tda::product_t &prod) const ;

    std::list<tda::invoice_t> getInvoicesList() const;

    std::list<tda::product_t> getProductsList() const;

    /**UTILS**/
    void plotTransactionMatrix();
};


#endif //TRANSACTIONTABLE_H