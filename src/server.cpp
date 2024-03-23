//Server Code
#include "global.h"
#include <regex>
#include <sstream>

using namespace std;

float BLOCK_SIZE = 0.04;
uint BLOCK_COUNT = 2;
uint PRINT_COUNT = 20;
Logger logger;
vector<string> tokenizedQuery;
ParsedQuery parsedQuery;
TableCatalogue tableCatalogue;
MatrixCatalogue matrixCatalogue;
BufferManager bufferManager;
int INPUT_TYPE = TABLE;
int BLOCKS_READ = 0;
int BLOCKS_WRITE = 0;

int main(void)
{

    std::regex delim("[^\\s,]+");
    string command;
    system("rm -rf ../data/temp");
    system("mkdir ../data/temp");

    while(!cin.eof())
    {
        cout << "\n> ";
        tokenizedQuery.clear();
        parsedQuery.clear();
        BLOCKS_READ = 0;
        BLOCKS_WRITE = 0;
        logger.log("\nReading New Command: ");
        getline(cin, command);
        logger.log(command);


        auto words_begin = std::sregex_iterator(command.begin(), command.end(), delim);
        auto words_end = std::sregex_iterator();
        for (std::sregex_iterator i = words_begin; i != words_end; ++i)
            tokenizedQuery.emplace_back((*i).str());

        if (tokenizedQuery.size() == 1 && tokenizedQuery.front() == "QUIT")
        {
            break;
        }

        if (tokenizedQuery.empty())
        {
            continue;
        }

        if (tokenizedQuery.size() == 1)
        {
            cout << "SYNTAX ERROR" << endl;
            continue;
        }

    if (syntacticParse() && semanticParse())
        executeCommand();
        cout << "Number of blocks read: " << BLOCKS_READ << endl;
        cout << "Number of blocks written: " << BLOCKS_WRITE << endl;
        cout << "Number of blocks accessed: " << BLOCKS_READ + BLOCKS_WRITE << endl;

    }
}