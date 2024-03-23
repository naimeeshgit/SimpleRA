#include "global.h"

/**
 * @brief
 * SYNTAX: EXPORT <relation_name>
 */

bool syntacticParseEXPORT()
{
    INPUT_TYPE = TABLE;
    logger.log("syntacticParseEXPORT");
    if (tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX")
    {
        parsedQuery.queryType = EXPORT;
        parsedQuery.exportMatrixName = tokenizedQuery[2];
        INPUT_TYPE = MATRIX;
        return true;
    }
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = EXPORT;
    parsedQuery.exportRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParseEXPORT()
{
    logger.log("semanticParseEXPORT");
    if (INPUT_TYPE == TABLE)
    {
        // Table should exist
        if (tableCatalogue.isTable(parsedQuery.exportRelationName))
            return true;
        cout << "SEMANTIC ERROR: No such relation exists" << endl;
    }
    else if (INPUT_TYPE == MATRIX)
    {
        // Matrix should exist
        if (matrixCatalogue.isMatrix(parsedQuery.exportMatrixName))
            return true;
        cout << "SEMANTIC ERROR: No such matrix exists" << endl;
    }

    return false;
}

void executeEXPORT()
{
    logger.log("executeEXPORT");
    if (INPUT_TYPE == TABLE)
    {
        Table *table = tableCatalogue.getTable(parsedQuery.exportRelationName);
        table->makePermanent();
    }
    else if (INPUT_TYPE == MATRIX)
    {
        Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.exportMatrixName);
        matrix->makePermanent();
    }

    return;
}