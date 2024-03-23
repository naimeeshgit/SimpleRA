#include "global.h"
/**
 * @brief File contains method to process SORT commands.
 *
 * syntax:
 * R <- SORT relation_name BY column_name IN sorting_order
 *
 * sorting_order = ASC | DESC
 */
bool syntacticParseSORT()
{
    parsedQuery.sortColumnNames.clear();
    parsedQuery.sortingStrategies.clear();
    logger.log("syntacticParseSORT");
    // command syntax: SORT <table_name> BY <column_name1, column_name2,..., column_namek> IN <ASC|DESC, ASC|DESC,..., ASC|DESC>
    parsedQuery.queryType = SORT;
    parsedQuery.sortRelationName = tokenizedQuery[1];
    if (tokenizedQuery[2] != "BY")
    {
        cout << "SYNTAX ERROR: Expected BY keyword at position 2" << endl;
        return false;
    }
    int i = 3;
    while (tokenizedQuery[i] != "IN" && i < tokenizedQuery.size())
    {
        parsedQuery.sortColumnNames.push_back(tokenizedQuery[i]);
        i++;
    }
    i++;
    if (i >= tokenizedQuery.size())
    {
        cout << "SYNTAX ERROR: Expected IN" << endl;
        return false;
    }

    // For each attribute, order must be mentioned
    while (i < tokenizedQuery.size())
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
    if (parsedQuery.sortColumnNames.size() != parsedQuery.sortingStrategies.size())
    {
        cout << "SYNTAX ERROR: Number of attributes and number of sorting strategies don't match" << endl;
        return false;
    }
    return true;
}

bool semanticParseSORT()
{
    logger.log("semanticParseSORT");

    if (!tableCatalogue.isTable(parsedQuery.sortRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    for (int i = 0; i < parsedQuery.sortColumnNames.size(); i++)
    {
        if (!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnNames[i], parsedQuery.sortRelationName))
        {
            cout << "SEMANTIC ERROR: Column doesn't exist in relation: " << parsedQuery.sortColumnNames[i] << endl;
            return false;
        }
    }

    return true;
}

void executeSORT()
{
    logger.log("executeSORT");
    Table *table = tableCatalogue.getTable(parsedQuery.sortRelationName);
    table->sort(parsedQuery.sortColumnNames, parsedQuery.sortingStrategies);
    parsedQuery.sortColumnNames.clear();
    parsedQuery.sortingStrategies.clear();
    return;
}