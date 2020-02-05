
#include "Cahd.h"
using namespace std;
using namespace tda;
extern bool debug;

Cahd::Cahd(const TransactionTable &tb, const std::list<tda::Transaction_t>& tl,
         const std::list<tda::product_t>& sensitiveProducts):
                    transactionTable(tb),
                    transaction_list()
{
    list<product_t>l;
    histogram=new Histogram(sensitiveProducts);
    for(const auto &t:tl){
        l=tb.getSensitiveItemInTransaction(t);
        if(!l.empty()){
            for(const auto &j:l){
                histogram->addItem(j);
            }
            transaction_list.emplace_back(t,l);
        }else{

            transaction_list.emplace_back(t);
        }
    }
};




std::list<TransactionGroup> Cahd::cahdGroupFormationEuristic(short p,short a) {
    if(p<1 || a<1){throw invalid_argument("Privacy degree or alpha must be >=1");}
    list<TransactionWrapper>::reverse_iterator iprec;
    list<TransactionWrapper>::iterator inext;
    list<TransactionGroup> result;
    short i;
    auto it=transaction_list.begin();
    histogram->printHistogram();
    while(!histogram->isEmpty()){
        if(debug){histogram->printHistogram();}
        it=get_next_sensitive_transaction(it);
        assert(!it->transaction.first.empty());
        TransactionGroup group(this->transactionTable,it);
        //OTTIENI ELEMENTI PRECEDENTI/SUCCESIVI ALPHA*PRIVACY
        iprec=make_reverse_iterator(it);//attenzione:reverse iterator restituisce l'elemento precedente a it!
        inext=it;
        ++inext;
        //ASSUNZIONE: ci sono abbastanza apha*p*2 elementi nella lista
        if(debug){cout<<"Log:Creating candidate list."<<endl;}
        for(i=0;i<p*a*2;){
            if(iprec!=transaction_list.rend()) {
                auto itTemp=iprec;                                                                                 //https://stackoverflow.com/questions/51309339/no-match-for-operator-on-reverse-iterator-for-stdmap
                ++itTemp;
                if (!group.isGroupConflicting((itTemp).base())) { //sarebbe (iprec+1).base()
                    group.addTransaction((itTemp).base()); //sarebbe (iprec+1).base()
                    i++;
                }
                ++iprec;
            }
            if(inext!=transaction_list.end()) {
                if (!group.isGroupConflicting(inext)) {
                    group.addTransaction(inext);
                    i++;
                }
                ++inext;
            }
            if(inext==transaction_list.end()&&iprec==transaction_list.rend()&&i!=p*a*2){
                throw domain_error("Not enough elements for cahd algorithm.");
            }
        }
        if(debug){cout<<"DEBUG:Filtering candidate list."<<endl;}
        group.filterForMostSimilarTransactions(p);
        if(debug){cout<<"DEBUG:Updating histogram."<<endl;}
        histogram->update(group.getSensitiveProductsList(),-1);
        if(debug){cout<<"DEBUG:Remaining transactions "<<transaction_list.size()<<endl;}
        if(histogram->getEntryMax() <= transaction_list.size()){
            if(iprec==transaction_list.rend()) {
                if(debug){cout<<"DEBUG:Publishing iteration results."<<endl;}
                group.publishAndRemove(transaction_list);
                result.push_back(group);
                it=transaction_list.begin();
            }else{
                ++iprec;
                if(debug){cout<<"DEBUG:Publishing iteration results."<<endl;}
                group.publishAndRemove(transaction_list);
                result.push_back(group);
                it=iprec.base();
            }
        }else{
            if(debug){cout<<"DEBUG:Warning undoing last operation."<<endl;}
            histogram->undo();
        }
    }
    //get other transactions
    if(!transaction_list.empty()){
        cout<<"LOG:Creating last group without sensitive transactions."<<endl;
        TransactionGroup group(transactionTable);
        for (auto lit = transaction_list.begin(); lit != transaction_list.end(); ++lit){
            group.addTransaction(lit);
        }
        group.publishAndRemove(transaction_list);
        result.push_back(group);
    }
    assert(!result.empty());
    assert(transaction_list.empty());
    return result;
}

Cahd::~Cahd() {
    //todo delete histogram
}

list<TransactionWrapper>::iterator Cahd::get_next_sensitive_transaction(list<TransactionWrapper>::iterator it)  {
    static int tentativi=0;
    for(;it!=transaction_list.end();++it){
        if(it->sensitive){
            tentativi=0;
            return it;
        }
    }
    assert(it==transaction_list.end());
    tentativi++;
    cout<<"LOG:No sensitive transaction found! restarting from 0. tentativo:"<<tentativi<<endl;
    return get_next_sensitive_transaction(transaction_list.begin());
}