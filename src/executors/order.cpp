#include "global.h"
/**
 * @brief File contains method to process SORT commands.
 *
 * syntax:
 * R <- ORDER BY column_name sorting_order ON relation_name
 *
 * sorting_order = ASC | DESC
 */
bool syntacticParseORDER()
{
    parsedQuery.sortColumnNames.clear();
    parsedQuery.sortingStrategies.clear();
    logger.log("syntacticParseORDER");
    // command syntax: R <- ORDER BY <column_name1, column_name2,..., column_namek> <ASC|DESC, ASC|DESC,..., ASC|DESC> ON <relation_name>
    parsedQuery.queryType = ORDER;
    parsedQuery.orderResultRelationName = tokenizedQuery[0];
    if (tokenizedQuery[1] != "<-")
    {
        cout << "SYNTAX ERROR: Expected <- assignment operator keyword at position 2" << endl;
        return false;
    }
    if (tokenizedQuery[3] != "BY")
    {
        cout << "SYNTAX ERROR: Expected BY keyword at position 4" << endl;
        return false;
    }
    int i = 4;
    while (tokenizedQuery[i] != "ASC" && tokenizedQuery[i] != "DESC")
    {
        parsedQuery.sortColumnNames.push_back(tokenizedQuery[i]);
        i++;
    }
    if (i >= tokenizedQuery.size())
    {
        cout << "SYNTAX ERROR: Expected Sorting Strategy at position " << i << endl;
        return false;
    }

    // For each attribute, order must be mentioned
    while (tokenizedQuery[i] != "ON")
    {
        if (tokenizedQuery[i] == "ASC")
            parsedQuery.sortingStrategies.push_back(ASC);
        else if (tokenizedQuery[i] == "DESC")
            parsedQuery.sortingStrategies.push_back(DESC);
        else
        {
            cout << "SYNTAX ERROR: Expected ASC or DESC at position " << i << endl;
            return false;
        }
        i++;
    }
    i++;
    parsedQuery.orderOnRelationName = tokenizedQuery[i];

    if (parsedQuery.sortColumnNames.size() != parsedQuery.sortingStrategies.size())
    {
        cout << "SYNTAX ERROR: Number of attributes and number of sorting strategies don't match" << endl;
        return false;
    }
    return true;
}

bool semanticParseORDER()
{
    logger.log("semanticParseSORT");

    if (!tableCatalogue.isTable(parsedQuery.orderOnRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    for (int i = 0; i < parsedQuery.sortColumnNames.size(); i++)
    {
        if (!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnNames[i], parsedQuery.orderOnRelationName))
        {
            cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
            return false;
        }
    }

    return true;
}

void executeORDER()
{
    logger.log("executeORDER");
    Table *table = tableCatalogue.getTable(parsedQuery.orderOnRelationName);
    table->order_by(parsedQuery.orderResultRelationName, parsedQuery.sortColumnNames, parsedQuery.sortingStrategies);
    parsedQuery.sortColumnNames.clear();
    parsedQuery.sortingStrategies.clear();
    return;
}