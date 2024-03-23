#include "global.h"
/**
 * @brief Construct a new Page object. Never used as part of the code
 *
 */
Page::Page()
{
    this->pageName = "";
    this->tableName = "";
    this->matrixName = "";
    this->pageIndex = -1;
    this->rowCount = 0;
    this->columnCount = 0;
    this->rows.clear();
}

/**
 * @brief Construct a new Page:: Page object given the table name and page
 * index. When tables are loaded they are broken up into blocks of BLOCK_SIZE
 * and each block is stored in a different file named
 * "<tablename>_Page<pageindex>". For example, If the Page being loaded is of
 * table "R" and the pageIndex is 2 then the file name is "R_Page2". The page
 * loads the rows (or tuples) into a vector of rows (where each row is a vector
 * of integers).
 *
 * @param name
 * @param pageIndex
 */
Page::Page(string name, int pageIndex)
{
    logger.log("Page::Page");
    logger.log(name);
    logger.log(to_string(pageIndex));

    this->pageIndex = pageIndex;
    this->pageName = "../data/temp/" + name + "_Page" + to_string(pageIndex);
    if (INPUT_TYPE == TABLE)
    {
        this->tableName = name;
        Table table = *tableCatalogue.getTable(this->tableName);
        this->columnCount = table.columnCount;
        uint maxRowCount = table.maxRowsPerBlock;
        logger.log("maxRowCount");
        logger.log(to_string(maxRowCount));
        vector<int> row(columnCount, 0);
        this->rows.assign(maxRowCount, row);
        this->rowCount = table.rowsPerBlockCount[pageIndex];
        ifstream fin(pageName, ios::in);
        int number;
        for (uint rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
        {
            for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
            {
                fin >> number;
                this->rows[rowCounter][columnCounter] = number;
            }
        }
        fin.close();
    }
    else if (INPUT_TYPE == MATRIX)
    {
        this->matrixName = name;
        Matrix matrix = *matrixCatalogue.getMatrix(matrixName);
        this->columnCount = matrix.columnCount;
        uint maxRowCount = matrix.maxRowsPerBlock;
        // this->rowCount = matrix.rowsPerBlockCount[pageIndex];
        // read a csv file which has a matrix of unknown dimensions
        ifstream file(pageName);
        string line;
        vector<vector<int>> submatrix;
        int num_rows = 0;
        int num_columns = 0;
        while (getline(file, line))
        {
            vector<int> row;
            stringstream ss(line);
            string cell;
            while (getline(ss, cell, ' '))
            {
                row.push_back(stoi(cell));
            }
            submatrix.push_back(row);
            num_rows++;
            num_columns = row.size();
        }

        vector<int> row(num_columns, 0);
        this->rows.assign(num_rows, row);

        for (int i = 0; i < num_rows; i++)
            for (int j = 0; j < num_columns; j++)
                this->rows[i][j] = submatrix[i][j];

        this->rowCount = num_rows;
        this->columnCount = num_columns;

        file.close();
    }
}

/**
 * @brief Get row from page indexed by rowIndex
 *
 * @param rowIndex
 * @return vector<int>
 */
vector<int> Page::getRow(int rowIndex)
{
    logger.log("Page::getRow");
    vector<int> result;
    result.clear();
    if (rowIndex >= this->rowCount)
        return result;
    return this->rows[rowIndex];
}

Page::Page(string name, int pageIndex, vector<vector<int>> rows, int rowCount)
{
    logger.log("Page::Page");
    if (INPUT_TYPE == TABLE)
        this->tableName = name;
    else
        this->matrixName = name;

    this->pageIndex = pageIndex;
    this->rows = rows;
    this->rowCount = rowCount;
    this->columnCount = rows[0].size();
    this->pageName = "../data/temp/" + name + "_Page" + to_string(pageIndex);
}

/**
 * @brief writes current page contents to file.
 *
 */
void Page::writePage()
{
    logger.log("Page::writePage");
    BLOCKS_WRITE++;
    ofstream fout;
    if (INPUT_TYPE == TABLE || parsedQuery.queryType == TRANSPOSE)
    {
        fout.open(this->pageName, ios::trunc);
    }
    else
    {
        fout.open(this->pageName, ios::app);
    }
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (columnCounter != 0)
                fout << " ";
            fout << this->rows[rowCounter][columnCounter];
        }
        fout << endl;
    }
    fout.close();
}

vector<vector<int>> Page::getAllRows()
{
    return this->rows;
}