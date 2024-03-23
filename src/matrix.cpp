#include "global.h"

/**
 * @brief Construct a new Matrix:: Matrix object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief Construct a new Matrix:: Matrix object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param matrixName
 */
Matrix::Matrix(string matrixName)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + matrixName + ".csv";
    this->matrixName = matrixName;
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates matrix
 * statistics.
 *
 * @return true if the matrix has been successfully loaded
 * @return false if an error occurred
 */
bool Matrix::load()
{
    logger.log("Matrix::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        this->setColumnCount(line);
        if (this->blockify())
            return true;
    }
    fin.close();
    return false;
}

/**
 * @brief Function sets column count from the first line of the .csv data
 * file.
 *
 * @param line
 */
void Matrix::setColumnCount(string firstLine)
{
    logger.log("Matrix::setColumnCount");
    int count = 0;
    string word;
    stringstream s(firstLine);
    while (getline(s, word, ','))
    {
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        count++;
    }
    this->columnCount = count;
    this->maxRowsPerBlock = SUBMATRIX_DIM;
    return;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size.
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Matrix::blockify()
{
    logger.log("Matrix::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    this->totalSubmatricesAcrossRow = (this->columnCount / SUBMATRIX_DIM + (this->columnCount % SUBMATRIX_DIM > 0));
    // vector<vector<int>> rowsInPage(this->maxRowsPerBlock, row);
    // unordered_set<int> dummy;
    // dummy.clear();
    // this->distinctValuesInColumns.assign(this->columnCount, dummy);
    // this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    int row_counter = 0;
    int pageNumber = 0;
    int startingPageNumber = 0;
    int endingPageNumber = 0;
    this->blockCount = totalSubmatricesAcrossRow * totalSubmatricesAcrossRow;
    while (getline(fin, line))
    {
        vector<vector<int>> rowInPages(this->totalSubmatricesAcrossRow);
        stringstream s(line);
        startingPageNumber = (row_counter / SUBMATRIX_DIM) * this->totalSubmatricesAcrossRow;
        endingPageNumber = (row_counter / SUBMATRIX_DIM) * this->totalSubmatricesAcrossRow + this->totalSubmatricesAcrossRow;

        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (!getline(s, word, ','))
                return false;

            pageNumber = startingPageNumber + (columnCounter / SUBMATRIX_DIM);
            rowInPages[pageNumber - startingPageNumber].push_back(stoi(word));
            // row[columnCounter] = stoi(word);
            // rowsInPage[pageCounter][columnCounter] = row[columnCounter];
        }
        // pageCounter++;
        // this->updateStatistics(row);

        // if (pageCounter == this->maxRowsPerBlock)
        // {
        //     bufferManager.writePage(this->matrixName, this->blockCount, rowsInPage, pageCounter);
        //     this->blockCount++;
        //     this->rowsPerBlockCount.emplace_back(pageCounter);
        //     pageCounter = 0;
        // }

        for (int i = startingPageNumber; i < endingPageNumber; i++)
        {
            vector<vector<int>> rows;
            rows.push_back(rowInPages[i - startingPageNumber]);
            bufferManager.writePage(this->matrixName, i, rows, 1);
        }
        row_counter++;
    }
    if (this->columnCount == 0)
        return false;
    // this->distinctValuesInColumns.clear();
    return true;
}

/**
 * @brief Given a row of values, this function will update the statistics it
 * stores i.e. it updates the number of rows that are present in the column and
 * the number of distinct values present in each column. These statistics are to
 * be used during optimisation.
 *
 * @param row
 */
void Matrix::updateStatistics(vector<int> row)
{
    this->rowCount++;
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (!this->distinctValuesInColumns[columnCounter].count(row[columnCounter]))
        {
            this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
            this->distinctValuesPerColumnCount[columnCounter]++;
        }
    }
}

void Matrix::rename(string fromMatrixName, string toMatrixName)
{
    matrixCatalogue.changeMatrixNameKey(fromMatrixName, toMatrixName);
}

/**
 * @brief Function prints the first few rows of the matrix. If the matrix contains
 * more than 20 rows, print the first 20 rows and columns only.
 */
void Matrix::print()
{
    logger.log("MATRIX::print");
    uint count = min((long long)PRINT_COUNT, this->columnCount);
    logger.log(this->matrixName);
    int startingPageNumber = 0;
    int endingPageNumber = 0;

    vector<Cursor> cursors;
    logger.log("BLOCK NUMBER: " + to_string(this->blockCount));

    for (int i = 0; i < this->blockCount; i++)
    {
        Cursor cursor(this->matrixName, i);
        cursors.push_back(cursor);
    }

    for (int rowCounter = 0; rowCounter < count; rowCounter++)
    {
        vector<int> row;
        startingPageNumber = (rowCounter / SUBMATRIX_DIM) * this->totalSubmatricesAcrossRow;
        endingPageNumber = (rowCounter / SUBMATRIX_DIM) * this->totalSubmatricesAcrossRow + this->totalSubmatricesAcrossRow;
        if (this->columnCount > 20)
            endingPageNumber = (rowCounter / SUBMATRIX_DIM) * this->totalSubmatricesAcrossRow + 20 / SUBMATRIX_DIM + (20 % SUBMATRIX_DIM > 0);

        for (int i = startingPageNumber; i < endingPageNumber; i++)
        {
            vector<int> subrow = cursors[i].getNext();
            row.insert(row.end(), subrow.begin(), subrow.end());
        }
        row.resize(count);
        this->writeRow(row, cout);
    }
    // printRowCount(this->rowCount);
}

/**
 * @brief This function returns one row of the matrix using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor
 * @return vector<int>
 */
void Matrix::getNextPage(Cursor *cursor)
{
    logger.log("Matrix::getNext");

    if (cursor->pageIndex < this->blockCount - 1)
    {
        cursor->nextPage(cursor->pageIndex + 1);
    }
}

/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Matrix::makePermanent()
{
    logger.log("Matrix::makePermanent");
    if (!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->matrixName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    uint count = this->columnCount;
    logger.log(matrixName);

    int startingPageNumber = 0;
    int endingPageNumber = 0;

    vector<Cursor> cursors;
    logger.log("BLOCK NUMBER: " + to_string(this->blockCount));

    for (int i = 0; i < this->blockCount; i++)
    {
        Cursor cursor(this->matrixName, i);
        cursors.push_back(cursor);
    }

    for (int rowCounter = 0; rowCounter < count; rowCounter++)
    {
        vector<int> row;
        startingPageNumber = (rowCounter / SUBMATRIX_DIM) * this->totalSubmatricesAcrossRow;
        endingPageNumber = (rowCounter / SUBMATRIX_DIM) * this->totalSubmatricesAcrossRow + this->totalSubmatricesAcrossRow;

        for (int i = startingPageNumber; i < endingPageNumber; i++)
        {
            vector<int> subrow = cursors[i].getNext();
            // print subrow
            //  cout<<"SUBROW: ";
            //  for(int i = 0; i < subrow.size(); i++)
            //      cout << subrow[i] << " ";
            row.insert(row.end(), subrow.begin(), subrow.end());
        }
        this->writeRow(row, fout);
    }
}

/**
 * @brief Function to check if the matrix is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Matrix::isPermanent()
{
    logger.log("Matrix::isPermanent");
    if (this->sourceFileName == "../data/" + this->matrixName + ".csv")
        return true;
    return false;
}

void Matrix::transpose()
{
    logger.log("Matrix::transpose");
    for (int i = 0; i < this->totalSubmatricesAcrossRow; i++)
    {
        for (int j = i; j < this->totalSubmatricesAcrossRow; j++)
        {
            if (i != j)
            {
                int pagenumber1 = i * this->totalSubmatricesAcrossRow + j;
                int pagenumber2 = j * this->totalSubmatricesAcrossRow + i;
                vector<vector<int>> matrix1 = bufferManager.getPage(matrixName, pagenumber1).getAllRows();
                vector<vector<int>> matrix2 = bufferManager.getPage(matrixName, pagenumber2).getAllRows();
                // transpose the matrices
                int rows = matrix1.size();
                int cols = matrix1[0].size();
                // create a transposed_matrix1
                vector<vector<int>> transposed_matrix1(cols, vector<int>(rows));
                for (int i = 0; i < rows; i++)
                    for (int j = 0; j < cols; j++)
                    {
                        transposed_matrix1[j][i] = matrix1[i][j];
                    }
                // create a transposed_matrix2
                rows = matrix2.size();
                cols = matrix2[0].size();
                vector<vector<int>> transposed_matrix2(cols, vector<int>(rows));
                for (int i = 0; i < rows; i++)
                    for (int j = 0; j < cols; j++)
                    {
                        transposed_matrix2[j][i] = matrix2[i][j];
                    }
                bufferManager.writePage(matrixName, pagenumber1, transposed_matrix2, transposed_matrix2.size());
                bufferManager.writePage(matrixName, pagenumber2, transposed_matrix1, transposed_matrix1.size());
            }
            else
            {
                int pagenumber = i * this->totalSubmatricesAcrossRow + j;
                vector<vector<int>> matrix = bufferManager.getPage(matrixName, pagenumber).getAllRows();
                // transpose the matrices
                int rows = matrix.size();
                int cols = matrix[0].size();
                // create a transposed_matrix
                vector<vector<int>> transposed_matrix(cols, vector<int>(rows));
                for (int i = 0; i < rows; i++)
                    for (int j = 0; j < cols; j++)
                    {
                        transposed_matrix[j][i] = matrix[i][j];
                    }
                bufferManager.writePage(matrixName, pagenumber, transposed_matrix, transposed_matrix.size());
            }
        }
    }
}

bool checkSubmatrixSymmetry(int pageIndex, string matrixName)
{
    // string filename = "../data/temp/" + matrixName + "_Page" + to_string(pageIndex);
    // load csv into a vector
    // vector<vector<int>> matrix = loadCSV(filename);
    vector<vector<int>> matrix = bufferManager.getPage(matrixName, pageIndex).getAllRows();

    // check if matrix is symmetric
    int rows = matrix.size();
    int cols = matrix[0].size();
    if (rows != cols)
        return false;
    for (int i = 0; i < rows; ++i)
        for (int j = i + 1; j < cols; ++j)
        {
            if (matrix[i][j] != matrix[j][i])
                return false;
        }
    return true;
}

bool Matrix::checksymmetry()
{
    // page number given submatrix index (i, j)
    // page number = i*total_matrices_across + j;
    // if i>j i.e upper triangular matrix
    // for i==j the matrices should be symmetric themselves
    // for i>j the transpose of the matrix should be equal to the corresponding matrix in the lower triangle

    bool flag = 1; // 1 if symmetric, 0 if not
    for (int i = 0; i < this->totalSubmatricesAcrossRow; i++)
    {
        for (int j = 0; j < this->totalSubmatricesAcrossRow; j++)
        {
            int pagenumber = i * this->totalSubmatricesAcrossRow + j;
            if (i == j)
            {
                bool isSymmetric = checkSubmatrixSymmetry(pagenumber, this->matrixName);
                if (!isSymmetric)
                {
                    flag = 0;
                    break;
                }
            }
            else if (i > j)
            {
                // string filename = "../data/temp/" + this->matrixName + "_Page" + to_string(pagenumber);
                // load csv into a vector
                // vector<vector<int>> matrix = loadCSV(filename);
                vector<vector<int>> matrix = bufferManager.getPage(matrixName, pagenumber).getAllRows();

                // transpose the matrix
                int rows = matrix.size();
                int cols = matrix[0].size();
                vector<vector<int>> transposed_matrix(cols, vector<int>(rows));
                for (int i = 0; i < rows; ++i)
                    for (int j = 0; j < cols; ++j)
                    {
                        transposed_matrix[j][i] = matrix[i][j];
                    }
                // check if the transpose is equal to the corresponding matrix in the lower triangle
                int new_page_number = ((pagenumber % this->totalSubmatricesAcrossRow) * this->totalSubmatricesAcrossRow) + (pagenumber / this->totalSubmatricesAcrossRow);
                // string new_filename = "../data/temp/" + this->matrixName + "_Page" + to_string(new_page_number);
                // vector<vector<int>> new_matrix = loadCSV(new_filename);
                vector<vector<int>> new_matrix = bufferManager.getPage(matrixName, new_page_number).getAllRows();

                if (transposed_matrix != new_matrix)
                {
                    flag = 0;
                    break;
                }
            }
        }
    }

    return flag;
}
/**
 * @brief Computes the expression A - A' and stores the result in A_RESULT (syntax: <matrix_name>_RESULT).
 */

void Matrix::compute(Matrix *newMatrix)
{
    logger.log("Matrix::compute");
    for (int i = 0; i < this->totalSubmatricesAcrossRow; i++)
    {
        for (int j = i; j < this->totalSubmatricesAcrossRow; j++)
        {
            if (i != j)
            {
                int pagenumber = i * this->totalSubmatricesAcrossRow + j;
                vector<vector<int>> matrix = bufferManager.getPage(matrixName, pagenumber).getAllRows();
                int opp_pagenumber = ((pagenumber % this->totalSubmatricesAcrossRow) * this->totalSubmatricesAcrossRow) + (pagenumber / this->totalSubmatricesAcrossRow);
                vector<vector<int>> opp_matrix = bufferManager.getPage(matrixName, opp_pagenumber).getAllRows();
                // transpose the opp_matrix
                int rows = opp_matrix.size();
                int cols = opp_matrix[0].size();
                vector<vector<int>> transposed_opp_matrix(cols, vector<int>(rows));
                for (int i = 0; i < rows; ++i)
                    for (int j = 0; j < cols; ++j)
                    {
                        transposed_opp_matrix[j][i] = opp_matrix[i][j];
                    }

                rows = matrix.size();
                cols = matrix[0].size();
                vector<vector<int>> result(rows, vector<int>(cols));
                for (int i = 0; i < rows; ++i)
                    for (int j = 0; j < cols; ++j)
                    {
                        result[i][j] = matrix[i][j] - transposed_opp_matrix[i][j];
                    }
                
                // do the same for the opp_matrix
                rows = matrix.size();
                cols = matrix[0].size();
                vector<vector<int>> transposed_matrix(cols, vector<int>(rows));
                for (int i = 0; i < rows; ++i)
                    for (int j = 0; j < cols; ++j)
                    {
                        transposed_matrix[j][i] = matrix[i][j];
                    }
                
                rows = opp_matrix.size();
                cols = opp_matrix[0].size();
                vector<vector<int>> opp_result(rows, vector<int>(cols));
                for (int i = 0; i < rows; ++i)
                    for (int j = 0; j < cols; ++j)
                    {
                        opp_result[i][j] = opp_matrix[i][j] - transposed_matrix[i][j];
                    }

                bufferManager.writePage(newMatrix->matrixName, pagenumber, result, matrix.size());
                bufferManager.writePage(newMatrix->matrixName, opp_pagenumber, opp_result, opp_matrix.size());
            }
            else
            {
                int pagenumber = i * this->totalSubmatricesAcrossRow + j;
                vector<vector<int>> matrix = bufferManager.getPage(matrixName, pagenumber).getAllRows();
                int opp_pagenumber = ((pagenumber % this->totalSubmatricesAcrossRow) * this->totalSubmatricesAcrossRow) + (pagenumber / this->totalSubmatricesAcrossRow);
                vector<vector<int>> opp_matrix = bufferManager.getPage(matrixName, opp_pagenumber).getAllRows();
                // transpose the opp_matrix
                int rows = opp_matrix.size();
                int cols = opp_matrix[0].size();
                vector<vector<int>> transposed_matrix(cols, vector<int>(rows));
                for (int i = 0; i < rows; ++i)
                    for (int j = 0; j < cols; ++j)
                    {
                        transposed_matrix[j][i] = opp_matrix[i][j];
                    }

                rows = matrix.size();
                cols = matrix[0].size();
                vector<vector<int>> result(rows, vector<int>(cols));
                for (int i = 0; i < rows; ++i)
                    for (int j = 0; j < cols; ++j)
                    {
                        result[i][j] = matrix[i][j] - transposed_matrix[i][j];
                    }
                bufferManager.writePage(newMatrix->matrixName, pagenumber, result, matrix.size());
            }
        }
    }
    newMatrix->totalSubmatricesAcrossRow = this->totalSubmatricesAcrossRow;
    newMatrix->blockCount = this->totalSubmatricesAcrossRow * this->totalSubmatricesAcrossRow;
    newMatrix->rowCount = this->rowCount;
    newMatrix->columnCount = this->columnCount;
    // matrixCatalogue.print();
}
/**
 * @brief The unload function removes the matrix from the database by deleting
 * all temporary files created as part of this matrix
 *
 */
void Matrix::unload()
{
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this matrix
 *
 * @return Cursor
 */
Cursor Matrix::getCursor()
{
    logger.log("Matrix::getCursor");
    Cursor cursor(this->matrixName, 0);
    return cursor;
}