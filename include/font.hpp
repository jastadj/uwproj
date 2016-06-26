#ifndef CLASS_FONT
#define CLASS_FONT

#include <vector>
#include <string>
#include "irrcommon.hpp"

struct UWFont
{
    ITexture *m_Texture;
    std::vector<core::rect<s32> > m_Clips;
    int m_Height;
    int m_WidestCharacter;
    int m_Count;
};

int loadFont(std::string tfilename, UWFont *font);
bool drawFontChar(UWFont *tfont, int charnum, position2d<s32> tpos, SColor tcolor = SColor(255,255,255,255));
bool drawFontString(UWFont *tfont, std::string tstring, position2d<s32> tpos, SColor tcolor = SColor(255,255,255,255));


#endif // CLASS_FONT
