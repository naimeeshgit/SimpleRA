#include "global.h"
/**
 * @brief
 * SYNTAX: PRINT relation_name
 */

bool syntacticParsePRINT()
{
    INPUT_TYPE = TABLE;
    logger.log("syntacticParsePRINT");
    if (tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX")
    {
        parsedQuery.queryType = PRINT;
        parsedQuery.printMatrixName = tokenizedQuery[2];
        INPUT_TYPE = MATRIX;
        return true;
    }
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = PRINT;
    parsedQuery.printRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParsePRINT()
{
    logger.log("semanticParsePRINT");

    if (INPUT_TYPE == MATRIX)
    {
        if (!matrixCatalogue.isMatrix(parsedQuery.printMatrixName))
        {
            cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
            return false;
        }
        return true;
    }
    else
    {
        if (!tableCatalogue.isTable(parsedQuery.printRelationName))
        {
            cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
            return false;
        }
        return true;
    }
}

void executePRINT()
{
    logger.log("executePRINT");
    if (INPUT_TYPE == MATRIX)
    {
        Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.printMatrixName);
        logger.log(parsedQuery.printMatrixName);
        matrix->print();
    }
    else
    {
        Table *table = tableCatalogue.getTable(parsedQuery.printRelationName);
        table->print();
    }

    return;
}
