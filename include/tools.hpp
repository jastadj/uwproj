#ifndef _TOOLS
#define _TOOLS

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>

bool readBin(std::ifstream *fptr, unsigned char *data, int length, bool quiet = true);
bool readBinAt(std::ifstream *fptr, unsigned char *data, int length, std::streampos offset, bool quiet = true);
int lsbSum(unsigned char *bytes, int length);
int getBitVal(int data, int startbit, int length);

#endif // _TOOLS
