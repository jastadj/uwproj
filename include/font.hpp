#ifndef CLASS_FONT
#define CLASS_FONT

#include <vector>

#include <irrlicht.h>

//irrlicht namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

struct UWFont
{
    ITexture *m_Texture;
    std::vector<core::rect<s32> > m_Clips;
};

#endif // CLASS_FONT
