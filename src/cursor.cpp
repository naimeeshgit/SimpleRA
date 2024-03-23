#include "global.h"

Cursor::Cursor(string name, int pageIndex)
{
    logger.log("Cursor::Cursor");
    this->page = bufferManager.getPage(name, pageIndex);
    this->pagePointer = 0;
    if(INPUT_TYPE == TABLE)
        this->tableName = name;
    else
        this->matrixName = name;
    this->pageIndex = pageIndex;
}

/**
 * @brief This function reads the next row from the page. The index of the
 * current row read from the page is indicated by the pagePointer(points to row
 * in page the cursor is pointing to).
 *
 * @return vector<int> 
 */
vector<int> Cursor::getNext()
{
    logger.log("Cursor::getNext");
    vector<int> result = this->page.getRow(this->pagePointer);   
    this->pagePointer++;
    if(result.empty()){
        if(INPUT_TYPE == TABLE)
            tableCatalogue.getTable(this->tableName)->getNextPage(this);
        else
            matrixCatalogue.getMatrix(this->matrixName)->getNextPage(this);
        if(!this->pagePointer){
            result = this->page.getRow(this->pagePointer);
            this->pagePointer++;
        }
    }
    return result;
}
/**
 * @brief Function that loads Page indicated by pageIndex. Now the cursor starts
 * reading from the new page.
 *
 * @param pageIndex 
 */
void Cursor::nextPage(int pageIndex)
{
    logger.log("Cursor::nextPage");
    if(INPUT_TYPE == TABLE)
        this->page = bufferManager.getPage(this->tableName, pageIndex);
    else
        this->page = bufferManager.getPage(this->matrixName, pageIndex);
    this->pageIndex = pageIndex;
    this->pagePointer = 0;
}