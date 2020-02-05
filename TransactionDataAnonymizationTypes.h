#ifndef TRANSACTIONDATAANONYMIZATIONTYPES_H
#define TRANSACTIONDATAANONYMIZATIONTYPES_H

#include <eigen3/Eigen/Sparse>

namespace tda{
    typedef std::string invoice_t;
    typedef std::string product_t;
    typedef Eigen::Index position_t;
    typedef short MatrixCellType;// todo check short
    typedef Eigen::SparseMatrix<MatrixCellType, Eigen::RowMajor> RowMatrix;
    typedef std::pair<invoice_t , position_t> Transaction_t;

    static_assert(((position_t)-1)+1==0, "position_t type can't handle negative value.");
}



#endif //TRANSACTIONDATAANONYMIZATIONTYPES_H
