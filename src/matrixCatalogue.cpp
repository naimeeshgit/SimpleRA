#include "global.h"

void MatrixCatalogue::insertMatrix(Matrix *matrix)
{
    logger.log("MatrixCatalogue::~insertMatrix");
    this->matrices[matrix->matrixName] = matrix;
}
void MatrixCatalogue::deleteMatrix(string matrixName)
{
    logger.log("MatrixCatalogue::deleteMatrix");
    this->matrices[matrixName]->unload();
    delete this->matrices[matrixName];
    this->matrices.erase(matrixName);
}
Matrix *MatrixCatalogue::getMatrix(string matrixName)
{
    logger.log("MatrixCatalogue::getMatrix");
    Matrix *matrix = this->matrices[matrixName];
    return matrix;
}
bool MatrixCatalogue::isMatrix(string matrixName)
{
    logger.log("MatrixCatalogue::isMatrix");
    if (this->matrices.count(matrixName))
        return true;
    return false;
}

// bool MatrixCatalogue::isColumnFromMatrix(string columnName, string matrixName)
// {
//     logger.log("MatrixCatalogue::isColumnFromMatrix");
//     if (this->isMatrix(matrixName))
//     {
//         Matrix* matrix = this->getMatrix(matrixName);
//         if (matrix->isColumn(columnName))
//             return true;
//     }
//     return false;
// }

void MatrixCatalogue::print()
{
    logger.log("MatrixCatalogue::print");
    cout << "\nMATRICES" << endl;

    int rowCount = 0;
    for (auto matrix : this->matrices)
    {
        cout << matrix.first << " " << matrix.second << endl;
        rowCount++;
    }
    printRowCount(rowCount);
}

void MatrixCatalogue::changeMatrixNameKey(string fromMatrixName, string toMatrixName)
{
    logger.log("MatrixCatalogue::changeMatrixNameKey");
    auto it = this->matrices.find(fromMatrixName);
    if (it != this->matrices.end())
    {
        this->matrices[toMatrixName] = it->second;
        this->matrices.erase(it);
        this->matrices[toMatrixName]->matrixName = toMatrixName;
        this->matrices[toMatrixName]->sourceFileName = "../data/temp/" + toMatrixName;
    }

    int flag = 1;
    int pageIndex = 0;
    while (flag)
    {
        string oldpagename = "../data/temp/" + fromMatrixName + "_Page" + to_string(pageIndex);
        string newpagename = "../data/temp/" + toMatrixName + "_Page" + to_string(pageIndex);
        std::error_code ec;
        std::filesystem::rename(oldpagename, newpagename, ec);
        if (!ec)
        {
            logger.log("File renamed successfully.");
            pageIndex++;
        }
        else
        {
            logger.log("Error renaming file: ");
            logger.log(ec.message());
            flag = 0;
        }
    }
}

MatrixCatalogue::~MatrixCatalogue()
{
    logger.log("MatrixCatalogue::~MatrixCatalogue");
    for (auto matrix : this->matrices)
    {
        matrix.second->unload();
        delete matrix.second;
    }
}