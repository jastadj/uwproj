#ifndef CLASS_STRINGS
#define CLASS_STRINGS

#include <fstream>
#include <vector>

//struct used to store huffman tree string nodes
struct hnode
{
    //1 byte each
    int chardata;
    int parent;
    int left;
    int right;
};

struct block
{
    //block header, total 6 bytes
    int blocknum; // 2 bytes
    std::streampos offset; // 4 bytes

    //string offset is relative position after end of block header
    int stringcount; // 2 bytes
    std::vector<std::streampos> stringoffsets; // 2 bytes
    std::vector<std::string> strings;
};

struct stringBlock
{
    int id;
    std::vector<std::string> strings;
};

int loadStrings(std::vector<stringBlock> *tblock);

#endif // CLASS_STRINGS
