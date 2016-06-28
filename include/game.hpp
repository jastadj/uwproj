#ifndef CLASS_GAME
#define CLASS_GAME

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>


#include "irrcommon.hpp"
#include "console.hpp"
#include "event.hpp"
#include "strings.hpp"
#include "graphics.hpp"
#include "level.hpp"
#include "object.hpp"
#include "font.hpp"
#include "thread.hpp"
#include "mouse.hpp"
#include "scroll.hpp"
#include "player.hpp"

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


//forward declaration
class Mouse;
class Scroll;
class MyEventReceiver;
class Console;

enum {ID_IsNotPickable = 0, ID_IsMap = 1 << 0, ID_IsObject = 1 << 1};
enum {IMODE_PLAY, IMODE_SCROLL_ENTRY, IMODE_TOTAL};

class Game
{
private:
    Game();
    static Game *mInstance;
    Console *m_Console;
    bool m_DoShutdown;

    //irrlicht renderer
    IrrlichtDevice *m_Device;
    IVideoDriver *m_Driver;
    ISceneManager *m_SMgr;
    ISceneCollisionManager *m_ColMgr;
    IGUIEnvironment *m_GUIEnv;
    IMetaTriangleSelector *m_MetaTriangleSelector;
    ITriangleSelector *m_TriangleSelector;
    f32 frameDeltaTime;

    //camera
    void updateCamera();
    ICameraSceneNode *m_Camera;

    ISceneNode *m_CameraTarget;
    f32 m_CameraDefaultFOV;
    ILightSceneNode *m_CameraLight;
    int m_LightRadius;
    SLight m_LightData;

    //player
    Player *m_Player;

    //threads
    std::vector<MyThreadClass*> m_Threads;

    //mesh stuff
    SMesh *getCubeMesh(f32 cubesize);
    SMesh *getSquareMesh(int ul, int ur, int br, int bl);

    //init / load
    void loadScreen(std::string loadmessage);
    int initIrrlicht();
    int initCamera();
    int initMouse();
    int initObjects();
    int initPlayer();
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
    std::vector<ITexture*> m_ModeButtonsTXT;
    std::vector<ITexture*> m_ModeButtonsMiscTXT;
    std::vector<ITexture*> m_DragonsTXT;

    //fonts
    UWFont m_FontNormal;

    //strings
    std::vector<stringBlock> m_StringBlocks;

    //objects
    std::vector<Object*> m_Objects;

    //mainloop

    void mainLoop();


    //input
    MyEventReceiver *m_Receiver;
    Mouse *m_Mouse;
    int m_InputContext;
    int m_PreviousInputContext;
    void handleInputs();
    void processEvent(const SEvent *event);


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
    void dbg_stringdump();
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

    //world functions
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
    std::string lookAtObject(ObjectInstance *tobj);

    //strings
    std::string getDefaultString() { return "no string";}
    std::string getString(int blockindex, int stringindex);

    //fonts
    UWFont *getNormalFont() { return &m_FontNormal;}

    //
    int getInputContext() { return m_InputContext;}
    int getPreviousInputContext() { return m_PreviousInputContext;}
    bool setInputContext(int ncontext);

    //scroll messages
    void addMessage(std::string msgstring, int fonttype = 0, SColor fcolor = SColor(255,255,255,255));

    //get irrlicht components
    ISceneManager *getSceneManager() { return m_SMgr;}
    IrrlichtDevice *getDevice() { return m_Device;}
    IVideoDriver *getDriver() { return m_Driver;}
    IGUIEnvironment *getGuiEnv() { return m_GUIEnv;}
    IMetaTriangleSelector *getMetaTriangleSelector() { return m_MetaTriangleSelector;}
    ICameraSceneNode *getCamera() { return m_Camera;}

    //console
    void sendToConsole(std::string nstring);

    friend MyEventReceiver;
    friend Console;
};
#endif // CLASS_GAME
