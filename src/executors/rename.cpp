#include "global.h"
/**
 * @brief
 * SYNTAX: RENAME column_name TO column_name FROM relation_name
 */
bool syntacticParseRENAME()
{
    INPUT_TYPE = TABLE;
    logger.log("syntacticParseRENAME");
    if (tokenizedQuery.size() == 4 && tokenizedQuery[1] == "MATRIX")
    {
        INPUT_TYPE = MATRIX;
        parsedQuery.queryType = RENAME;
        parsedQuery.renameFromMatrixName = tokenizedQuery[2];
        parsedQuery.renameToMatrixName = tokenizedQuery[3];
        return true;
    }
    if (tokenizedQuery.size() != 6 || tokenizedQuery[2] != "TO" || tokenizedQuery[4] != "FROM")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = RENAME;
    parsedQuery.renameFromColumnName = tokenizedQuery[1];
    parsedQuery.renameToColumnName = tokenizedQuery[3];
    parsedQuery.renameRelationName = tokenizedQuery[5];
    return true;
}

bool semanticParseRENAME()
{
    logger.log("semanticParseRENAME");

    if (INPUT_TYPE == TABLE)
    {
        if (!tableCatalogue.isTable(parsedQuery.renameRelationName))
        {
            cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
            return false;
        }

        if (!tableCatalogue.isColumnFromTable(parsedQuery.renameFromColumnName, parsedQuery.renameRelationName))
        {
            cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
            return false;
        }

        if (tableCatalogue.isColumnFromTable(parsedQuery.renameToColumnName, parsedQuery.renameRelationName))
        {
            cout << "SEMANTIC ERROR: Column with name already exists" << endl;
            return false;
        }
        return true;
    }
    else
    {
        if (!matrixCatalogue.isMatrix(parsedQuery.renameFromMatrixName))
        {
            cout << "SEMANTIC ERROR: Matrix " << parsedQuery.renameFromMatrixName << " doesn't exist" << endl;
            return false;
        }

        if (matrixCatalogue.isMatrix(parsedQuery.renameToMatrixName))
        {
            cout << "SEMANTIC ERROR: Matrix" << parsedQuery.renameToMatrixName << " already exists" << endl;
            return false;
        }

        return true;
    }
}

void executeRENAME()
{
    logger.log("executeRENAME");
    if (INPUT_TYPE == TABLE)
    {
        Table *table = tableCatalogue.getTable(parsedQuery.renameRelationName);
        table->renameColumn(parsedQuery.renameFromColumnName, parsedQuery.renameToColumnName);
    }
    else
    {
        Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.renameFromMatrixName);
        matrix->rename(parsedQuery.renameFromMatrixName, parsedQuery.renameToMatrixName);
    }

    return;
}