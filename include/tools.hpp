#ifndef _TOOLS
#define _TOOLS

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <irrlicht.h>


bool readBin(std::ifstream *fptr, unsigned char *data, int length, bool quiet = true);
bool readBinAt(std::ifstream *fptr, unsigned char *data, int length, std::streampos offset, bool quiet = true);
int lsbSum(unsigned char *bytes, int length);
int getBitVal(int data, int startbit, int length);

int getCount(std::vector<int> ndata, int *curindex, int nibsize = 4);

irr::core::vector2df projectVectorAontoB(irr::core::vector2df va, irr::core::vector2df vb);

#endif // _TOOLS
