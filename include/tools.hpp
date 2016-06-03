#ifndef _TOOLS
#define _TOOLS

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>

void readBin(std::ifstream *fptr, unsigned char *data, int length, bool quiet = true);
void readBinAt(std::ifstream *fptr, unsigned char *data, int length, uint64_t offset, bool quiet = true);
int lsbSum(unsigned char *bytes, int length);
int getBitVal(int data, int startbit, int length);

#endif // _TOOLS
