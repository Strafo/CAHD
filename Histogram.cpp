#include "Histogram.h"
using namespace std;
using namespace tda;

void Histogram::printHistogram() const {
    //verbose
    cout<<"HISTOGRAM---------------------------------------------"<<endl;
    for(const auto& elem : *histogram)
    {
        cout << "StockCode:"<<elem.first << "\t Number of sensitive transactions:" << elem.second << endl;
    }
    cout<<"------------------------------------------------------"<<endl;
}

bool Histogram::isEmpty() const{
    for(const auto &i :*histogram)
        if(i.second!=0)
            return false;
    return true;
}

Histogram::Histogram(const list<product_t>&sensitiveItemList) {
    histogram=new map<product_t ,unsigned int>();
    for( const auto &i:sensitiveItemList){
        assert(histogram->insert(pair<product_t, unsigned int>(i,0)).second);
    }
}

unsigned int Histogram::getEntry(const invoice_t& entry) const {
    return histogram->at(entry);
}

unsigned int Histogram::getEntryMax() const {
    unsigned int max=0;
    for(const auto &i:*histogram){
        if(i.second>max){
            max=i.second;
        }
    }
    return max;
}

void Histogram::addItem(const tda::product_t& product) {
    auto it = histogram->find(product);
    assert(it!=histogram->end());
    it->second++;
}

void Histogram::update(const forward_list<product_t> &productsList,short n) {
    delete backup;
    assert(!productsList.empty());
    backup= new map<product_t,unsigned int>(*histogram);
    for(const auto &i:productsList){
        auto it = histogram->find(i);
        assert(it!=histogram->end());
        it->second+=n;
    }
}

void Histogram::undo() {
    delete histogram;
    histogram=backup;
    backup= nullptr;
}

/*
void Histogram::init_histogram(){
    pair<map<string,unsigned int>::iterator,bool> ret;
    unsigned int counter;
    position_t pos;

    for(auto const& i:sensitive_items_set){
        counter=0;
        pos = products_position_map.at(i);
        for(Eigen::Index r=0;r<matrix->rows();++r) {
            if(matrix->coeff(r,pos)!=0){
                counter++;
            }
        }
        ret = histogram.insert (pair<string,unsigned int>(i,counter) );
        assert(ret.second);
    }
}
*/


/*

void TransactionsWrapper::update_histogram(const Transaction &sensitive_transaction_removed, int to_add) {
    auto it = histogram.find(sensitive_transaction_removed.first);
    assert(it!=histogram.end());
    it->second+=to_add;
}

void TransactionsWrapper::update_histogram_list(const list<Transaction> &sens_t_list, int to_add) {
    for (auto const& i : histogram) {
        update_histogram(i,to_add);
    }
}

*/
