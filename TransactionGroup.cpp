#include "TransactionGroup.h"

using namespace tda;
using namespace std;

bool TransactionGroup::isGroupConflicting(const std::list<TransactionWrapper>::iterator &t) const{
    if(!t->sensitive)return false;
    for(const auto &s:transactions) {
        if(TransactionWrapper::areConflicting(*t,*s)){
            return true;
        }
    }
    return false;
}

TransactionGroup::TransactionGroup(const TransactionTable &tb, std::list<TransactionWrapper>::iterator originalEl)
        :transaction_table(tb){
    transactions.push_back(originalEl);
}

TransactionGroup::TransactionGroup(const TransactionTable &tb)
        :transaction_table(tb){}

void TransactionGroup::addTransaction( std::list<TransactionWrapper>::iterator t) {
    transactions.push_back(t);
}

// Driver function to sort the vector elements by first element of pairs
bool sortbyfirstdescendig(const pair<float,list<list<TransactionWrapper>::iterator>::iterator> &a,
        const pair<float,list<list<TransactionWrapper>::iterator>::iterator> &b){
    return (a.first > b.first);
}

/*todo ricordarsi che alla pubblicazione è necessario randomizzare le  liste
perchè la sens transaction altrimenti è sempre in prima posizione!*/
void TransactionGroup::filterForMostSimilarTransactions(short privacyDegree) {
    vector<pair<double,list<list<TransactionWrapper>::iterator>::iterator>>similarityPoints;
    similarityPoints.reserve(transactions.size()-1);
    auto doubleit=transactions.begin();
    ++doubleit;//skipping first element
    for(short i=0; doubleit != transactions.end(); ++doubleit,++i) {
        similarityPoints.emplace_back(
                        transaction_table.compareTransactionSimilarity(
                                transactions.front()->transaction,(*doubleit)->transaction
                                ),
                        doubleit
                        );
    }
    //sort the result
    sort(similarityPoints.begin(),similarityPoints.end(),sortbyfirstdescendig);
    for(int i=transactions.size();i>privacyDegree;--i){
        auto p=similarityPoints.back().second;
        transactions.erase(p);
        similarityPoints.pop_back();
    }
}

std::forward_list<tda::product_t> TransactionGroup::getSensitiveProductsList() const{
    forward_list<product_t> lista;
    for(const auto &t:transactions){
        if(t->sensitive){
            for(const auto &i:t->sensProds){
                lista.push_front(i);
            }
        }
    }
    return lista;
}

std::forward_list<tda::product_t> TransactionGroup::getSensitiveProductsListPublished() const{
    forward_list<product_t> lista;
    for(const auto &t:published){
        if(t.sensitive){
            for(const auto &i:t.sensProds){
                lista.push_front(i);
            }
        }
    }
    return lista;
}



void TransactionGroup::publishAndRemove(list<TransactionWrapper> &listToRemove) {
    for(auto &i:transactions){
        published.emplace_back(*i);//tapullata con il copy constructor
        listToRemove.erase(i);
        // published.splice(listToRemove.begin(),listToRemove,i); todo not working... why?
    }
    transactions.clear();
    assert(!published.empty());
    //TODO WARNING Possible Vulnerability on randomize function!
    //In the Transaction Group, the sensitive transaction is always in the first position.
    //So we need to randomize the published list. But this can lead to vulnerabilties.
}
