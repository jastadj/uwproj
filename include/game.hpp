#ifndef CLASS_GAME
#define CLASS_GAME

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <irrlicht.h>

#include "level.hpp"

#define UNIT_SCALE 4
#define GRAVITY -0.5

//irrlicht namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

enum
{
    // I use this ISceneNode ID to indicate a scene node that is
    // not pickable by getSceneNodeAndCollisionPointFromRay()
    ID_IsNotPickable = 0,

    // I use this flag in ISceneNode IDs to indicate that the
    // scene node can be picked by ray selection.
    IDFlag_IsPickable = 1 << 0,

    // I use this flag in ISceneNode IDs to indicate that the
    // scene node can be highlighted.  In this example, the
    // homonids can be highlighted, but the level mesh can't.
    IDFlag_IsHighlightable = 1 << 1
};

class MyEventReceiver : public IEventReceiver
{

private:

    std::vector<const SEvent*> eventque;

    bool Keys[KEY_KEY_CODES_COUNT];

public:
    MyEventReceiver() {  }

    // This is the one method that we have to implement
    virtual bool OnEvent(const SEvent& event)
    {
        eventque.push_back(&event);

        //capture state of key presses
        if(event.EventType == EET_KEY_INPUT_EVENT)
        {
            Keys[event.KeyInput.Key] = event.KeyInput.PressedDown;
        }

        return false;
    }

    bool processEvents(const SEvent *&event)
    {
        //no events to process
        if(eventque.empty()) return false;

        //setting event pointer to current event in que
        event = eventque.back();

        //removing processed event from que
        eventque.erase(eventque.begin() + eventque.size()-1);

        return true;
    }

    bool isKeyPressed(EKEY_CODE keycode)
    {
        return Keys[keycode];
    }

    int queSize() { return int(eventque.size());}
};

class Game
{
private:
    Game();
    static Game *mInstance;

    //irrlicht renderer
    IrrlichtDevice *m_Device;
    IVideoDriver *m_Driver;
    ISceneManager *m_SMgr;
    ISceneCollisionManager *m_IMgr;
    IGUIEnvironment *m_GUIEnv;
    MyEventReceiver m_Receiver;
    ITriangleSelector *m_TriangleSelector;

    //camera
    void updateCamera(vector3df cameratargetpos);
    ICameraSceneNode *m_Camera;
    vector3df m_CameraPos;
    vector3df m_CameraTarget;
    f32 m_CameraDefaultFOV;

    //mouse
    vector2di m_MousePos;

    //mesh stuff
    SMesh *getCubeMesh(f32 cubesize);
    SMesh *getSquareMesh(int ul, int ur, int br, int bl);

    //init
    bool initIrrlicht();
    bool initCamera();
    bool loadLevel();
    bool loadPalette();
    bool loadGraphic(std::string tfilename, std::vector<ITexture*> *tlist);
    bool loadTexture(std::string tfilename, std::vector<ITexture*> *tlist);


    //levels
    int m_CurrentLevel;
    std::vector<Level> mLevels;
    std::vector<IMeshSceneNode*> mLevelMeshes;

    //palettes
    std::vector< std::vector<SColor> > m_Palettes;

    //textures
    std::vector<ITexture*> m_Wall64TXT;
    std::vector<ITexture*> m_Floor32TXT;
    std::vector<ITexture*> m_CharHeadTXT;

    //mainloop
    void mainLoop();

public:
    static Game *getInstance()
    {
        if(mInstance == NULL) mInstance = new Game;
        return mInstance;
    }
    ~Game();

    //start initialization
    int start();

    //mesh
    bool configMeshSceneNode(IMeshSceneNode *tnode);
    bool registerForCollision(IMeshSceneNode *tnode);

    //textures
    const std::vector<ITexture*> *getWall64Textures() const { return &m_Wall64TXT;}
    const std::vector<ITexture*> *getFloor32Textures() const { return &m_Floor32TXT;}

    //get irrlicht components
    ISceneManager *getSceneManager() { return m_SMgr;}
    IrrlichtDevice *getDevice() { return m_Device;}
    IVideoDriver *getDriver() { return m_Driver;}
    IGUIEnvironment *getGuiEnv() { return m_GUIEnv;}
    ITriangleSelector *getTriangleSelector() { return m_TriangleSelector;}
    ICameraSceneNode *getCamera() { return m_Camera;}

};
#endif // CLASS_GAME
