#include "executor.h"
#define TABLE 0
#define MATRIX 1
#define SUBMATRIX_DIM 15
#define SORTING_BUFFER_SIZE 3
extern float BLOCK_SIZE;
extern uint BLOCK_COUNT;
extern uint PRINT_COUNT;
extern vector<string> tokenizedQuery;
extern ParsedQuery parsedQuery;
extern TableCatalogue tableCatalogue;
extern MatrixCatalogue matrixCatalogue;
extern BufferManager bufferManager;
extern int INPUT_TYPE;
extern int BLOCKS_READ;
extern int BLOCKS_WRITE;
