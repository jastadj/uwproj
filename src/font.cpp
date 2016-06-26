#include "font.hpp"

#include <fstream>
#include <sstream>
#include "tools.hpp"
#include "game.hpp"

Game *gptr = Game::getInstance();


int loadFont(std::string tfilename, UWFont *font)
{
    //note : this assume a 64x64 font image

    //height and width of texture sheet
    const int sheetdim = 12;

    if(font == NULL) return -1; // font is null

    //get driver ref
    IVideoDriver *m_Driver = gptr->getDriver();

    //open font (.sys) file
    std::ifstream ifile;
    ifile.open(tfilename.c_str(), std::ios_base::binary);

    //check if file loaded properly
    if(!ifile.is_open()) return -2; // error unable to open file

    //determine file size
    std::streampos fsize = 0;
    ifile.seekg(0, std::ios::end);
    fsize = ifile.tellg();
    ifile.seekg(0);

    //store binary font data
    std::vector< std::vector<bool> > fontbin;

    //read in header
    //unknown byte
    unsigned char unkbuf[2];
    int unkbyte = 0;
    readBin(&ifile, unkbuf, 2);
    unkbyte = lsbSum(unkbuf, 2);

    //character size (in bytes)
    unsigned char charsizebuf[2];
    int charbytes = 0;
    readBin(&ifile, charsizebuf, 2);
    charbytes = lsbSum(charsizebuf, 2);

    //width of blank space character
    unsigned char wblankspacebuf[2];
    int blankwidthpx = 0;
    readBin(&ifile, wblankspacebuf, 2);
    blankwidthpx = lsbSum(wblankspacebuf, 2);

    //font height
    unsigned char fontheightbuf[2];
    int heightpx = 0;
    readBin(&ifile, fontheightbuf, 2);
    heightpx = lsbSum(fontheightbuf, 2);

    //width of character row in bytes
    unsigned char wcharrowsizebuf[2];
    int widthbytes = 0;
    readBin(&ifile, wcharrowsizebuf, 2);
    widthbytes = lsbSum(wcharrowsizebuf, 2);

    //max width of character in pixels
    unsigned char maxpixelwidthbuf[2];
    int maxwidthpx = 0;
    readBin(&ifile, maxpixelwidthbuf, 2);
    maxwidthpx = lsbSum(maxpixelwidthbuf, 2);

    //characters to read
    int charstoread = (fsize - 12) / (charbytes + 1); // header is 12 bytes, charsize + 1 currentcharwidth byte

    //set fonts character count
    font->m_Count = charstoread;

    //set font height
    font->m_Height = heightpx * SCREEN_SCALE;

    //debug info
    std::cout << std::hex;
    std::cout << "unk val           = 0x" << unkbyte << std::endl;
    std::cout << "char bytes        = 0x" << charbytes << std::endl;
    std::cout << "width bytes       = 0x" << widthbytes << std::endl;
    std::cout << "blank width px    = 0x" << blankwidthpx << std::endl;
    std::cout << "height px         = 0x" << heightpx << std::endl;
    std::cout << "max width px      = 0x" << maxwidthpx << std::endl;
    std::cout << std::dec;

    int widestcharacter = maxwidthpx; // when creating texture, assume each character as widest font
                             // the UWFont rect will determine clip size
    std::vector<int> charwidths;

    //STORE FONT IN MONO TEXTURE
    // note : texture is stored in uwfont object
    IImage *newimg = NULL;

    //read in each character data (charbytes + 1 byte)
    for(int j = 0; j < charstoread; j++)
    {
        bool isblankspace = true;

        //read in binary font data
        unsigned char fontdatabuf[charbytes];
        readBin(&ifile, fontdatabuf, charbytes);
        fontbin.push_back( printByteToBin(fontdatabuf, charbytes));

        //read in current character width in pixels
        unsigned char currentwidthpxbuf[1];
        int currentwidthpx = 0;
        readBin(&ifile, currentwidthpxbuf, 1);
        currentwidthpx = int(currentwidthpxbuf[0]);
        if(currentwidthpx > widestcharacter) widestcharacter = currentwidthpx; // find widest character

        //if current width is 0, assume maximum pixel width value
        if(currentwidthpx == 0) currentwidthpx = maxwidthpx;

        //determine if character is blank character (no data)
        for(int i = 0; i < int(fontbin[j].size()); i++)
        {
            //search for any data found, then it's not a blank space
            if(fontbin[j][i] && isblankspace) isblankspace = false;

            //if font is determined not to be a blank space, stop looking
            if(!isblankspace) break;
        }

        //if is a blank space, set current pixel width to blank space width
        if(isblankspace) currentwidthpx = blankwidthpx;

        //store character width
        charwidths.push_back(currentwidthpx);


    }

    //save widest character value
    font->m_WidestCharacter = widestcharacter;

    //create clip rects
    for(int j = 0; j < charstoread; j++)
    {
        //create font clipping rect
        core::rect<s32> fontrect(position2d<s32>( int(j%sheetdim) * widestcharacter * SCREEN_SCALE, int(j/sheetdim) * heightpx * SCREEN_SCALE),
                                 dimension2d<u32>(charwidths[j]*SCREEN_SCALE, heightpx*SCREEN_SCALE));
        font->m_Clips.push_back(fontrect);
    }

    //create new image and copy binary font data to image
    newimg = m_Driver->createImage(ECF_A1R5G5B5, dimension2d<u32>(widestcharacter * sheetdim * SCREEN_SCALE, heightpx * sheetdim * SCREEN_SCALE));
    newimg->fill(SColor(TRANSPARENCY_COLOR) );
    if(newimg == NULL) return -5; // error creating image

    for(int i = 0; i < int(fontbin.size()); i++)
    {
        int binpos = 0;

        for(int n = 0; n < heightpx; n++)
        {
            for(int k = binpos; k < binpos + widthbytes*8; k++)
            {
                //if reading beyond current font width, ignore
                if(k - binpos >= font->m_Clips[i].getWidth()) continue;

                if(fontbin[i][k])
                {
                    for(int q = 0; q < SCREEN_SCALE; q++)
                    {
                        for(int w = 0; w < SCREEN_SCALE; w++)
                        {
                            newimg->setPixel( font->m_Clips[i].UpperLeftCorner.X + ((k - binpos)*SCREEN_SCALE)+w,
                                              font->m_Clips[i].UpperLeftCorner.Y + (int(k/(widthbytes*8))*SCREEN_SCALE)+q,
                                              SColor(255,255,255,255));
                        }
                    }

                }
            }

            //advance binary position
            binpos += widthbytes*8;
        }
    }

    //create scaled image
    //IImage *scaledimage = m_Driver->createImage(ECF_A1R5G5B5, dimension2d<u32>(widestcharacter * sheetdim * SCREEN_SCALE, heightpx * sheetdim * SCREEN_SCALE));
    //newimg->copyToScaling(scaledimage);


    //create texture name
    std::stringstream texturename;
    texturename << "font";

    //create texture from image
    //font->m_Texture = m_Driver->addTexture( texturename.str().c_str(), scaledimage );
    font->m_Texture = m_Driver->addTexture( texturename.str().c_str(), newimg );

    //set transparency color (pink, 255,0,255)
    //note : this is palette index #0, set automatically when
    //       loading in palettes (see loadPalette())
    m_Driver->makeColorKeyTexture(font->m_Texture,  SColor(TRANSPARENCY_COLOR));

    if(font->m_Texture == NULL) return -12; // error creating texture

    //drop image, no longer needed
    newimg->drop();
    //scaledimage->drop();

    ifile.close();

    return 0;
}


///////////////////////////////////////////////////////////////////////////////////
//  FONT DRAWING
bool drawFontChar(UWFont *tfont, int charnum, position2d<s32> tpos, SColor tcolor)
{
    if(tfont == NULL) return false;

    if(charnum < 0 || charnum >= 127) return false;

    gptr->getDriver()->draw2DImage( tfont->m_Texture,
                          tpos,
                          tfont->m_Clips[charnum],
                          NULL,
                          tcolor,
                          true);
    return true;
}

bool drawFontString(UWFont *tfont, std::string tstring, position2d<s32> tpos, SColor tcolor)
{
    if(tfont == NULL) return false;

    //draw each character in string
    for(int i = 0; i < tstring.length(); i++)
    {
        int charval = int(tstring[i]);

        //if able to draw character, advance position by characters width
        if(drawFontChar(tfont, charval, tpos, tcolor))
        {
            //advance tpos by character width
            tpos.X += tfont->m_Clips[charval].getWidth();
        }
    }

    return true;
}
