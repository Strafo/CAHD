#include "TransactionsTable.h"
#include <iostream>
#include <boost/graph/cuthill_mckee_ordering.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bandwidth.hpp>
#include <vector>
#include "Cahd.h"
#include <boost/program_options.hpp>

typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, boost::property<boost::vertex_color_t, boost::default_color_type,boost::property<boost::vertex_degree_t,tda::MatrixCellType> > > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef boost::graph_traits<Graph>::vertices_size_type size_type;
namespace po = boost::program_options;


using namespace std;
using namespace tda;

Graph create_graph(const TransactionTable &t );
bool check_permutation(unsigned int n_vertex, vector<Vertex> &inv_perm, const boost::property_map<Graph, boost::vertex_index_t>::type &index_map);
list<product_t> splitStrings(string str, char dl);

bool debug;
bool verbose;

int main(int ac,char* av[]){
    string file_name,fileOutputName;
    list<product_t> sensitive_items_list;
    short privacy_degree,alpha;
    filebuf fb;
    vector<position_t> inverse_permutation;

    /**PARSE ARGS**/
    try{
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help,h", "produce help message")
                ("input,i", po::value<string>(), "name of the input file")
                ("output,o", po::value<string>(), "name of the anonymized output file")
                ("privacy,p", po::value<short>(), "privacy degree")
                ("alpha,a", po::value<short>(), "search width parameter")
                ("sensitiveitems,s", po::value<string>(), "sensitive items separated by -")
                ("verbose,v",po::bool_switch(&verbose),"display additional information")
                ("debug,d",po::bool_switch(&debug),"display debug information")
                ;
        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {cout << desc << endl;return EXIT_SUCCESS;}
        if (vm.count("input")) {file_name= vm["input"].as<string>();}
        else {cout << "Insert input file name."<<endl;return EXIT_SUCCESS;}
        if (vm.count("privacy")) {privacy_degree= vm["privacy"].as<short>();}
        else {cout << "Insert privacy degree."<<endl;return EXIT_SUCCESS;}
        if (vm.count("alpha")) {alpha= vm["alpha"].as<short>();}
        else {cout << "Insert alpha degree."<<endl;return EXIT_SUCCESS;}
        if (vm.count("output")) {fileOutputName= vm["output"].as<string>();}
        else {fileOutputName= "output.csv";cout<<"LOG:No file name output provided. Writing result to "<<fileOutputName<<endl;}
        if (vm.count("sensitiveitems")) {
            sensitive_items_list=splitStrings(vm["sensitiveitems"].as<string>(),'-');
        }
        else {cout << "Insert sensitive items."<<endl;return EXIT_SUCCESS;}
    }catch(exception &e){
        cerr << "error: " << e.what() << endl;return EXIT_FAILURE;
    }catch(...){
        cerr << "Exception of unknown type!"<<endl;return EXIT_FAILURE;
    }
    /**ALGO**/
    auto t1 = std::chrono::high_resolution_clock::now();
    cout<<"Opening file :'"<<file_name<<"'"<<endl;
    if (!fb.open (file_name,ios::in))
    {
        cerr<<"Unable to open file: '"<<file_name<<"'"<<endl;
        exit(0);
    }
    istream is(&fb);
    cout<<"LOG:Creating matrix..."<<endl;
    /**Creates the sparse matrix from input file**/
    TransactionTable t(file_name,is,sensitive_items_list,true);
    cout<<"LOG:Transaction matrix created with size : "<<t.getNumberOfRows()<<" X ("<<t.getNumberOfColumns()-t.getNumberOfFakeEntries()<<" + fake :"<<t.getNumberOfFakeEntries()<<")"<<endl;
    fb.close();
    assert(t.getNumberOfRows()==t.getNumberOfColumns());
    if(verbose){t.plotTransactionMatrix();}
    /**Creates the symmetric matrix S=A+A_transposed**/
    cout<<"LOG:Creating symmetric matrix. Calculating A+A_T..."<<endl;
    TransactionTable sym_m=t.createSymmetricMatrix();
    cout<<"LOG:Symmetric matrix created with size : "<<t.getNumberOfRows()<<" X "<<t.getNumberOfColumns()<<endl;
    /**Creates the graph for rcm from symmetric matrix interpreted as adjacency matrix**/
    Graph g = create_graph(sym_m);
    cout<<"LOG:Original bandwidth: " << boost::bandwidth(g) << endl;
    /**RCM**/
    boost::property_map<Graph, boost::vertex_index_t>::type index_map = get(boost::vertex_index, g);
    vector<Vertex> inv_perm(num_vertices(g));
    vector<size_type> perm(num_vertices(g));
    cout <<"LOG:Reverse Cuthill-McKee ordering..." << endl;
    boost::cuthill_mckee_ordering(g,inv_perm.rbegin(), get(boost::vertex_color, g),make_degree_map(g));
    assert(check_permutation(sym_m.getNumberOfRows(),inv_perm,index_map));
    for (unsigned long i : inv_perm) {
        inverse_permutation.push_back(index_map[i]);
    }
    for (size_type c = 0; c != inv_perm.size(); ++c)
        perm[index_map[inv_perm[c]]] = c;
    cout<<"LOG:New bandwidth: "<< boost::bandwidth(g, make_iterator_property_map(&perm[0], index_map, perm[0]))<<endl;
    /**Permutes the matrix to obtain the band matrix**/
    cout<<"LOG:Permuting TransactionMatrix..."<<endl;
    t.permuteTransactionMatrix(inverse_permutation);
    if(verbose){t.plotTransactionMatrix();}
    list<product_t> sensItemsList;
    sensItemsList.assign(t.getSensitiveItemsSet().begin(),t.getSensitiveItemsSet().end());
    /**Cahd algorithm**/
    cout<<"LOG:Starting cahd algorithm..."<<endl;
    Cahd cahd(t,t.getTransactionList(),sensItemsList);
    list<TransactionGroup> anonymizedTransactions=cahd.cahdGroupFormationEuristic(privacy_degree,alpha);
    /**Writing results**/
    cout<<"LOG:Writing results on "<<fileOutputName<<"..."<<endl;
    ofstream ofstream1=TransactionTable::createAnonymizedTransactionFile(fileOutputName);
    assert(!anonymizedTransactions.empty());
    for(const auto &i:anonymizedTransactions){
        auto sensItems=i.getSensitiveProductsListPublished();
        assert(!i.published.empty());
        for(const auto &j:i.published){
            t.addTransactionToAnonymizedFile(j.transaction,sensItems,ofstream1);
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::duration<double>>( t2 - t1 );
    cout<<"LOG:Completed! execution time:";
    cout << time.count()<<endl<<"Insert any key to continue...";
    cin.get();
    cin.get();
    return EXIT_SUCCESS;
}

/**
 * Create a boost undirected graph from adjacency matrix
 */
Graph create_graph(const TransactionTable &t ){
    float rows=t.getNumberOfRows();
    float r=0;
    Graph g(rows);
    short perc,prev=-1;
    string to_display="0%";
    vector<pair<position_t,position_t>> vector=t.getNonZeroEntriesCordinates();
    cout<<"LOG:Creating graph:"<<to_display<<flush;
    for(const auto &i:vector){
        boost::add_edge(i.first,i.second,g);
        perc=(short)(r/rows);
        if(perc!=prev) {
            cout << string(to_display.length(), '\b');
            to_display = to_string(perc) + "%";
            cout << to_display<<flush;
            prev=perc;
        }
        r++;
    }
    cout << string(to_display.length(), '\b');
    to_display ="100%\n";
    cout << to_display<<flush;
    return g;
}


bool check_permutation(unsigned int n_vertex, vector<Vertex> &inv_perm, const boost::property_map<Graph, boost::vertex_index_t>::type &index_map){
    vector<unsigned int> v;
    v.reserve(n_vertex);
    if(n_vertex!=inv_perm.size()){return false;}
    for (unsigned long i : inv_perm) {
        v.push_back(index_map[i]);
    }
    sort(v.begin(),v.end());
    for(unsigned int i=0;i<n_vertex;i++){
        if(i!=v[i])
            return false;
    }
    return true;
}

list<product_t> splitStrings(string str, char dl)
{
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
