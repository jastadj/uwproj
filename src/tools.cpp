#include "tools.hpp"

bool readBin(std::ifstream *fptr, unsigned char *data, int length, bool quiet)
{
    unsigned char *buf = new unsigned char[length];

    if(fptr == NULL) return false;;

    if(!quiet)
    {
        std::cout << "Start offset : 0x" << std::hex << fptr->tellg() << " (" << std::dec << fptr->tellg() << ")\n";
        std::cout << "Length       : " << length << std::endl;
    }

    if(fptr->eof())
    {
        std::cout << "END OF FILE REACHED!!  @ 0x" << std::hex << fptr->tellg() << std::dec << std::endl;
        return false;
    }

    fptr->read( (char*)(buf), length);

    for(int i = 0; i < length; i++)
    {
        data[i] = buf[i];
        if(!quiet) std::cout << std::hex << data[i] << ":" << int(data[i]) << std::endl;
    }

    return true;
}

bool readBinAt(std::ifstream *fptr, unsigned char *data, int length, std::streampos offset, bool quiet)
{
    if(fptr == NULL) return false;

    fptr->seekg(offset);
    return readBin(fptr, data, length, quiet);
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

int getCount(std::vector<int> ndata, int *curindex, int nibsize)
{
    int ccount = ndata[*curindex];
    //if count = 0, get next to nibbles and OR them
    if(ccount == 0)
    {
        ccount = (ndata[*curindex+1] << nibsize) | ndata[*curindex+2];
        *curindex += 2;

        //if count still = 0, get next 3 nibbles and get count
        if(ccount == 0)
        {
            ccount = (((ndata[*curindex+1] << nibsize*2) | ndata[*curindex+2]) << nibsize) | ndata[*curindex+3];
            *curindex += 3;

            //if count still = 0, there was a problem
            if(ccount == 0) return -1;
        }
    }
    return ccount;
}


irr::core::vector2df projectVectorAontoB(irr::core::vector2df va, irr::core::vector2df vb)
{

    float ndot(va.X*vb.X + va.Y*vb.Y);
    float magsqr( sqrt(vb.X*vb.X + vb.Y*vb.Y) );

    magsqr = magsqr*magsqr;

    return irr::core::vector2df( (vb.X*ndot)/magsqr, (vb.Y*ndot)/magsqr);


}
