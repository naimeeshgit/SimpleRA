#ifndef CURSOR_H
#define CURSOR_H
#endif
#include <unordered_set>
#include <sstream>

// enum IndexingStrategy
// {
//     BTREE,
//     HASH,
//     NOTHING
// };

/**
 * @brief The Matrix class holds all information related to a loaded matrix. It
 * also implements methods that interact with the parsers, executors, cursors
 * and the buffer manager. There are typically 2 ways a matrix object gets
 * created through the course of the workflow - the first is by using the LOAD
 * command and the second is to use assignment statements (COMPUTE). 
 *
 */
class Matrix
{
    vector<unordered_set<int>> distinctValuesInColumns;

public:
    string sourceFileName = "";
    string matrixName = "";
    // vector<string> columns;
    vector<uint> distinctValuesPerColumnCount;
    long long int columnCount = 0;
    long long int rowCount = 0;
    uint blockCount = 0;
    uint maxRowsPerBlock = 0;
    uint totalSubmatricesAcrossRow = 0;
    // vector<uint> rowsPerBlockCount;
    bool indexed = false;
    string indexedColumn = "";
    // IndexingStrategy indexingStrategy = NOTHING;
    
    void setColumnCount(string firstLine);
    bool blockify();
    void updateStatistics(vector<int> row);
    Matrix();
    Matrix(string matrixName);
    // Matrix(string tableName, vector<string> columns);
    bool load();
    // bool isColumn(string columnName);
    void rename(string fromMatrixName, string toMatrixName);
    void print();
    void makePermanent();
    bool isPermanent();
    void getNextPage(Cursor *cursor);
    Cursor getCursor();
    // int getColumnIndex(string columnName);
    void unload();
    void transpose();
    bool checksymmetry();
    void compute(Matrix* newMatrix);

    /**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row 
 */
template <typename T>
// EXPORT
void writeRow(vector<T> row, ostream &fout)
{
    logger.log("Matrix::printRow");
    for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
    {
        if (columnCounter != 0)
            fout << ", ";
        fout << row[columnCounter];
    }
    fout << endl;
}

/**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row 
 */
template <typename T>
void writeRow(vector<T> row)
{
    logger.log("Table::printRow");
    ofstream fout(this->sourceFileName, ios::app);
    this->writeRow(row, fout);
    fout.close();
}
};