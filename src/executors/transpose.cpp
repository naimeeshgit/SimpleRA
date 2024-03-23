#include "global.h"

/**
 * @brief
 * SYNTAX: TRANSPOSE MATRIX matrix_name
 */

bool syntacticParseTRANSPOSE()
{
    logger.log("syntacticParseTRANSPOSE");
    if (tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX")
    {
        parsedQuery.queryType = TRANSPOSE;
        parsedQuery.transposeMatrixName = tokenizedQuery[2];
        INPUT_TYPE = MATRIX;
        return true;
    }
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
}

bool semanticParseTRANSPOSE()
{
    logger.log("semanticParseTRANSPOSE");
    if (!matrixCatalogue.isMatrix(parsedQuery.transposeMatrixName))
    {
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }

    return true;
}

void executeTRANSPOSE()
{
    logger.log("executeTRANSPOSE");
    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.transposeMatrixName);
    matrix->transpose();
    return;
}
