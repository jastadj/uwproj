#ifndef CLASS_GAME
#define CLASS_GAME

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <irrlicht.h>

#include "level.hpp"
#include "object.hpp"

#define TRANSPARENCY_COLOR 0,255,0,255
#define FULLSCREEN 0
#define USE_OCTREE 1
#define CONFIG_FOR_COLLISION 1
#define SCREEN_SCALE 4
#define OBJECT_SCALE 1
#define SCREEN_WIDTH 320*SCREEN_SCALE
#define SCREEN_HEIGHT 200*SCREEN_SCALE
#define SCREEN_WORLD_POS_X 50*SCREEN_SCALE
#define SCREEN_WORLD_POS_Y 18*SCREEN_SCALE
#define SCREEN_WORLD_WIDTH 175*SCREEN_SCALE
#define SCREEN_WORLD_HEIGHT 114*SCREEN_SCALE
#define UNIT_SCALE 4
#define TILE_UNIT 8
#define GRAVITY_ACCEL 0.001
#define TERMINAL_GRAVITY 0.1
#define ROTATION_SPEED 120
#define MOVE_SPEED 15
#define STANDING_HEIGHT 3

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
    ID_IsPickable = 1 << 0,

    // I use this flag in ISceneNode IDs to indicate that the
    // scene node can be highlighted.  In this example, the
    // homonids can be highlighted, but the level mesh can't.
    ID_IsHighlightable = 1 << 1
};

struct stringBlock
{
    int id;
    std::vector<std::string> strings;
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
    ISceneCollisionManager *m_ColMgr;
    IGUIEnvironment *m_GUIEnv;
    MyEventReceiver m_Receiver;
    IMetaTriangleSelector *m_MetaTriangleSelector;
    ITriangleSelector *m_TriangleSelector;
    f32 frameDeltaTime;

    //camera
    void updateCamera();
    ICameraSceneNode *m_Camera;
    vector3df m_CameraPos;
    vector3df m_CameraRot;
    vector3df m_CameraVel;
    ISceneNode *m_CameraTarget;
    f32 m_CameraDefaultFOV;
    ILightSceneNode *m_CameraLight;

    //mouse
    vector2di m_MousePos;
    line3d<f32> m_CameraMouseRay;

    //mesh stuff
    SMesh *getCubeMesh(f32 cubesize);
    SMesh *getSquareMesh(int ul, int ur, int br, int bl);

    //init / load
    int initIrrlicht();
    int initCamera();
    int loadStrings();
    int loadLevel();
    int loadPalette();
    int loadAuxPalette();
    int loadGraphic(std::string tfilename, std::vector<ITexture*> *tlist);
    int loadTexture(std::string tfilename, std::vector<ITexture*> *tlist);
    int loadBitmap(std::string tfilename, std::vector<ITexture*> *tlist, int tpalindex);
    int initObjects();


    //levels
    int m_CurrentLevel;
    std::vector<Level> mLevels;
    std::vector<IMeshSceneNode*> mLevelMeshes;

    //palettes
    std::vector< std::vector<SColor> > m_Palettes;
    std::vector< std::vector<SColor> > m_AuxPalettes;

    //textures
    std::vector<ITexture*> m_Wall64TXT;
    std::vector<ITexture*> m_Floor32TXT;
    std::vector<ITexture*> m_CharHeadTXT;
    std::vector<ITexture*> m_BitmapsTXT;
    std::vector<ITexture*> m_CursorsTXT;
    std::vector<ITexture*> m_ObjectsTXT;
    std::vector<ITexture*> m_QuestionTXT;

    //strings
    std::vector<stringBlock> m_StringBlocks;

    //objects
    std::vector<Object*> m_Objects;

    //mainloop
    void mainLoop();

    //debug
    bool dbg_noclip;
    bool dbg_nolighting;
    bool dbg_showboundingbox;
    bool dbg_showmainui;
    void reconfigureAllLevelMeshes();
    void reconfigureAllLevelObjects();

public:
    static Game *getInstance()
    {
        if(mInstance == NULL) mInstance = new Game;
        return mInstance;
    }
    ~Game();

    //start initialization
    int start();

    //mesh stuff
    bool configMeshSceneNode(IMeshSceneNode *tnode);
    bool configBillboardSceneNode(IBillboardSceneNode *tnode);

    //world funcitons
    bool processCollision(vector3df *pos, vector3df *vel);

    //textures
    const std::vector<ITexture*> *getWall64Textures() const { return &m_Wall64TXT;}
    const std::vector<ITexture*> *getFloor32Textures() const { return &m_Floor32TXT;}
    ITexture *getDefaultTexture() { return m_QuestionTXT[0];}

    //objects
    bool updateObject(ObjectInstance *tobj, Tile *ttile);

    //strings
    std::string getDefaultString() { return "no string";}

    //get irrlicht components
    ISceneManager *getSceneManager() { return m_SMgr;}
    IrrlichtDevice *getDevice() { return m_Device;}
    IVideoDriver *getDriver() { return m_Driver;}
    IGUIEnvironment *getGuiEnv() { return m_GUIEnv;}
    IMetaTriangleSelector *getMetaTriangleSelector() { return m_MetaTriangleSelector;}
    ICameraSceneNode *getCamera() { return m_Camera;}

};
#endif // CLASS_GAME
