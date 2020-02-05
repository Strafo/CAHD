#include "TransactionsTable.h"
#include <iostream>
#include <boost/program_options.hpp>
#include "TransactionDataAnonymizationTypes.h"
#include <cmath>

using namespace std;
using namespace tda;
namespace po = boost::program_options;

list<product_t> splitStrings(string str, char dl);
float computeActOrEst(const TransactionTable &table,const set<product_t> &mask,const product_t &sensItem,const int& tot);

/**
 * Compute kl divergence for the first item in sensitiveitems list.
 */
bool debug=false;
bool verbose=false;


product_t randProd(const vector<product_t> &v){
    int index=rand()%v.size();
    return v.at(index);
}


set<set<product_t >> limitedPowerSet(list<product_t> prodList,int cardinality){
    int maskDim;
    vector<product_t> v{prodList.begin(),prodList.end()};
    set<set<product_t>>result;
    for(int i=0;i<cardinality;++i){
        maskDim=rand()%prodList.size();
        set<product_t> toAdd;
        for(int j=0;j<maskDim;++j){
            toAdd.insert(randProd(v));
        }
        result.insert(toAdd);
    }
    return result;
}



template <class S>
void printPowerSet(const  S&res){
    // Print result
    for (auto&& subset: res) {
        std::cout << "{ ";
        char const* prefix = "";
        for (auto&& e: subset) {
            std::cout << prefix << e;
            prefix = ", ";
        }
        std::cout << " }\n";
    }
}

int main(int ac,char* av[]) {
    string anonFile, origFile;
    list<product_t> sensitiveItemsList;
    filebuf fbor, fbanon;
    int totalOccurSInOrig=0;
    int totalOccurSInAnon=0;
    int powersetdim;

    /**PARSE ARGS**/
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help,h", "produce help message")
                ("originalcsv,i", po::value<string>(), "name of the original csv file")
                ("anonymizedcsv,a", po::value<string>(), "name of the anonymized csv file")
                ("sensitiveitems,s", po::value<string>(), "sensitive items separated by -")
                ("powesetdimension,d",po::value<int>(),"number of masks")
                ;
        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            cout << desc << endl;
            return EXIT_SUCCESS;
        }
        if (vm.count("originalcsv")) { origFile = vm["originalcsv"].as<string>(); }
        else {
            cout << "Insert original csv file name." << endl;
            return EXIT_SUCCESS;
        }
        if (vm.count("anonymizedcsv")) { anonFile = vm["anonymizedcsv"].as<string>(); }
        else {
            cout << "Insert anonymized csv file name." << endl;
            return EXIT_SUCCESS;
        }
        if (vm.count("sensitiveitems")) {
            sensitiveItemsList = splitStrings(vm["sensitiveitems"].as<string>(), '-');
        } else {
            cout << "Insert sensitive items." << endl;
            return EXIT_SUCCESS;
        }
        if (vm.count("powesetdimension")) { powersetdim = vm["powesetdimension"].as<int>(); }
        else {
            cout << "Insert dimension for the subset of the powerset." << endl;
            return EXIT_SUCCESS;
        }
    } catch (exception &e) {
        cerr << "error: " << e.what() << endl;
        return EXIT_FAILURE;
    } catch (...) {
        cerr << "Exception of unknown type!" << endl;
        return EXIT_FAILURE;
    }


    /**init original table**/
    cout << "Opening file :'" << origFile << "'" << endl;
    if (!fbor.open(origFile, ios::in)) {
        cerr << "Unable to open file: '" << origFile << "'" << endl;
        exit(EXIT_SUCCESS);
    }
    istream is(&fbor);
    TransactionTable origT(origFile, is, sensitiveItemsList, false);
    //total occurences of s in T
    list<Transaction_t> transactions=origT.getTransactionList();
    for(const auto &t:transactions){
        if(origT.isPresent(t.first,sensitiveItemsList.front())){
            totalOccurSInOrig++;
        }
    }

    /**init anontable**/
    cout << "Opening file :'" << anonFile << "'" << endl;
    if (!fbanon.open(anonFile, ios::in)) {
        cerr << "Unable to open file: '" << anonFile << "'" << endl;
        exit(EXIT_SUCCESS);
    }
    istream is2(&fbanon);

    TransactionTable anonT(anonFile, is2, sensitiveItemsList, false);
    //total occurences of s in T
    transactions=anonT.getTransactionList();
    for(const auto &t:transactions){
        if(anonT.isPresent(t.first,sensitiveItemsList.front())){
            totalOccurSInAnon++;
        }
    }
    cout<<"originalTotSens "<<totalOccurSInOrig<<" anonTotSens "<<totalOccurSInAnon<<endl;
    //init C masks
    list<product_t> prodList = origT.getProductsList();
    //remove sensitive items
    for (const auto &i:sensitiveItemsList) {
        prodList.remove(i);
    }
    cout<<pow(2,prodList.size())<<endl;
    set<set<product_t>> masks = limitedPowerSet(prodList,powersetdim);
    printPowerSet(masks);
    float sum=0,actsc,estsc;
    for (const auto& subset: masks) {
        actsc=computeActOrEst(origT,subset,sensitiveItemsList.front(),totalOccurSInOrig);
        estsc=computeActOrEst(anonT,subset,sensitiveItemsList.front(),totalOccurSInAnon);
        sum+=actsc*log(actsc/estsc);
    }
    cout<<"KL_Divergence:"<<sum<<endl;
}

float computeActOrEst(const TransactionTable &table,const set<product_t> &mask,const product_t &sensItem,const int& tot){
    int counter=0;
    list<Transaction_t> l=table.subTableFromMask(mask);
    for(const auto &i:l){
        if(table.isPresent(i.first,sensItem)){
            ++counter;
        }
    }
    return (float)counter/(float)tot;
}




list<product_t> splitStrings(string str, char dl){
    string word = "";
    int num = 0;
    str = str + dl;
    int l = str.size();
    list<product_t> substr_list;
    for (int i = 0; i < l; i++) {
        if (str[i] != dl)
            word = word + str[i];
        else {
            if ((int)word.size() != 0)
                substr_list.push_back(word);
            word = "";
        }
    }
    return substr_list;
}

