#include "global.h"
#include <regex>
using namespace std;
/**
 * @brief
 * SYNTAX: <new_table> <- GROUP BY <grouping_attribute> FROM <table_name> HAVING <aggregate(attribute)> <bin_op> <attribute_value> RETURN <aggregate_func(attribute)>
 e.g. G <- GROUP BY EID FROM R HAVING AVG(Salary) <= 55000 RETURN COUNT(Salary)
 */
bool syntacticParseGROUP()
{
    logger.log("syntacticParseGROUP");
    if (tokenizedQuery.size() != 13 || tokenizedQuery[3] != "BY" || tokenizedQuery[5] != "FROM" || tokenizedQuery[7] != "HAVING" || tokenizedQuery[11] != "RETURN")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = GROUP;
    parsedQuery.groupResultantRelationName = tokenizedQuery[0];
    parsedQuery.groupRelationName = tokenizedQuery[6];
    parsedQuery.attributeValue = stoi(tokenizedQuery[10]);
    parsedQuery.groupingAttribute = tokenizedQuery[4];

    string binaryOperator = tokenizedQuery[9];
    if (binaryOperator == "<")
        parsedQuery.selectionBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.selectionBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.selectionBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.selectionBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.selectionBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.selectionBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    // if tokenizedQuery[8] = MAX(Salary) -> give MAX as aggregateFunc and Salary as aggregateAttribute
    string aggregateFunc = tokenizedQuery[8].substr(0, tokenizedQuery[8].find("("));
    string binaryAttribute = tokenizedQuery[8].substr(tokenizedQuery[8].find("(") + 1, tokenizedQuery[8].find(")") - tokenizedQuery[8].find("(") - 1);
    parsedQuery.binaryAttribute = binaryAttribute;

    if (aggregateFunc == "SUM")
        parsedQuery.aggregateFunc = SUM;
    else if (aggregateFunc == "COUNT")
        parsedQuery.aggregateFunc = COUNT;
    else if (aggregateFunc == "AVG")
        parsedQuery.aggregateFunc = AVG;
    else if (aggregateFunc == "MIN")
        parsedQuery.aggregateFunc = MIN;
    else if (aggregateFunc == "MAX")
        parsedQuery.aggregateFunc = MAX;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    // if tokenizedQuery[12] = MAX(Salary) -> give MAX as aggregateFunc and Salary as aggregateAttribute
    string aggregateFunc2 = tokenizedQuery[12].substr(0, tokenizedQuery[12].find("("));
    string aggregateAttribute = tokenizedQuery[12].substr(tokenizedQuery[12].find("(") + 1, tokenizedQuery[12].find(")") - tokenizedQuery[12].find("(") - 1);
    parsedQuery.aggregateAttribute = aggregateAttribute;

    if (aggregateFunc2 == "SUM")
        parsedQuery.aggregateFunc2 = SUM;
    else if (aggregateFunc2 == "COUNT")
        parsedQuery.aggregateFunc2 = COUNT;
    else if (aggregateFunc2 == "AVG")
        parsedQuery.aggregateFunc2 = AVG;
    else if (aggregateFunc2 == "MIN")
        parsedQuery.aggregateFunc2 = MIN;
    else if (aggregateFunc2 == "MAX")
        parsedQuery.aggregateFunc2 = MAX;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseGROUP()
{
    logger.log("semanticParseGROUP");

    if (tableCatalogue.isTable(parsedQuery.groupResultantRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupingAttribute, parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: Column " << parsedQuery.groupingAttribute << " doesn't exist in relation" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.binaryAttribute, parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: Column " << parsedQuery.binaryAttribute << " doesn't exist in relation" << endl;
        return false;
    }

    return true;
}

void executeGROUP()
{
    logger.log("executeGROUP");

    Table *table = tableCatalogue.getTable(parsedQuery.groupRelationName);
    table->group_by(parsedQuery.groupResultantRelationName, parsedQuery.groupingAttribute, parsedQuery.aggregateFunc, parsedQuery.binaryAttribute, parsedQuery.selectionBinaryOperator, parsedQuery.attributeValue, parsedQuery.aggregateFunc2, parsedQuery.aggregateAttribute);
    return;
}