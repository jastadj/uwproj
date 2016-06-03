#include "tools.hpp"

void readBin(std::ifstream *fptr, unsigned char *data, int length, bool quiet)
{
    unsigned char *buf = new unsigned char[length];

    if(fptr == NULL) return;

    if(!quiet)
    {
        std::cout << "Start offset : 0x" << std::hex << fptr->tellg() << " (" << std::dec << fptr->tellg() << ")\n";
        std::cout << "Length       : " << length << std::endl;
    }


    fptr->read( (char*)(buf), length);

    for(int i = 0; i < length; i++)
    {
        data[i] = buf[i];
        if(!quiet) std::cout << std::hex << data[i] << ":" << int(data[i]) << std::endl;
    }

}

void readBinAt(std::ifstream *fptr, unsigned char *data, int length, uint64_t offset, bool quiet)
{
    if(fptr == NULL) return;

    fptr->seekg(std::streampos(offset));
    readBin(fptr, data, length, quiet);
}

int lsbSum(unsigned char *bytes, int length)
{
    int sum = 0;

    for(int i = 0; i < length; i++) sum += ( int(bytes[i]) << (i*8) );

    return sum;
}

int getBitVal(int data, int startbit, int length)
{
    //create mask
    int mask = 0;
    for(int i = 0; i < length; i++)
    {
        mask = mask << 0x01;
        mask = mask | 0x01;
    }

    return (data >> startbit) & mask;
}
