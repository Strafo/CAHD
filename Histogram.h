#ifndef HISTOGRAM_H
#define HISTOGRAM_H
#include "TransactionDataAnonymizationTypes.h"
#include <map>
#include <iostream>
#include <forward_list>


class Histogram {
    std::map<tda::product_t ,unsigned int>*histogram;
    std::map<tda::product_t ,unsigned int>*backup= nullptr;

public:
    explicit Histogram(const std::list<tda::product_t> &sensitiveItemList );
    bool isEmpty() const ;
    void printHistogram() const ;
    unsigned int getEntry(const tda::invoice_t& entry) const;
    /**
     * @return the highest column.
     */
    unsigned int getEntryMax() const;
    /**
     * Initialize the column for the histogram
     * @param product the new column
     */
    void addItem(const tda::product_t& product);
    /**
     * Add, for each product in product list, toAdd  to the column.
     * @param productsList t
     * @param toAdd
     */
    void update(const std::forward_list<tda::product_t> &productsList,short toAdd);
    /**
     * Undo the last update operation.
     */
    void undo();
};
#endif //HISTOGRAM_H