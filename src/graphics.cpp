#include "graphics.hpp"
#include <fstream>
#include <sstream>

#include "tools.hpp"
#include "game.hpp"

int loadPalette(std::vector< std::vector<SColor> > *pals)
{
    std::string palfile = "UWDATA\\pals.dat";
    std::ifstream pfile;
    pfile.open(palfile.c_str(), std::ios_base::binary);

    //check if file loaded
    if(!pfile.is_open()) return -1; // error unable to open file

    //resize palettes for 256
    pals->resize(8);
    for(int i = 0; i < int(pals->size()); i++) (*pals)[i].resize(256);

    //UW palette intensities are 64, mult by 4 to get 256 intensity
    // (3x 8-bit r, g, b) = 1 index
    // each palette has 256 indices
    // pal file has 8 palettes

    for(int i = 0; i < int(pals->size()); i++)
    {
        for(int p = 0; p < int((*pals)[i].size()); p++)
        {
            unsigned char rgb[3];

            //read in color data (0-63 intensity for red, green , and blue)
            readBin(&pfile, rgb, 3);

            //index 0 always = transparent
            if(p == 0) (*pals)[i][p] = SColor(TRANSPARENCY_COLOR );
            else (*pals)[i][p] = SColor(255, (int(rgb[0])+1)*4-1, (int(rgb[1])+1)*4-1, (int(rgb[2])+1)*4-1 );

        }
    }

    pfile.close();

    return 0;
}

int loadAuxPalette(std::vector< std::vector<SColor> > *pals)
{
    std::string palfile = "UWDATA\\allpals.dat";
    std::ifstream pfile;
    pfile.open(palfile.c_str(), std::ios_base::binary);

    //get references
    Game *gptr = NULL;
    gptr = Game::getInstance();

    std::vector< std::vector<SColor> > *mainpal = gptr->getPalletes();

    // aux palettes are indices into palette #0, so make sure it exists first!
    if(mainpal->empty()) return -2;

    //check if file loaded
    if(!pfile.is_open()) return -1; // error unable to open file

    //resize palettes for 16 colors x 31 palettes
    pals->resize(31);
    for(int i = 0; i < int(pals->size()); i++) (*pals)[i].resize(16);

    //UW palette intensities are 64, mult by 4 to get 256 intensity
    // (3x 8-bit r, g, b) = 1 index
    // each palette has 256 indices
    // pal file has 8 palettes

    for(int i = 0; i < int(pals->size()); i++)
    {
        for(int p = 0; p < int((*pals)[i].size()); p++)
        {
            unsigned char palindex[1];

            //read in color data (0-63 intensity for red, green , and blue)
            if(!readBin(&pfile, palindex, 1)) return -3; // error reading aux pal byte

            //index 0 always = transparent
            (*pals)[i][p] = (*mainpal)[0][int(palindex[0])];
        }
    }

    pfile.close();

    return 0;
}

int loadTexture(std::string tfilename, std::vector<ITexture*> *tlist)
{
    if(tlist == NULL) return -1; //error texture list is null

    //get game reference
    Game *gptr = NULL;
    gptr = Game::getInstance();

    //open texture file
    std::ifstream ifile;
    ifile.open(tfilename.c_str(), std::ios_base::binary);

    //check if texture file loaded properly
    if(!ifile.is_open()) return -2; // error unable to open file


    //temp variables
    unsigned char headerbuf[2];
    unsigned char txtcountbuf[2];
    std::vector<std::streampos> offsets;
    int txtdim = 0;
    int txtcount = 0;

    //read texture header
    readBin(&ifile, headerbuf, 2);
    readBin(&ifile, txtcountbuf, 2);

    //set variable data from header
    txtdim = int(headerbuf[1]);
    txtcount = lsbSum(txtcountbuf, 2);

    //read in texture offsets
    for(int i = 0; i < txtcount; i++)
    {
        unsigned char offsetbuf[4];
        readBin(&ifile, offsetbuf, 4);

        //store texture offsets into indexed list
        offsets.push_back(std::streampos(lsbSum(offsetbuf, 4)) );

        //std::cout << std::dec << "texture offset " << i << ": 0x" << std::hex << offsets.back() << std::endl;
    }

    //read each texture from file offset into an opengl texture
    for(int i = 0; i < txtcount; i++)
    {
        ITexture *newtxt = NULL;
        IImage *newimg = NULL;
        int palSel = 0; //  wall/floor textures always use palette 0

        //set the stream position to offset
        ifile.seekg(offsets[i]);

        //create image using texture dimension size
        newimg = gptr->getDriver()->createImage(ECF_A1R5G5B5, dimension2d<u32>(txtdim, txtdim));

        //offset points to dim^2 bytes long data where each byte points to palette index
        for(int n = 0; n < txtdim; n++)
        {
            for(int p = 0; p < txtdim; p++)
            {
                unsigned char pindex[1];

                //error reading?
                if(!readBin(&ifile, pindex, 1))
                {
                    std::cout << "Error reading texture at 0x" << std::hex << offsets[i] << std::endl;
                    return -8; // error reading texture at offset
                }


                //set the pixel at x,y using current selected palette with read in palette index #
                newimg->setPixel(p, n, (*gptr->getPalletes())[palSel][int(pindex[0])]);
            }
        }

        //create texture name
        std::stringstream texturename;
        texturename << "txt_" << i;

        //create texture from image
        newtxt = gptr->getDriver()->addTexture( texturename.str().c_str(), newimg );

        //push texture into texture list
        tlist->push_back(newtxt);

        //drop image, no longer needed
        newimg->drop();
    }
    ifile.close();

    return 0;
}

int loadGraphic(std::string tfilename, std::vector<ITexture*> *tlist)
{
    if(tlist == NULL) return false;

    //get game reference
    Game *gptr = NULL;
    gptr = Game::getInstance();

    //open graphic (.gr) file
    std::ifstream ifile;
    ifile.open(tfilename.c_str(), std::ios_base::binary);

    //check if graphic file loaded properly
    if(!ifile.is_open()) return -1; // error loading file

    //check if palettes have been loaded first
    if( gptr->getPalletes()->empty() || gptr->getAuxPalletes()->empty()) return -13; // error palettes are empty

    //temp vars
    unsigned char fformatbuf[1];
    unsigned char bitmapcntbuf[2];
    std::vector<std::streampos> offsets;
    //int fformat = 0;
    int bitmapcnt = 0;

    //read header data
    if(!readBin(&ifile, fformatbuf, 1)) return -2; // error reading header format
    //fformat = int(fformatbuf[0]);
    if(!readBin(&ifile, bitmapcntbuf, 2) ) return -3; // error reading header bitmap count
    bitmapcnt = lsbSum(bitmapcntbuf, 2);

    //for each bitmap count, read in offsets
    for(int i = 0; i < bitmapcnt; i++)
    {

        unsigned char offsetbuf[4];
        if(!readBin(&ifile, offsetbuf, 4)) return -4; // error reading offset

        offsets.push_back( std::streampos( lsbSum(offsetbuf, 4)));

        //std::cout << "Offset " << i << " = 0x" << std::hex << offsets.back() << std::dec << std::endl;
    }

    //if bitmap count and offset count do not match, something went wrong!
    if(bitmapcnt != int(offsets.size()) )
    {
        std::cout << std::dec << "Graphics file bitmap count (" << bitmapcnt << ") != offset count (" << offsets.size() << ") !!\n";
        ifile.close();
        return false;
    }

    //read in each bitmap at offset
    for(int i = 0; i < bitmapcnt; i++)
    {

        //each bitmap at offset has its own header
        unsigned char btypebuf[1];
        unsigned char bwidthbuf[1];
        unsigned char bheightbuf[1];
        unsigned char bauxpalbuf[1];
        unsigned char bsizebuf[2];
        int btype;
        int bwidth;
        int bheight;
        int bauxpal;
        int bsize;

        //texture
        ITexture *newtxt = NULL;
        IImage *newimg = NULL;
        int palSel = 0; //  note : 4-bit images use aux pals, standard images use pal 0

        //jump to offset
        ifile.seekg(offsets[i]);

        //read in header and set data
        //bitmap data is read differently depending on what bitmap type it is
        // types are:
        //          0x04 = 8bit uncompressed
        //          0x08 = 4bit run length
        //          0x0A = 4bit uncompressed
        if(!readBin(&ifile, btypebuf, 1)) return -6; // error reading bitmap type
        btype = int(btypebuf[0]);

        if(!readBin(&ifile, bwidthbuf, 1))
        {
            std::cout << "Error reading binary file " << tfilename << " at offset " << std::hex << "0x" << offsets[i] << std::endl;
            std::cout << "Ignoring...\n";
            ifile.clear();
            ifile.close();
            std::cout << std::dec;
            return 0;
            return -7; // error reading bitmap width
        }
        bwidth = int(bwidthbuf[0]);

        if(!readBin(&ifile, bheightbuf, 1)) return -8; // error reading bitmap height
        bheight = int(bheightbuf[0]);


        //create new image using bitmap dimensions
        newimg = gptr->getDriver()->createImage(ECF_A1R5G5B5, dimension2d<u32>(bwidth, bheight));
        if(newimg == NULL) return -5; // error creating image

        //NOTE 4-bit images also have an auxillary palette selection byte
        //if 4-bit uncompressed, read in aux pal byte
        if(btype == 0x0a || btype == 0x08)
        {
            if(!readBin(&ifile, bauxpalbuf, 1)) return -9; // error reading auxillary palette
            bauxpal = int(bauxpalbuf[0]);
        }

        //get size
        // note : for 4 bit, this is nibble count, not byte count
        //if not uncompressed, read in size
        if(!readBin(&ifile, bsizebuf, 2)) return -10; // error reading size
        bsize = lsbSum(bsizebuf, 2);

        //read bitmap data
        //if uncompressed format - 8 bit
        if(btype == 0x04)
        {
            for(int n = 0; n < bheight; n++)
            {
                for(int p = 0; p < bwidth; p++)
                {
                    //read in one byte at a time
                    unsigned char bbyte[1];

                    if(!readBin(&ifile, bbyte, 1)) return -11; // error reading image data

                        //set the pixel at x,y using current selected palette with read in palette index #
                        newimg->setPixel(p, n, (*gptr->getPalletes())[palSel][int(bbyte[0])]);

                }
            }
        }
        //else if uncompressed format - 4bit
        else if(btype == 0x0a)
        {
            //read in 4-bit stream using size for nibble count
            std::vector<int> nibbles;
            nibbles.resize(bwidth*bheight);

            //read in entire stream
            unsigned char bstream[bsize];
            readBin(&ifile, bstream, bsize);

            //parse each byte by nibble, high nibble first, then lo
            for(int k = 0; k < bsize; k++)
            {
                int lobyte = getBitVal( int(bstream[k]), 0, 4);
                int hibyte = getBitVal( int(bstream[k]), 4, 4);
                nibbles[k*2] = hibyte;
                nibbles[k*2+1] = lobyte;
            }

            //set image pixel using image height and width to pull from nibble index
            for(int n = 0; n < bheight; n++)
            {
                for(int p = 0; p < bwidth; p++)
                {
                    newimg->setPixel(p, n, (*gptr->getAuxPalletes())[bauxpal][  nibbles[ (n*bwidth) + p]  ]);
                }
            }
        }
        // compressed bitmap
        else if(btype == 0x08)
        {
            std::vector<int> nibbledata;
            std::vector<int> pixeldata;

            int nibindex = 0;
            int nibsize = 4;


            for(int n = 0; n < bsize; n++)
            {
                unsigned char nbuf[1];
                if(!readBin(&ifile, nbuf, 1)) {ifile.close(); return -20;}

                nibbledata.push_back( getBitVal(int(nbuf[0]), 4, 4 ) );
                n++;
                if(n < bsize) nibbledata.push_back( getBitVal(int(nbuf[0]), 0, 4 ) );
            }


            //debug
            /*
            for(int n = 0; n < int(nibbledata.size()); n++)
            {
                std::cout << n << " : " << nibbledata[n] << std::endl;
            }
            */

            bool dorunrecord = false;

            do
            {
                //get count
                int rcount = getCount(nibbledata, &nibindex);
                /*
                if(!dorunrecord) std::cout << "repeat -- ";
                else std::cout << "run    -- ";
                std::cout << "index = " << nibindex << "   --   count = " << rcount << std::endl;
                */

                //if count is 2, do repeat
                if(rcount == 2 && !dorunrecord)
                {
                    //get repeat count
                    nibindex++;
                    int repeatcount = getCount(nibbledata, &nibindex);

                    //do repeat
                    for(int n = 0; n < repeatcount; n++)
                    {
                        nibindex++;
                        int altcount = getCount(nibbledata, &nibindex);

                        nibindex++;
                        for(int k = 0; k < altcount; k++) pixeldata.push_back(nibbledata[nibindex]);
                    }
                }
                //process count
                else if(rcount > 1 && !dorunrecord)
                {
                    nibindex++;
                    //repeat nibble by count times
                    for(int n = 0; n < rcount; n++) pixeldata.push_back(nibbledata[nibindex]);
                }
                else if(dorunrecord)
                {
                    //read in raw pixel data
                    for(int n = 0; n < rcount; n++)
                    {
                        nibindex++;
                        pixeldata.push_back( nibbledata[nibindex]);
                    }
                }
                //else if count == 1, ignore this record

                //change modes between run / repeat record
                dorunrecord = !dorunrecord;

                nibindex++;
            }while(nibindex < bsize); // done parsing nibbles


            //create image data
            for(int n = 0; n < bheight; n++)
            {
                for(int k = 0; k < bwidth; k++)
                {
                    newimg->setPixel(k, n, (*gptr->getAuxPalletes())[bauxpal][  pixeldata[ (n*bwidth) + k]  ]);
                }
            }


        }
        else
        {
            std::cout << "Unrecognized graphic type : " << std::hex << "0x" << btype << std::dec << std::endl;
            ifile.close();
            return -14;
        }


        //create texture name
        std::stringstream texturename;
        texturename << "txt_" << i;



        //create texture from image
        IImage *stretchedimage = gptr->getDriver()->createImage(ECF_A1R5G5B5, dimension2d<u32>(bwidth*SCREEN_SCALE, bheight*SCREEN_SCALE));
        newimg->copyToScaling(stretchedimage);
        newtxt = gptr->getDriver()->addTexture( texturename.str().c_str(), stretchedimage );
        //set transparency color (pink, 255,0,255)
        //note : this is palette index #0, set automatically when
        //       loading in palettes (see loadPalette())
        gptr->getDriver()->makeColorKeyTexture(newtxt,  SColor(TRANSPARENCY_COLOR));

        if(newtxt == NULL) return -12; // error creating texture
        //push texture into texture list
        tlist->push_back(newtxt);

        //drop image, no longer needed
        newimg->drop();
        stretchedimage->drop();
    }

    ifile.close();

    std::cout << std::dec;

    return 0;
}

int loadBitmap(std::string tfilename, std::vector<ITexture*> *tlist, int tpalindex)
{
    const int bitmap_width = 320;
    const int bitmap_height = 200;

    if(tlist == NULL) return -1; // vector list is null

    //get game reference
    Game *gptr = NULL;
    gptr = Game::getInstance();

    //open bitmap (.byt) file
    std::ifstream ifile;
    ifile.open(tfilename.c_str(), std::ios_base::binary);

    //check if bitmap file loaded properly
    if(!ifile.is_open()) return -2; // error unable to open file

    //check if palette is valid
    if(tpalindex < 0 || tpalindex >= int( gptr->getPalletes()->size()) )
    {
        std::cout << "Error loading bitmap : Invalid palette index # - " << tpalindex << std::endl;
        return -3;
    }

    //texture
    ITexture *newtxt = NULL;
    IImage *newimg = NULL;
    //create new image using bitmap dimensions
    newimg = gptr->getDriver()->createImage(ECF_A1R5G5B5, dimension2d<u32>(bitmap_width, bitmap_height));

    //read in each bitmap byte
    for(int i = 0; i < bitmap_height; i++)
    {
        for(int n = 0; n < bitmap_width; n++)
        {
            unsigned char bbyte[1];

            readBin(&ifile, bbyte, 1);

            newimg->setPixel(n, i, (*gptr->getPalletes())[tpalindex][int(bbyte[0])]);
        }
    }

    //create texture name
    std::stringstream texturename;
    texturename << "txt_" << tfilename;

    //create texture from image
    IImage *stretchedimage = gptr->getDriver()->createImage(ECF_A1R5G5B5, dimension2d<u32>(bitmap_width*SCREEN_SCALE, bitmap_height*SCREEN_SCALE));
    newimg->copyToScaling(stretchedimage);
    newtxt = gptr->getDriver()->addTexture( texturename.str().c_str(), stretchedimage );
    //set transparency color (pink, 255,0,255)
    //note : this is palette index #0, set automatically when
    //       loading in palettes (see loadPalette())
    gptr->getDriver()->makeColorKeyTexture(newtxt,  SColor(TRANSPARENCY_COLOR));

    //push texture into texture list
    tlist->push_back(newtxt);

    //drop image, no longer needed
    newimg->drop();
    stretchedimage->drop();

    ifile.close();

    return 0;
}

