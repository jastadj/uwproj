#ifndef CLASS_MOUSE
#define CLASS_MOUSE

#include <irrlicht.h>
#include "game.hpp"

//irrlicht namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

//forward declarations
class Game;

class Mouse
{
private:

    //engine references
    Game *gptr;
    IVideoDriver *m_Driver;
    IrrlichtDevice *m_Device;

public:
    Mouse(Game *tgame);
    ~Mouse();

    vector2di m_MousePos;
    line3d<f32> m_CameraMouseRay;

    ITexture *m_Texture;
    void setTexture(ITexture *ttxt);
    void draw();

    void updatePosition();
    int getMousePositionX() { return m_MousePos.X;}
    int getMousePositionY() { return m_MousePos.Y;}
    vector2di *getMousePosition() { return &m_MousePos;}
};

#endif // CLASS_MOUSE
