#include "global.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>

/**
 * @brief Construct a new Table:: Table object
 *
 */
Table::Table()
{
    logger.log("Table::Table");
}

/**
 * @brief Construct a new Table:: Table object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param tableName
 */
Table::Table(string tableName)
{
    logger.log("Table::Table");
    this->sourceFileName = "../data/" + tableName + ".csv";
    this->tableName = tableName;
}

/**
 * @brief Construct a new Table:: Table object used when an assignment command
 * is encountered. To create the table object both the table name and the
 * columns the table holds should be specified.
 *
 * @param tableName
 * @param columns
 */
Table::Table(string tableName, vector<string> columns)
{
    logger.log("Table::Table");
    this->sourceFileName = "../data/temp/" + tableName + ".csv";
    this->tableName = tableName;
    this->columns = columns;
    this->columnCount = columns.size();
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
    this->writeRow<string>(columns);
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates table
 * statistics.
 *
 * @return true if the table has been successfully loaded
 * @return false if an error occurred
 */
bool Table::load()
{
    logger.log("Table::load");
    logger.log(this->sourceFileName);
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        if (this->extractColumnNames(line))
            if (this->blockify())
                return true;
    }
    fin.close();
    return false;
}

/**
 * @brief Function extracts column names from the header line of the .csv data
 * file.
 *
 * @param line
 * @return true if column names successfully extracted (i.e. no column name
 * repeats)
 * @return false otherwise
 */
bool Table::extractColumnNames(string firstLine)
{
    logger.log("Table::extractColumnNames");
    unordered_set<string> columnNames;
    string word;
    stringstream s(firstLine);
    while (getline(s, word, ','))
    {
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        if (columnNames.count(word))
            return false;
        columnNames.insert(word);
        this->columns.emplace_back(word);
    }
    this->columnCount = this->columns.size();
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->columnCount));
    return true;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size.
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Table::blockify()
{
    logger.log("Table::blockify");
    // cout << this->sourceFileName << endl;
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    vector<int> row(this->columnCount, 0);
    vector<vector<int>> rowsInPage(this->maxRowsPerBlock, row);
    // maxRowsPerBlock*columnCount
    int pageCounter = 0;
    unordered_set<int> dummy;
    dummy.clear();
    // vector of unordered set (size of vector = columnCount)
    this->distinctValuesInColumns.assign(this->columnCount, dummy);
    this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    getline(fin, line);
    while (getline(fin, line))
    {
        stringstream s(line);
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (!getline(s, word, ','))
                return false;
            row[columnCounter] = stoi(word);
            rowsInPage[pageCounter][columnCounter] = row[columnCounter];
        }
        pageCounter++;
        this->updateStatistics(row);
        if (pageCounter == this->maxRowsPerBlock)
        {
            bufferManager.writePage(this->tableName, this->blockCount, rowsInPage, pageCounter);
            this->blockCount++;
            this->rowsPerBlockCount.emplace_back(pageCounter);
            pageCounter = 0;
        }
    }
    if (pageCounter)
    {
        bufferManager.writePage(this->tableName, this->blockCount, rowsInPage, pageCounter);
        this->blockCount++;
        this->rowsPerBlockCount.emplace_back(pageCounter);
        pageCounter = 0;
    }

    if (this->rowCount == 0)
        return false;
    this->distinctValuesInColumns.clear();
    return true;
}

/**
 * @brief Given a row of values, this function will update the statistics it
 * stores i.e. it updates the number of rows that are present in the column and
 * the number of distinct values present in each column. These statistics are to
 * be used during optimisation.
 *
 * @param row
 */
void Table::updateStatistics(vector<int> row)
{
    this->rowCount++;
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (!this->distinctValuesInColumns[columnCounter].count(row[columnCounter]))
        {
            this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
            this->distinctValuesPerColumnCount[columnCounter]++;
        }
    }
}

/**
 * @brief Checks if the given column is present in this table.
 *
 * @param columnName
 * @return true
 * @return false
 */
bool Table::isColumn(string columnName)
{
    logger.log("Table::isColumn");
    for (auto col : this->columns)
    {
        if (col == columnName)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Renames the column indicated by fromColumnName to toColumnName. It is
 * assumed that checks such as the existence of fromColumnName and the non prior
 * existence of toColumnName are done.
 *
 * @param fromColumnName
 * @param toColumnName
 */
void Table::renameColumn(string fromColumnName, string toColumnName)
{
    logger.log("Table::renameColumn");
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (columns[columnCounter] == fromColumnName)
        {
            columns[columnCounter] = toColumnName;
            break;
        }
    }
    return;
}

/**
 * @brief Function prints the first few rows of the table. If the table contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Table::print()
{
    logger.log("Table::print");
    uint count = min((long long)PRINT_COUNT, this->rowCount);

    // print headings
    this->writeRow(this->columns, cout);

    Cursor cursor(this->tableName, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < count; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, cout);
    }
    printRowCount(this->rowCount);
}

/**
 * @brief This function returns one row of the table using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor
 * @return vector<int>
 */
void Table::getNextPage(Cursor *cursor)
{
    logger.log("Table::getNext");

    if (cursor->pageIndex < this->blockCount - 1)
    {
        cursor->nextPage(cursor->pageIndex + 1);
    }
}

/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Table::makePermanent()
{
    logger.log("Table::makePermanent");
    if (!this->isPermanent())
    {
        bufferManager.deleteFile(this->sourceFileName);
    }
    string newSourceFile = "../data/" + this->tableName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    // print headings
    this->writeRow(this->columns, fout);

    Cursor cursor(this->tableName, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, fout);
    }
    fout.close();
}

/**
 * @brief Function to check if table is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Table::isPermanent()
{
    logger.log("Table::isPermanent");
    // cout << this->sourceFileName << endl;
    // cout << "../data/" + this->tableName + ".csv" << endl;
    if (this->sourceFileName == "../data/" + this->tableName + ".csv")
        return true;
    return false;
}

/**
 * @brief The unload function removes the table from the database by deleting
 * all temporary files created as part of this table
 *
 */
void Table::unload()
{
    logger.log("Table::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->tableName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this table
 *
 * @return Cursor
 */
Cursor Table::getCursor()
{
    logger.log("Table::getCursor");
    Cursor cursor(this->tableName, 0);
    return cursor;
}
/**
 * @brief Function that returns the index of column indicated by columnName
 *
 * @param columnName
 * @return int
 */
int Table::getColumnIndex(string columnName)
{
    logger.log("Table::getColumnIndex");
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (this->columns[columnCounter] == columnName)
            return columnCounter;
    }
}

void internal_sort(std::vector<std::vector<int>> &vecOfVecs, std::vector<SortingStrategy> &sortingStrategies, std::vector<int> &columnIndices)
{
    // Define a custom sorting lambda function
    auto customSort = [&columnIndices, &sortingStrategies](const std::vector<int> &a, const std::vector<int> &b)
    {
        for (int i = 0; i < columnIndices.size(); i++)
        {
            int col = columnIndices[i];
            SortingStrategy strategy = sortingStrategies[i];

            if (strategy == ASC)
            {
                if (a[col] < b[col])
                    return true;
                else if (a[col] > b[col])
                    return false;
                // If a[col] and b[col] are equal, continue to the next column
            }
            else if (strategy == DESC)
            {
                if (a[col] > b[col])
                    return true;
                else if (a[col] < b[col])
                    return false;
                // If a[col] and b[col] are equal, continue to the next column
            }
        }
        // If all columns are equal, the rows are considered equal for sorting purposes
        return false;
    };

    // Perform a sort on the vector of vectors using the custom sorting function
    std::sort(vecOfVecs.begin(), vecOfVecs.end(), customSort);
}

struct CustomComparator
{
    vector<int> columns;
    vector<SortingStrategy> strategies;

    CustomComparator(const std::vector<int> &cols, const std::vector<SortingStrategy> &strats)
        : columns(cols), strategies(strats) {}

    bool operator()(const std::pair<vector<int>, int> &a, const std::pair<vector<int>, int> &b) const
    {
        for (int i = 0; i < columns.size(); ++i)
        {
            int col = columns[i];
            if (a.first[col] != b.first[col])
            {
                return strategies[i] == ASC ? (a.first[col] > b.first[col]) : (a.first[col] < b.first[col]);
            }
        }
        return false; // default if all columns are equivalent
    }
};

int merge(vector<Cursor *> &cursors, vector<vector<int>> &merged_block, vector<int> &columnIndices, vector<SortingStrategy> &sortingStrategies, int maxRowsPerBlock, vector<int> &nrecords_remaining, priority_queue<pair<vector<int>, int>, vector<pair<vector<int>, int>>, CustomComparator> &heap)
{
    // print nrecords_remaining
    // cout << "nrecords_remaining" << endl;
    // for (int i = 0; i < nrecords_remaining.size(); i++)
    //     cout << nrecords_remaining[i] << " ";
    // cout << endl;

    int flag = 1;
    merged_block.clear();

    // sorting strategy for the corresponding column numbers is given in sortingStrategies
    // corresponding column numbers are given in columnIndices
    // cursors is a vector of cursors
    // merged_block is the block that is to be written to the new subfile
    // maxRowsPerBlock is the maximum number of rows that can be stored in a block

    // we have to merge maxRowsPerBlock rows from k-1 blocks
    // create a heap structure
    // each element of the heap is a pair of the form (row, index of the subfile from which the row is taken)
    // use a custom comparator to compare the rows

    // PRINT HEAP CONTENTS
    // cout << "HEAP CONTENTS" << endl;
    // priority_queue<pair<vector<int>, int>, vector<pair<vector<int>, int>>, CustomComparator> heap_copy(CustomComparator(columnIndices, sortingStrategies));
    // heap_copy = heap;
    // while (!heap_copy.empty())
    // {
    //     pair<vector<int>, int> row = heap_copy.top();
    //     heap_copy.pop();
    //     for (int i = 0; i < row.first.size(); i++)
    //         cout << row.first[i] << " ";
    //     cout << endl;
    // }

    while (maxRowsPerBlock-- && !heap.empty())
    {
        // pop the top element from the heap
        pair<vector<int>, int> row = heap.top();
        heap.pop();
        merged_block.push_back(row.first);
        // cout << "first elem of going " << row.first[0] << endl;
        if (nrecords_remaining[row.second] > 0)
        {
            vector<int> new_row = cursors[row.second]->getNext();
            nrecords_remaining[row.second]--;
            if (!new_row.empty())
            {
                // cout << "first elem of coming " << new_row[0] << endl;
                heap.push(make_pair(new_row, row.second));
            }
            else
            {
                nrecords_remaining[row.second] = 0;
                int f = 0;
                for (int i = 0; i < nrecords_remaining.size(); i++)
                    if (nrecords_remaining[i] > 0)
                        f = 1;
                if (f == 0)
                    break;
            }
        }
    }
    flag = 0;
    for (int i = 0; i < nrecords_remaining.size(); i++)
    {
        if (nrecords_remaining[i] > 0)
        {
            flag = 1;
            break;
        }
    }
    if (flag == 0)
    {
        if (!heap.empty())
        {
            // add the remaining elements in the heap to the merged_block
            while (!heap.empty())
            {
                pair<vector<int>, int> row = heap.top();
                heap.pop();
                merged_block.push_back(row.first);
            }
        }
    }
    return flag;
}

void sorting_phase(vector<Table *> &runs, Table *Original_Table, vector<int> &columnIndices, vector<SortingStrategy> &sortingStrategies)
{
    Table *previous_run = new Table(Original_Table->tableName + "_previous_run");
    logger.log("Table::sorting phase");
    tableCatalogue.insertTable(previous_run);
    previous_run->sourceFileName = "../data/" + previous_run->tableName + ".csv";
    ofstream fout;
    fout.open(previous_run->sourceFileName, ios::trunc);
    Original_Table->writeRow(Original_Table->columns, fout);

    int i = 1;                                      // iterator for sorting runs
    int total_blocks = Original_Table->blockCount;  // size of files in blocks
    int k = SORTING_BUFFER_SIZE;                    // available blocks in buffer
    int m = ceil((double)total_blocks / (double)k); // number of subfiles (total sorting runs)

    // sorting phase
    while (i <= m)
    {
        // read next k blocks into main memory
        Cursor cursor(Original_Table->tableName, (i - 1) * k);
        vector<vector<int>> rows;

        for (int pageCounter = (i - 1) * k; pageCounter < i * k; pageCounter++)
        {
            if (pageCounter >= Original_Table->blockCount)
                break;
            for (int i = 0; i < Original_Table->rowsPerBlockCount[pageCounter]; i++)
            {
                rows.push_back(cursor.getNext());
            }
        }

        // sort rows using internal sort
        internal_sort(rows, sortingStrategies, columnIndices);
        // cout << "after SORTING PHASE"
        //      << " #internally sorted rows= " << rows.size() << endl;
        for (int i = 0; i < rows.size(); i++)
        {
            // for (int j = 0; j < rows[i].size(); j++)
            //     cout << rows[i][j] << " ";
            // cout << endl;
            Original_Table->writeRow(rows[i], fout);
        }
        i++;
    }
    fout.close();
    previous_run->load();

    // put the sortint phase table in runs array
    runs.push_back(previous_run);
}

void merging_phase(vector<Table *> &runs, vector<int> &columnIndices, vector<SortingStrategy> &sortingStrategies, Table *Original_Table)
{
    logger.log("Table::merging phase");
    int total_blocks = Original_Table->blockCount;                // total blocks in the Table
    int k = SORTING_BUFFER_SIZE;                                  // available blocks in buffer
    int merge_factor = k - 1;                                     // number of subfiles to be merged at a time
    int m = ceil((double)total_blocks / (double)k);               // number of subfiles after sorting phase (phase_0) (total sorting runs)
    int p = ceil(double(log10(m)) / double(log10(merge_factor))); // number merging phases

    int i = 1;              // iterator for phases
    int nsubfiles_curr = m; // number of subfiles in the current phase

    while (i <= p)
    {
        // cout << "Phase " << i << endl;
        Table *next_run = new Table(Original_Table->tableName + "_run_" + to_string(i));
        runs.push_back(next_run);
        tableCatalogue.insertTable(runs[i]);
        runs[i]->sourceFileName = "../data/" + runs[i]->tableName + ".csv";
        runs[i]->blockCount = 0;
        ofstream fout;
        fout.open(runs[i]->sourceFileName, ios::trunc);
        runs[i]->writeRow(Original_Table->columns, fout);

        // cout << "Phase " << i << " has " << nsubfiles_curr << " subfiles" << endl;

        int j = 1;                                                   // iterator for merge steps
        int q = ceil(double(nsubfiles_curr) / double(merge_factor)); // number of merge steps in this phase
        // cout << "#merge steps in this phase: " << q << endl;

        while (j <= q)
        {
            // calculate how many cursors to make for this merge step
            int merge_factor_updated = merge_factor;
            if (j == q)
            {
                // last merge step, merge_factor_updated = number of subfiles in the last merge step
                if (nsubfiles_curr % (merge_factor) != 0)
                    merge_factor_updated = nsubfiles_curr % (merge_factor);
            }
            vector<Cursor *> cursors(merge_factor_updated);
            for (int itr = 0; itr < merge_factor_updated; itr++)
            {
                // we have to make merge_factor_updated cursors
                // for any phase, number of blocks in the second last subfile - number of blocks in the last subfile = 4 i.e. this->blockCount/k - this->blockCount%k = y
                int x = SORTING_BUFFER_SIZE * pow(k - 1, i - 1);
                cursors[itr] = new Cursor(runs[i - 1]->tableName, x * ((j - 1) * (merge_factor) + itr));
                // cout << "Cursor " << itr << " " << x * ((j - 1) * (merge_factor) + itr) << endl;
            }
            // k-1 way merging using the cursors
            // this while loop runs until all subfiles for this merge step are merged

            int flag = 1;
            vector<int> nrecords_remaining(merge_factor_updated, Original_Table->maxRowsPerBlock * SORTING_BUFFER_SIZE * pow(merge_factor, i - 1));
            priority_queue<pair<vector<int>, int>, vector<pair<vector<int>, int>>, CustomComparator> heap(CustomComparator(columnIndices, sortingStrategies));

            // insert the first row from each subfile into the heap
            for (int i = 0; i < cursors.size(); i++)
            {
                vector<int> row = cursors[i]->getNext();
                nrecords_remaining[i]--;
                if (row.size() == 0)
                {
                    nrecords_remaining[i] = 0;
                    break;
                }
                heap.push(make_pair(row, i));
            }

            while (flag)
            {
                vector<vector<int>> merged_block;
                flag = merge(cursors, merged_block, columnIndices, sortingStrategies, Original_Table->maxRowsPerBlock, nrecords_remaining, heap); // k-1=cursors.size() way merge
                // PRINT MERGED BLOCK
                // cout << "----------------------------" << endl;
                // for (int i = 0; i < merged_block.size(); i++)
                // {
                //     for (int j = 0; j < merged_block[i].size(); j++)
                //         cout << merged_block[i][j] << " ";
                //     cout << endl;
                // }
                // cout << "----------------------------" << endl;
                // write merged_block to next_run
                // bufferManager.writePage(runs[i]->tableName, runs[i]->blockCount, merged_block, merged_block.size());
                // runs[i]->blockCount++;
                // WRITE MERGED BLOCK IN FILE
                for (int i = 0; i < merged_block.size(); i++)
                {
                    runs[i]->writeRow(merged_block[i], fout);
                }
            }
            j++;
            merge_factor_updated = merge_factor;
        }
        runs[i]->load();
        fout.close();

        nsubfiles_curr = q;
        i++;
    }
}

void external_sort_implementation(vector<Table *> &runs, vector<int> &columnIndices, vector<SortingStrategy> &sortingStrategies, Table *Original_Table)
{
    // sorting phase
    sorting_phase(runs, Original_Table, columnIndices, sortingStrategies);

    // merging phase
    merging_phase(runs, columnIndices, sortingStrategies, Original_Table);
}

void Table::sort(vector<string> columnNames, vector<SortingStrategy> sortingStrategies)
{
    logger.log("Table::sort");
    Table *Original_Table = this;
    vector<Table *> runs;
    vector<int> columnIndices;
    for (int i = 0; i < columnNames.size(); i++)
    {
        columnIndices.push_back(this->getColumnIndex(columnNames[i]));
    }

    // external sort implementation
    external_sort_implementation(runs, columnIndices, sortingStrategies, Original_Table);

    // last element of runs is the sorted table
    // copy the sorted table to this table

    // get the source file of the orignal table and overwrite its contents with the sorted table line by line
    string sourceFileName = Original_Table->sourceFileName;
    ofstream fout;
    fout.open(sourceFileName, ios::trunc);
    Original_Table->writeRow(Original_Table->columns, fout);

    Cursor cursor(runs[runs.size() - 1]->tableName, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < runs[runs.size() - 1]->rowCount; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, fout);
    }
    fout.close();

    // delete all the runs
    for (int i = 0; i < runs.size(); i++)
    {
        runs[i]->unload();
    }

    // delete the files from data folder
    for (int i = 0; i < runs.size(); i++)
    {
        // delete files using file path
        string path = "../data/" + runs[i]->tableName + ".csv";
        // delete the file using c++ function
        remove(path.c_str());
    }

    // delete the temporary tables from the table catalogue
    for (int i = 0; i < runs.size(); i++)
    {
        tableCatalogue.deleteTable(runs[i]->tableName);
    }

    runs.clear();
    // delete the temporary tables
    for (int i = 0; i < runs.size(); i++)
    {
        delete runs[i];
    }

    string original_table = Original_Table->tableName;

    // remove from table catalogue
    tableCatalogue.deleteTable(this->tableName);

    // create new table with the same name
    Table *new_table = new Table(original_table);
    tableCatalogue.insertTable(new_table);
    new_table->sourceFileName = "../data/" + new_table->tableName + ".csv";
    new_table->blockCount = 0;
    new_table->load();
}

void Table::order_by(string TableName, vector<string> columnNames, vector<SortingStrategy> sortingStrategies)
{
    logger.log("Table::order_by");
    Table *Original_Table = this;
    vector<Table *> runs;
    vector<int> columnIndices;
    for (int i = 0; i < columnNames.size(); i++)
    {
        columnIndices.push_back(this->getColumnIndex(columnNames[i]));
    }

    // external sort implementation
    external_sort_implementation(runs, columnIndices, sortingStrategies, Original_Table);

    // last element of runs is the sorted table
    // create new table named TableName
    Table *new_table = new Table(TableName);
    tableCatalogue.insertTable(new_table);
    new_table->sourceFileName = "../data/" + new_table->tableName + ".csv";
    new_table->blockCount = 0;
    ofstream fout;
    fout.open(new_table->sourceFileName, ios::trunc);
    new_table->writeRow(Original_Table->columns, fout);

    // write the contents of runs[runs.size() - 1] to new_table
    Cursor cursor(runs[runs.size() - 1]->tableName, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < runs[runs.size() - 1]->rowCount; rowCounter++)
    {
        row = cursor.getNext();
        new_table->writeRow(row, fout);
    }
    fout.close();

    // load the new table
    new_table->load();

    // delete all the runs
    for (int i = 0; i < runs.size(); i++)
    {
        runs[i]->unload();
    }

    // delete the temporary tables from the table catalogue
    for (int i = 0; i < runs.size(); i++)
    {
        tableCatalogue.deleteTable(runs[i]->tableName);
    }

    // delete the files from data folder
    for (int i = 0; i < runs.size(); i++)
    {
        // delete files using file path
        string path = "../data/" + runs[i]->tableName + ".csv";
        // delete the file using c++ function
        remove(path.c_str());
    }

    runs.clear();
    // delete the temporary tables
    for (int i = 0; i < runs.size(); i++)
    {
        delete runs[i];
    }

    
}

// Function to compare two values based on the specified operator
bool compareValues(int value1, int value2, int op)
{
    switch (op)
    {
    case EQUAL:
        return value1 == value2;
    case LESS_THAN:
        return value1 < value2;
    case LEQ:
        return value1 <= value2;
    case GREATER_THAN:
        return value1 > value2;
    case GEQ:
        return value1 >= value2;
    default:
        // Handle any other cases or raise an error if needed
        return false;
    }
}

void Table::join(Table *secondTable, string joinFirstColumnName, string joinSecondColumnName, int joinBinaryOperator, string joinResultRelationName)
{
    logger.log("Table::join");

    Table *FirstTable = this;
    Table *SecondTable = secondTable;

    // create new table named joinResultRelationName
    Table *new_table = new Table(joinResultRelationName);
    tableCatalogue.insertTable(new_table);
    new_table->sourceFileName = "../data/" + new_table->tableName + ".csv";
    new_table->blockCount = 0;
    ofstream fout;
    fout.open(new_table->sourceFileName, ios::trunc);

    // get the column indices of the join columns
    int joinFirstColumnIndex = FirstTable->getColumnIndex(joinFirstColumnName);
    int joinSecondColumnIndex = SecondTable->getColumnIndex(joinSecondColumnName);

    // write the column names of the new table
    vector<string> new_table_columns;
    for (int i = 0; i < FirstTable->columns.size(); i++)
    {
        new_table_columns.push_back(FirstTable->columns[i]);
    }

    for (int i = 0; i < SecondTable->columns.size(); i++)
    {

        new_table_columns.push_back(SecondTable->columns[i]);
    }

    new_table->writeRow(new_table_columns, fout);

    // sort the two tables using order_by on the join columns
    FirstTable->order_by(FirstTable->tableName + "_sorted", {joinFirstColumnName}, {ASC});
    SecondTable->order_by(SecondTable->tableName + "_sorted", {joinSecondColumnName}, {ASC});

    // get the cursors for the two tables
    Cursor FirstTableCursor(FirstTable->tableName + "_sorted", 0);

    // get the first rows of the two tables
    vector<int> FirstTableRow = FirstTableCursor.getNext();

    // join the two tables and write to new_table
    while (FirstTableRow.size() != 0)
    {
        Cursor SecondTableCursor(SecondTable->tableName + "_sorted", 0);
        vector<int> SecondTableRow = SecondTableCursor.getNext();
        while (SecondTableRow.size() != 0)
        {
            if (compareValues(FirstTableRow[joinFirstColumnIndex], SecondTableRow[joinSecondColumnIndex], joinBinaryOperator))
            {
                vector<int> combined_row;
                for (int i = 0; i < FirstTableRow.size(); i++)
                {
                    combined_row.push_back(FirstTableRow[i]);
                }
                for (int i = 0; i < SecondTableRow.size(); i++)
                {
                    combined_row.push_back(SecondTableRow[i]);
                }
                new_table->writeRow(combined_row, fout);
            }
            SecondTableRow = SecondTableCursor.getNext();
        }
        FirstTableRow = FirstTableCursor.getNext();
    }
    tableCatalogue.deleteTable(FirstTable->tableName + "_sorted");
    tableCatalogue.deleteTable(SecondTable->tableName + "_sorted");

    // delete the files from data folder
    string path = "../data/" + FirstTable->tableName + "_sorted.csv";
    remove(path.c_str());
    path = "../data/" + SecondTable->tableName + "_sorted.csv";
    remove(path.c_str());
    
    new_table->load();
}

void Table::group_by(string groupResultantRelationName, string groupingAttribute, int aggregateFunc1, string binaryAttribute, int selectionBinaryOperator, int attributeValue, int aggregateFunc2, string aggregateAttribute)
{
    // sort table by groupingAttribute
    this->order_by(this->tableName + "_sorted", {groupingAttribute}, {ASC});

    // get sorted table cursor
    Cursor cursor(this->tableName + "_sorted", 0);

    // create a resultant table
    Table *resultant_table = new Table(groupResultantRelationName);
    tableCatalogue.insertTable(resultant_table);
    resultant_table->sourceFileName = "../data/" + resultant_table->tableName + ".csv";
    resultant_table->blockCount = 0;

    vector<string> resultant_table_columns = {groupingAttribute, aggregateAttribute};
    ofstream fout;
    fout.open(resultant_table->sourceFileName, ios::trunc);
    resultant_table->writeRow(resultant_table_columns, fout);

    // get the column indices
    int groupingAttributeIndex = this->getColumnIndex(groupingAttribute);
    int binaryAttributeIndex = this->getColumnIndex(binaryAttribute);
    int aggregateAttributeIndex = this->getColumnIndex(aggregateAttribute);

    // binaryAttribute is same as aggregateAttribute

    vector<int> row = cursor.getNext();
    while (true)
    {
        if(row.size() == 0) break;
        int min = INT32_MAX;
        int max = INT32_MIN;
        int sum = 0;
        int count = 0;
        int avg = 0;
        int grouping_attribute_value = row[groupingAttributeIndex];
        while(row.size() != 0 && (row[groupingAttributeIndex] == grouping_attribute_value)){
            if (row[binaryAttributeIndex] < min)
                min = row[binaryAttributeIndex];
            if (row[binaryAttributeIndex] > max)
                max = row[binaryAttributeIndex];
            
            sum += row[binaryAttributeIndex];
            count++;
            row = cursor.getNext();
        }
        
        avg = sum / count;
        int flag = 0;
        switch (aggregateFunc1)
        {
        case MIN:
            if (compareValues(min, attributeValue, selectionBinaryOperator))
                flag = 1;
            break;
        case MAX:
            if (compareValues(max, attributeValue, selectionBinaryOperator))
                flag = 1;
            break;
        case SUM:
            if (compareValues(sum, attributeValue, selectionBinaryOperator))
                flag = 1;
            break;
        case COUNT:
            if (compareValues(count, attributeValue, selectionBinaryOperator))
                flag = 1;
            break;
        case AVG:
            if (compareValues(avg, attributeValue, selectionBinaryOperator))
                flag = 1;
            break;
        default:
            break;
        }
        if (flag)
        {
            vector<int> new_row;
            new_row.push_back(grouping_attribute_value);
            switch (aggregateFunc2)
            {
            case MIN:
                new_row.push_back(min);
                break;
            case MAX:
                new_row.push_back(max);
                break;
            case SUM:
                new_row.push_back(sum);
                break;
            case COUNT:
                new_row.push_back(count);
                break;
            case AVG:
                new_row.push_back(avg);
                break;
            default:
                break;
            }
            resultant_table->writeRow(new_row, fout);
        }
    }
    resultant_table->load();

    // unload sorted table
    tableCatalogue.deleteTable(this->tableName + "_sorted");

    // delete file from data folder
    string path = "../data/" + this->tableName + "_sorted.csv";
    remove(path.c_str());
}