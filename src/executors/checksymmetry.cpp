#include "global.h"

/**
 * @brief
 * SYNTAX: CHECKSYMMETRY matrix_name
 */

bool syntacticParseCHECKSYMMETRY()
{
    logger.log("syntacticParseCHECKSYMMETRY");
    if (tokenizedQuery.size() == 2)
    {
        parsedQuery.queryType = CHECKSYMMETRY;
        parsedQuery.checksymmetryMatrixName = tokenizedQuery[1];
        INPUT_TYPE = MATRIX;
        return true;
    }
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
}

bool semanticParseCHECKSYMMETRY()
{
    logger.log("semanticParseCHECKSYMMETRY");
    if (!matrixCatalogue.isMatrix(parsedQuery.checksymmetryMatrixName))
    {
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }

    return true;
}

void executeCHECKSYMMETRY()
{
    logger.log("executeCHECKSYMMETRY");
    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.checksymmetryMatrixName);
    int isSymmetric = matrix->checksymmetry();
    if (isSymmetric)
        cout << "TRUE" << endl;
    else
        cout << "FALSE" << endl;
    return;
}
