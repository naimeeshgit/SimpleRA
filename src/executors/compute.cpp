#include "global.h"

/**
 * @brief
 * SYNTAX: COMPUTE matrix_name
 */

bool syntacticParseCOMPUTE()
{
    logger.log("syntacticParseCOMPUTE");
    if (tokenizedQuery.size() == 2)
    {
        parsedQuery.queryType = COMPUTE;
        parsedQuery.computeMatrixName = tokenizedQuery[1];
        INPUT_TYPE = MATRIX;
        return true;
    }
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
}

bool semanticParseCOMPUTE()
{
    logger.log("semanticParseCOMPUTE");
    if (!matrixCatalogue.isMatrix(parsedQuery.computeMatrixName))
    {
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }

    return true;
}

void executeCOMPUTE()
{
    logger.log("executeCOMPUTE");
    string new_matrix_name = parsedQuery.computeMatrixName + "_RESULT";
    Matrix *new_matrix = new Matrix(new_matrix_name);
    matrixCatalogue.insertMatrix(new_matrix);

    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.computeMatrixName);
    matrix->compute(new_matrix);
    return;
}
