#ifndef CLASS_GAME
#define CLASS_GAME

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>


#include "irrcommon.hpp"

#include "event.hpp"
#include "strings.hpp"
#include "graphics.hpp"
#include "level.hpp"
#include "object.hpp"
#include "font.hpp"
#include "thread.hpp"
#include "mouse.hpp"
#include "scroll.hpp"

#define DEBUG_NO_START 0
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

enum
{
    // I use this ISceneNode ID to indicate a scene node that is
    // not pickable by getSceneNodeAndCollisionPointFromRay()
    ID_IsNotPickable = 0,

    // I use this flag in ISceneNode IDs to indicate that the
    // scene node can be picked by ray selection.
    ID_IsMap = 1 << 0,

    // I use this flag in ISceneNode IDs to indicate that the
    // scene node can be highlighted.  In this example, the
    // homonids can be highlighted, but the level mesh can't.
    ID_IsObject = 1 << 1
};



//forward declaration
class Mouse;
class Scroll;

class Game
{
private:
    Game();
    static Game *mInstance;

    bool m_DoShutdown;

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
    int m_LightRadius;
    SLight m_LightData;

    //threads
    std::vector<MyThreadClass*> m_Threads;

    //mouse
    Mouse *m_Mouse;

    //mesh stuff
    SMesh *getCubeMesh(f32 cubesize);
    SMesh *getSquareMesh(int ul, int ur, int br, int bl);

    //init / load
    void loadScreen(std::string loadmessage);
    int initIrrlicht();
    int initCamera();
    int initMouse();
    int initObjects();
    int initMainUI();


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
    std::vector<ITexture*> m_InventoryTXT;
    std::vector<ITexture*> m_ScrollEdgeTXT;

    //fonts
    UWFont m_FontNormal;

    //strings
    std::vector<stringBlock> m_StringBlocks;

    //objects
    std::vector<Object*> m_Objects;

    //mainloop
    void mainLoop();

    //main UI
    int drawMainUI();
    Scroll *m_Scroll;

    //debug
    bool dbg_noclip;
    bool dbg_nolighting;
    bool dbg_showboundingbox;
    bool dbg_showmainui;
    bool dbg_dodrawpal;
    void dbg_drawpal(std::vector<SColor> *tpal);
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
    int getCurrentLevel() { return m_CurrentLevel;}

    //textures
    const std::vector<ITexture*> *getWall64Textures() const { return &m_Wall64TXT;}
    const std::vector<ITexture*> *getFloor32Textures() const { return &m_Floor32TXT;}
    ITexture *getDefaultTexture() { return m_QuestionTXT[0];}
    std::vector< std::vector<SColor> > *getPalletes() { return &m_Palettes;}
    std::vector< std::vector<SColor> > *getAuxPalletes() { return &m_AuxPalettes;}

    //objects
    bool updateObject(ObjectInstance *tobj, Tile *ttile);
    Object *getObject(int id);

    //strings
    std::string getDefaultString() { return "no string";}

    //fonts
    UWFont *getNormalFont() { return &m_FontNormal;}

    //get irrlicht components
    ISceneManager *getSceneManager() { return m_SMgr;}
    IrrlichtDevice *getDevice() { return m_Device;}
    IVideoDriver *getDriver() { return m_Driver;}
    IGUIEnvironment *getGuiEnv() { return m_GUIEnv;}
    IMetaTriangleSelector *getMetaTriangleSelector() { return m_MetaTriangleSelector;}
    ICameraSceneNode *getCamera() { return m_Camera;}

};
#endif // CLASS_GAME
