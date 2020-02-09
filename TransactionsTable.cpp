#include "TransactionsTable.h"

using namespace std;
using namespace tda;

void reset_file(istream&in);
tda::invoice_t find_key_in_map(const map<tda::product_t,position_t> &m,const position_t &val);
RowMatrix create_permutation_matrix(long size,const vector<position_t> &perm);

/**CONSTRUCTORS**/

TransactionTable::TransactionTable():
                        products_position_map(),invoice_position_map(),
                        sensitive_items_set(),max_transaction_size(0),
                        n_fake_entries(0),matrix(nullptr){}

TransactionTable::TransactionTable(const string &file_name, istream &in,const list<product_t> &sensitive_items,bool squared):TransactionTable()
{
    init_products_invoices_maps(file_name, in);
    reset_file(in);
    init_transaction_matrix(file_name,in,squared);
    init_sensitive_item_set(sensitive_items);
}

/**CONSTRUCTOR HELPERS**/

void TransactionTable::init_products_invoices_maps(const string &file_name,  istream &in){
    using namespace io;
    product_t stock_code;
    invoice_t invoice_no;
    map<invoice_t, unsigned int> row_dim_counter;
    pair<map<string,unsigned int>::iterator,bool> ret;
    map<string, unsigned int>::iterator it;

    CSVReader<2, trim_chars<' ', '\t'>, double_quote_escape<',', '"'>> csvReader(file_name, in);
    csvReader.read_header(ignore_extra_column, "InvoiceNo","StockCode");

    position_t prod_pos=0,invoice_pos=0;
    while (csvReader.read_row( invoice_no,stock_code)) {
        assert(!stock_code.empty()&&!invoice_no.empty());
        if((products_position_map.insert(pair<product_t,position_t>(stock_code,prod_pos))).second){
            prod_pos++;
        }
        if((invoice_position_map.insert(pair<invoice_t,position_t>(invoice_no,invoice_pos))).second){
            invoice_pos++;
        }
        //Cerco la transazione con il numero di colonne maggiore in modo poi
        //da riservare il numero giusto di colenne nella matrice sparsa.
        ret = row_dim_counter.insert (pair<invoice_t,unsigned int>(invoice_no,1));
        if (!ret.second) {
            //element found...
            ret.first->second+=1;
        }
    }
    max_transaction_size = 0;
    invoice_no = "ERROR";
    for ( it = row_dim_counter.begin(); it != row_dim_counter.end(); it++ )
    {
        if(it->second>max_transaction_size){
            max_transaction_size = it->second;
            invoice_no = it->first; // to print later the max size invoice
        }
    }
}

void TransactionTable::init_transaction_matrix(const string &file_name, istream &in,bool squared){
    using namespace io;
    string stock_code,invoice_no;
    position_t row,col;
    CSVReader<2, trim_chars<' ', '\t'>, double_quote_escape<',', '"'>> csvReader(file_name, in);
    if(squared){create_fake_entries();}
    csvReader.read_header(ignore_extra_column, "StockCode", "InvoiceNo");
    matrix = new RowMatrix (invoice_position_map.size(),products_position_map.size());
    matrix->reserve(Eigen::VectorXi::Constant(invoice_position_map.size(),max_transaction_size));
    while (csvReader.read_row(stock_code, invoice_no)) {
        row = invoice_position_map.at(invoice_no);
        col = products_position_map.at(stock_code);
        MatrixCellType old_value= matrix->coeff(row,col);
        if(old_value==0){ // todo is really necessary?
            matrix->insert(row,col)=1;
        }
    }
}

void TransactionTable::create_fake_entries(){
    string stock_code_fake;
    position_t offset = products_position_map.size();
    assert(invoice_position_map.size()>=products_position_map.size());
    unsigned long pad_dim = invoice_position_map.size() - products_position_map.size();
    for(unsigned long i = 0 ; i < pad_dim ; ++i){
        stock_code_fake="FAKE_"+to_string(offset + i);
        assert((products_position_map.insert(pair<string,position_t>( stock_code_fake, offset + i))).second);
    }
    n_fake_entries = pad_dim;
}

void TransactionTable::init_sensitive_item_set(const list<product_t> &sensitive_items){
    for (auto const& i : sensitive_items)
        sensitive_items_set.insert(i);
    for(auto const& i:sensitive_items_set)
        assert(products_position_map.find(i)!=products_position_map.end());
}


/** PRIVATE METHODS**/
void TransactionTable::permuteMatrix(const std::vector<position_t> &permutation) {
    RowMatrix p_m,p_m_transpose;
    auto *band_matrix = new RowMatrix (matrix->rows(),matrix->cols());
    p_m=create_permutation_matrix(matrix->rows(),permutation);
    p_m_transpose= p_m.transpose();
    *band_matrix =p_m*(*matrix)*p_m_transpose;
    delete matrix;
    matrix=band_matrix;
}

void TransactionTable::permuteMaps(const vector<position_t> &permutation){
    std::map<invoice_t , position_t >::iterator it;
    std::map<product_t , position_t >::iterator it2;
    position_t curr=0;
    map<string,position_t> temp_prod(products_position_map);
    map<string,position_t> temp_invo(invoice_position_map);
    string invo_key,prod_key;
    assert(products_position_map.size()==invoice_position_map.size());
    for (const position_t &i : permutation) {
        invo_key=find_key_in_map(temp_invo,i);
        prod_key=find_key_in_map(temp_prod,i);
        it = invoice_position_map.find(invo_key);
        assert(it != invoice_position_map.end());
        it->second = curr;
        it2 = products_position_map.find(prod_key);
        assert(it2 != products_position_map.end());
        it2->second = curr;
        curr++;
    }
}

/**PUBLIC METHODS**/
bool TransactionTable::isSensitive(const Transaction_t& transaction) const{
    position_t sens_pos;
    for(auto const& i:sensitive_items_set) {
        sens_pos = products_position_map.at(i);
        // if transaction contains at least one sensitive item -> sensitive transaction
        if(matrix->coeff(transaction.second,sens_pos)!=0){
            return true;
        }
    }
    return false;
}

bool TransactionTable::isSensitive(const tda::Transaction_t &transaction, const tda::product_t &sp) const {
    assert(sensitive_items_set.find(sp)!=sensitive_items_set.end());
    return matrix->coeff(transaction.second,products_position_map.at(sp));
}



bool TransactionTable::areConflicting(const Transaction_t &t1,const Transaction_t &t2) const {
    position_t sens_pos;
    for(auto const& i:sensitive_items_set) {
        sens_pos = products_position_map.at(i);
        if(matrix->coeff(t1.second,sens_pos)&&matrix->coeff(t2.second,sens_pos)){
            return true;
        }
    }
    return false;
}

TransactionTable TransactionTable::createSymmetricMatrix() const {
    TransactionTable res;
    res.matrix = new RowMatrix (matrix->rows(),matrix->cols());
    RowMatrix transposed_m = matrix->transpose();
    *res.matrix=matrix->operator+(transposed_m);
    return res;
}

void TransactionTable::permuteTransactionMatrix(const std::vector<position_t> &permutation) {
    //assert(products_position_map.size()==330);
    permuteMatrix(permutation);
    permuteMaps(permutation);
}

/*
void TransactionTable::permuteMatrix(const std::vector<position_t> &permutation) {
    Eigen::VectorXi perm_vec(permutation.size());
    for (unsigned int i=0;i<permutation.size();++i){
        perm_vec[i]=permutation[i];
    }
    *matrix=matrix->twistedBy(perm_vec.asPermutation());
}*/

//compare only NON-sensitive attributes
double TransactionTable::compareTransactionSimilarity(const tda::Transaction_t &original, const Transaction_t &t2) const {
    const static double tot=(double)products_position_map.size()-sensitive_items_set.size();
    double common=0.0;
    for(const auto &p:products_position_map){
        if(sensitive_items_set.find(p.first) == sensitive_items_set.end()){//non conto i prodotti sensibili
            if(matrix->coeff(original.second,p.second)==matrix->coeff(t2.second,p.second)){
                common++;
            }
        }
    }
    return common/tot ;
}

list<Transaction_t> TransactionTable::subTableFromMask(const set<product_t> &productsMask) const{
    list<Transaction_t> list;
    for (const auto &t : invoice_position_map){
        if(mask(productsMask,t)){
            list.emplace_back(t);
        }
    }
    return list;
}

bool TransactionTable::mask(const std::set<tda::product_t> &productsMask,const Transaction_t &transaction) const {
    bool isInMask;
    for(const auto &p:products_position_map){
        if(sensitive_items_set.find(p.first)!=sensitive_items_set.end()){
            continue;//skip sensitive items
        }
        isInMask=productsMask.find(p.first)!=productsMask.end();
        if(isInMask!=isPresent(transaction.first,p.first)){
            return false;
        }
    }
    return true;
}

std::ofstream TransactionTable::createAnonymizedTransactionFile(const std::string &fileName){
    ofstream output;
    output.open(fileName);
    output<<"InvoiceNo,StockCode"<<endl;
    return output;
}


void TransactionTable::addTransactionToAnonymizedFile(const tda::Transaction_t& toAnonymize,
        const std::forward_list<tda::product_t> &sensItems,std::ofstream &outputFile) const {
    for(const auto &p:products_position_map){
        auto findIter = find(sensItems.begin(), sensItems.end(),p.first);
        if(findIter==sensItems.end()){//non sensibile
            if(matrix->coeff(toAnonymize.second,p.second)){
                outputFile<<toAnonymize.first<<","<<p.first<<endl;
            }
        }else{//sensibile
            outputFile<<toAnonymize.first<<","<<p.first<<endl;
        }
    }

}


/**  GETTER  **/

bool TransactionTable::isPresent(const invoice_t &invo,const product_t &prod) const {
    return matrix->coeff(invoice_position_map.at(invo),products_position_map.at(prod));
}

position_t TransactionTable::getNumberOfColumns() const {
    return matrix->cols();
}

position_t TransactionTable::getNumberOfRows() const {
    return matrix->rows();
}

unsigned int TransactionTable::getNumberOfFakeEntries() const {
    return n_fake_entries;
}

unsigned int TransactionTable::getMaxTransactionSize() const {
    return max_transaction_size;
}

Transaction_t TransactionTable::getNextSensitiveTransaction(const Transaction_t & offset)  const {
    position_t pos_offset;
    Transaction_t curr;
    pos_offset = offset.first.empty()?  -1 : invoice_position_map.at(offset.first);
    for(position_t r=pos_offset+1;r<matrix->rows();++r) {
        curr=make_pair(find_key_in_map(invoice_position_map,r),r);
        if(isSensitive(curr)) {
            return curr;
        }
    }
    throw out_of_range("No sensitive transaction found.");
}

Transaction_t TransactionTable::getTransaction(const invoice_t &invoice) const {
    Transaction_t res;
    res.first=invoice;
    res.second=invoice_position_map.at(invoice);
    return res;
}

Transaction_t TransactionTable::getTransaction(const position_t &position) const {
    Transaction_t res;
    res.first=find_key_in_map(invoice_position_map,position);
    res.second=position;
    return res;
}

vector<pair<position_t, position_t>> TransactionTable::getNonZeroEntriesCordinates() const {
    vector<pair<position_t, position_t>> v;
    v.resize(matrix->nonZeros());
    for ( Eigen::Index i = 0; i < matrix->outerSize(); ++i ) {
        for (RowMatrix::InnerIterator it(*matrix, i); it; ++it) {
            v.emplace_back(it.row(),it.col());
        }
    }
    return v;
}


/*
const map<std::string, position_t> &TransactionTable::getProductsPositionMap() const {
    return products_position_map;
}*/

const set<product_t> &TransactionTable::getSensitiveItemsSet() const {
    return sensitive_items_set;
}

list<Transaction_t> TransactionTable::getTransactionList() const {
    list<Transaction_t> l;
    for(const auto &item:invoice_position_map){
        l.emplace_back(item.first,item.second);
    }
    return l;
}

std::list<tda::product_t> TransactionTable::getSensitiveItemInTransaction(const Transaction_t &t) const {
    position_t sens_pos;
    list<product_t> sensProds;
    for(auto const& i:sensitive_items_set) {
        sens_pos = products_position_map.at(i);
        // if transaction contains at least one sensitive item -> sensitive transaction
        if(matrix->coeff(t.second,sens_pos)!=0){
            sensProds.push_back(i);
        }
    }
    return sensProds;
}

list<invoice_t> TransactionTable::getInvoicesList() const {
    list<invoice_t> l;
    for(const auto &i : invoice_position_map){
        l.push_back(i.first);
    }
    return l;
}

list<product_t> TransactionTable::getProductsList() const {
    list<product_t> l;
    for(const auto &i : products_position_map){
        l.push_back(i.first);
    }
    return l;
}

/**PRINT METHODS**/
void TransactionTable::plotTransactionMatrix() {
    vector<long> vx,vy;
    position_t col,row;
    for ( Eigen::Index i = 0; i < matrix->outerSize(); ++i ){
        for (RowMatrix::InnerIterator it(*matrix,i); it; ++it){
            col=it.col();
            row=it.row();
            vx.push_back( col);
            vy.push_back( row);

        }
    }
    matplotlibcpp::figure();
    matplotlibcpp::xlabel("product number ");
    matplotlibcpp::ylabel("invoice number");
    matplotlibcpp::scatter(vx,vy);
    matplotlibcpp::draw();
    matplotlibcpp::pause(0.001);
}



/** UTIL FUNCTIONS**/
void reset_file(istream&in){
    //riporta stream all'inizio del file
    in.clear();
    in.seekg(0, ios::beg);
}

invoice_t find_key_in_map(const map<invoice_t ,position_t> &m,const position_t &val){
    for(const auto & it : m){
        if (it.second == val) {
            return it.first;
        }
    }
    throw logic_error("Unexpected error. <find_key_in_map()>");
}

RowMatrix create_permutation_matrix(long size,const vector<position_t> &perm){
    RowMatrix res(size,size);
    long j=0;
    res.reserve(size);
    for (position_t i : perm) {
        res.insert(j++,i)=1;
    }
    return res;
}
