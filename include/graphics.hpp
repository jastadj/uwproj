#ifndef CLASS_GRAPHICS
#define CLASS_GRAPHICS

#include "irrcommon.hpp"
#include <string>
#include <vector>

#define TRANSPARENCY_COLOR 0,255,0,255

int loadPalette(std::vector< std::vector<SColor> > *pals);
int loadAuxPalette(std::vector< std::vector<SColor> > *pals);

int loadGraphic(std::string tfilename, std::vector<ITexture*> *tlist);
int loadTexture(std::string tfilename, std::vector<ITexture*> *tlist);
int loadBitmap(std::string tfilename, std::vector<ITexture*> *tlist, int tpalindex);

#endif // CLASS_GRAPHICS
