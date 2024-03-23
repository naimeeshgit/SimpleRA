#include "global.h"
/**
 * @brief
 * SYNTAX: LOAD relation_name
 */

bool syntacticParseLOAD()
{
    INPUT_TYPE = TABLE;
    logger.log("syntacticParseLOAD");
    if (tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX")
    {
        parsedQuery.queryType = LOAD;
        parsedQuery.loadRelationName = tokenizedQuery[2];
        parsedQuery.loadMatrixName = tokenizedQuery[2];
        INPUT_TYPE = MATRIX;
        return true;
    }
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = LOAD;
    parsedQuery.loadRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParseLOAD()
{
    logger.log("semanticParseLOAD");
    if (INPUT_TYPE)
    {
        // matrixCatalogue.print();
        if (matrixCatalogue.isMatrix(parsedQuery.loadMatrixName))
        {
            cout << "SEMANTIC ERROR: Matrix already exists" << endl;
            return false;
        }
        if (!isFileExists(parsedQuery.loadMatrixName))
        {
            cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
            return false;
        }
        return true;
    }
    else
    {
        if (tableCatalogue.isTable(parsedQuery.loadRelationName))
        {
            cout << "SEMANTIC ERROR: Relation already exists" << endl;
            return false;
        }

        if (!isFileExists(parsedQuery.loadRelationName))
        {
            cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
            return false;
        }
        return true;
    }
}

void executeLOAD()
{
    logger.log("executeLOAD");

    if (INPUT_TYPE)
    {
        Matrix *matrix = new Matrix(parsedQuery.loadMatrixName);
        if (matrix->load())
        {
            matrixCatalogue.insertMatrix(matrix);
            cout << "Loaded Matrix. Column Count: " << matrix->columnCount << " Row Count: " << matrix->columnCount << endl;
        }
    }
    else
    {
        Table *table = new Table(parsedQuery.loadRelationName);
        if (table->load())
        {
            tableCatalogue.insertTable(table);
            cout << "Loaded Table. Column Count: " << table->columnCount << " Row Count: " << table->rowCount << endl;
        }
    }
    return;
}