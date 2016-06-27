#include "game.hpp"
#include "tools.hpp"

#include <sstream>

Game *Game::mInstance = NULL;

Game::Game()
{
    //init pointers
    m_Device = NULL;
    m_Camera = NULL;
    m_MetaTriangleSelector = NULL;
    m_Mouse = NULL;
    m_Receiver = NULL;
    m_Player = NULL;
    m_InputContext = IMODE_PLAY;
    m_PreviousInputContext = IMODE_PLAY;

    m_CurrentLevel = 0;

    m_DoShutdown = false; //shutdown flag to let threads know they need to die

    //debug parameters
    dbg_noclip = false;
    dbg_nolighting = false;
    dbg_showboundingbox = false;
    dbg_showmainui = true;
    dbg_dodrawpal = false;

    //debug build options
#ifdef DEBUG
    dbg_noclip = true;
    dbg_nolighting = true;
#endif
}

Game::~Game()
{
    //destroy rendering device
    m_Device->drop();
}

/////////////////////////////////////////////////////////////////////
//  INITIALIZATION
int Game::start()
{
    int errorcode = 0;

    std::cout << "Game started.\n";

    std::cout << "Initializing console...";
    m_Console = Console::getInstance();
    std::cout << "done.\n";

    //init irrlicht
    std::cout << "Initialzing irrlicht...";
        errorcode = initIrrlicht();
        if(errorcode) {std::cout << "Error initializing irrlicht!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "done.\n";

    std::cout << "Loading fonts...\n";
        errorcode = loadFont("UWDATA\\font5x6p.sys", &m_FontNormal);
        if(errorcode) {std::cout << "Error loading normal font 5x6p!  ERROR CODE " << errorcode << "\n"; return -1;}
        std::cout << std::endl;

    std::cout << "Initializing camera...";
    loadScreen("Initializing camera...");
        errorcode = initCamera();
        if(errorcode) {std::cout << "Error initializing camera!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "done.\n";
        std::cout << std::endl;

    //load UW data
    std::cout << "Loading strings...";
    loadScreen("Loading strings...");
        errorcode = loadStrings(&m_StringBlocks);
        if(errorcode < 0) {std::cout << "Error loading string data!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "done.\n";
        std::cout << "Loaded " << errorcode << " strings.\n";
        //print test string, should = "Hey, its all the game strings"
        std::cout << m_StringBlocks[0].strings[0] << std::endl;

    std::cout << "Loading palette data...\n";
    loadScreen("Loading palettes...");
        errorcode = loadPalette(&m_Palettes);
        if(errorcode) { std::cout << "Error loading palette!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "....." << m_Palettes.size() << " palettes loaded.\n";
        errorcode = loadAuxPalette(&m_AuxPalettes);
        if(errorcode) { std::cout << "Error loading aux palette!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "....." << m_AuxPalettes.size() << " aux palettes loaded.\n";

    std::cout << "Loading textures...\n";
    loadScreen("Loading textures...");
        errorcode = loadTexture("UWDATA\\w64.tr", &m_Wall64TXT);
        if(errorcode) { std::cout << "Error loading textures!  ERROR CODE " << errorcode << "\n"; return -1;}
            else std::cout << "....." << m_Wall64TXT.size() << " wall64 textures loaded.\n";
        errorcode = loadTexture("UWDATA\\f32.tr", &m_Floor32TXT);
        if(errorcode) { std::cout << "Error loading textures!  ERROR CODE " << errorcode << "\n"; return -1;}
            else std::cout << "....." << m_Floor32TXT.size() << " floor32 textures loaded.\n";
        std::cout << std::endl;

    std::cout << "Loading graphics...\n";
    loadScreen("Loading graphics...");
    //note : this needs to be fixed, throws bad alloc, need to investigate parsing for graphics load
        errorcode = loadGraphic("UWDATA\\charhead.gr", &m_CharHeadTXT);
        if(errorcode) {std::cout << "Error loading charhead graphic!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "....." << m_CharHeadTXT.size() << " character portrait graphics loaded.\n";
        errorcode = loadGraphic("UWDATA\\cursors.gr", &m_CursorsTXT);
        if(errorcode) {std::cout << "Error loading cursors graphic!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "....." << m_CursorsTXT.size() << " cursor graphics loaded.\n";
        errorcode = loadGraphic("UWDATA\\objects.gr", &m_ObjectsTXT);
        if(errorcode) {std::cout << "Error loading object graphics!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "....." << m_ObjectsTXT.size() << " object graphics loaded.\n";
        errorcode = loadGraphic("UWDATA\\question.gr", &m_QuestionTXT);
        if(errorcode) {std::cout << "Error loading question mark graphic!  ERROR CODE " << errorcode << "\n"; return -1;}
        errorcode = loadGraphic("UWDATA\\inv.gr", &m_InventoryTXT);
        if(errorcode) {std::cout << "Error loading inventory graphics!  ERROR CODE " << errorcode << "\n"; return -1;}
        errorcode = loadGraphic("UWDATA\\scrledge.gr", &m_ScrollEdgeTXT);
        if(errorcode) {std::cout << "Error loading scroll graphics!  ERROR CODE " << errorcode << "\n"; return -1;}
        std::cout << std::endl;

    std::cout << "Loading bitmaps...";
    loadScreen("Loading bitmaps...");
        errorcode = loadBitmap("UWDATA\\pres1.byt", &m_BitmapsTXT, 5);
        if(errorcode) {std::cout << "Error loading bitmap!  ERROR CODE " << errorcode << "\n"; return -1;}
        errorcode = loadBitmap("UWDATA\\pres2.byt", &m_BitmapsTXT, 5);
        if(errorcode) {std::cout << "Error loading bitmap!  ERROR CODE " << errorcode << "\n"; return -1;}
        errorcode = loadBitmap("UWDATA\\main.byt", &m_BitmapsTXT, 0);
        if(errorcode) {std::cout << "Error loading bitmap!  ERROR CODE " << errorcode << "\n"; return -1;}
        errorcode = loadBitmap("UWDATA\\opscr.byt", &m_BitmapsTXT, 2);
        if(errorcode) {std::cout << "Error loading bitmap!  ERROR CODE " << errorcode << "\n"; return -1;}
        std::cout << "done.\n";
        std::cout << std::endl;




    std::cout << "Initializing objects...";
    loadScreen("Initializing objects...");
        errorcode = initObjects();
        if(errorcode < 0) {std::cout << "Error initializing objects!  ERROR CODE " << errorcode << "\n"; return-1;}
        else std::cout << errorcode << " objects initialized.\n";


    std::cout << "Initializing mouse...";
    loadScreen("Initializing mousea...");
        errorcode = initMouse();
        if(errorcode) {std::cout << "Error initializing mouse!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "done.\n";

    std::cout << "Initializing main UI...";
    loadScreen("Initializing UI...");
        errorcode = initMainUI();
        if(errorcode) {std::cout << "Error initializing main UI!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "done.\n";

    std::cout << "Loading level data...";
    loadScreen("Loading level data...");
        errorcode = loadLevel(&mLevels);
        if(errorcode) {std::cout << "Error loading level data!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << mLevels.size() << " levels loaded.\n";

    //mLevels[0].printDebug();

    std::cout << "Initializing player...";
    loadScreen("Initializing player...");
        errorcode = initPlayer();
        if(errorcode) {std::cout << "Error initializing player!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "done.\n";

    if(!DEBUG_NO_START)
    {
    //generate level geometry
    std::cout << "Generating level geometry...\n";
    loadScreen("Generating level geometry...");
        errorcode = mLevels[0].buildLevelGeometry();
        if(!errorcode) { std::cout << "Error generating level geometry!!  ERROR CODE " << errorcode << "\n"; return -1;}
        std::cout << mLevels[m_CurrentLevel].getMeshes().size() << " meshes generated for level " << m_CurrentLevel << std::endl;
        std::cout << std::endl;

    //create threads
    loadScreen("Launching threads...");
    MouseUpdateThread *mouseupdatethread = new MouseUpdateThread(m_Mouse, &m_DoShutdown);
    m_Threads.push_back(mouseupdatethread);

    //start threads
    for(int i = 0; i < int(m_Threads.size()); i++) m_Threads[i]->StartInternalThread();

    //start main loop
    std::cout << "Starting main loop...\n";
    loadScreen("Starting...");
    mainLoop();

    m_DoShutdown = true;
    }

    /*
    for(int i = 0; i < int(m_Threads.size()); i++)
    {
        m_Threads[i]->WaitForInternalThreadToExit();
    }
    */

    return 0;
}

void Game::loadScreen(std::string loadmessage)
{
    //clear scene
    m_Driver->beginScene(true, true, SColor(255,0,0,0));

    drawFontString(&m_FontNormal, loadmessage, vector2d<s32>(0,0), SColor(255,255,255,255));

    //done and display
    m_Driver->endScene();

}

int Game::initIrrlicht()
{
    //already initialized!!
    if(m_Device != NULL) return -1; // error, already initialized

    //init receiver
    m_Receiver = new MyEventReceiver(this);

    //init device
    m_Device = createDevice( video::EDT_OPENGL, dimension2d<u32>(SCREEN_WIDTH, SCREEN_HEIGHT), 16, FULLSCREEN, false, false, m_Receiver);
    if(!m_Device) return -2; // error device unable to be created successfully
    m_Device->setWindowCaption(L"UWproj");

    //get video
    m_Driver = m_Device->getVideoDriver();
    m_Driver->setTextureCreationFlag(irr::video::ETCF_ALWAYS_32_BIT,false);
    m_Driver->setTextureCreationFlag(irr::video::ETCF_ALLOW_NON_POWER_2, true);
    m_Driver->setTextureCreationFlag(irr::video::ETCF_ALWAYS_16_BIT,true);
    m_Driver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS,false);

    //get scene manager
    m_SMgr = m_Device->getSceneManager();
    //get gui environment
    m_GUIEnv = m_Device->getGUIEnvironment();
    //get collision manager
    m_ColMgr = m_SMgr->getSceneCollisionManager();

    //draw screen
    m_Driver->endScene();

    //create meta triangle selector
    m_MetaTriangleSelector = m_SMgr->createMetaTriangleSelector();
    m_TriangleSelector = NULL;

    //init frame delta time
    frameDeltaTime = 1.f;

    //some 2d rendering config
    //m_Driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
    //m_Driver->getMaterial2D().AntiAliasing=video::EAAM_FULL_BASIC;

    return 0;
}

int Game::initCamera()
{
    //already initialized!!
    if(m_Camera != NULL) return -1; // error already initialized!

    m_CameraPos = vector3df(245.5,15,127.5);
    m_CameraRot = vector3df(0,0,0);

    //add camera to scene
    m_Camera = m_SMgr->addCameraSceneNode(0, m_CameraPos);
    m_Camera->setID(ID_IsNotPickable);

    //create an empty scene node in front of the camera as "target"
    m_CameraTarget = m_SMgr->addEmptySceneNode();
    m_CameraTarget->setID(ID_IsNotPickable);

    //capture default FOV
    m_CameraDefaultFOV = m_Camera->getFOV();



    //camera light
    m_LightRadius = 5*8; // 5 tiles * tile units
    m_CameraLight = m_SMgr->addLightSceneNode(0, vector3df(0,0,0), SColorf(1.0f, 1.0f, 1.0f, 1.f), 4.0f);
    m_CameraLight->setID(ID_IsNotPickable);
    m_CameraLight->setLightType(video::ELT_POINT);
    //m_CameraLight->getLightData().Attenuation = vector3df(0, 1.f/(m_LightRadius/40), 0);
    m_CameraLight->getLightData().Attenuation = vector3df(0, 0.5, 0);
    m_CameraLight->enableCastShadow(false);
    //m_CameraLight->setRadius(m_LightRadius/40);
    m_LightData = m_CameraLight->getLightData();


    //additional camera settings
    m_Camera->setUpVector(vector3df(0,1,0));
    m_Camera->setFarValue(m_LightRadius/1.6);
    m_Camera->setNearValue(0.1f);
    //m_Camera->setAutomaticCulling(EAC_OFF);
    //m_CameraTarget->setPosition( vector3df(0,0,1));
    //m_Camera->addChild(m_CameraTarget);
    //m_CameraTarget->addChild(m_Camera);



    //level camera (avoid camera rotation when pointing at target)


    //unbind camera to target
    //m_Camera->bindTargetAndRotation(false);

    //updateCamera();

    //m_Camera = m_SMgr->addCameraSceneNodeFPS();
    //for collision testing
    //addCameraSceneNodeFPS (ISceneNode *parent=0, f32 rotateSpeed=100.0f, f32 moveSpeed=0.5f, s32 id=-1, SKeyMap *keyMapArray=0, s32 keyMapSize=0, bool noVerticalMovement=false, f32 jumpSpeed=0.f, bool invertMouse=false, bool makeActive=true)=0
    //m_Camera = m_SMgr->addCameraSceneNodeFPS(0, 100.0f, 0.5f, ID_IsNotPickable, 0, 0, false, 3.f);
    //m_Camera->setPosition( vector3df(0,4,0));
    //m_Camera->setPosition( m_CameraPos);
    //m_Camera->setRotation( m_CameraRot );


    //core::list<ISceneNodeAnimator*>::ConstIterator anim = m_Camera->getAnimators().begin();
    //ISceneNodeAnimatorCameraFPS *animfps = (ISceneNodeAnimatorCameraFPS*)(*anim);
    //animfps->setMoveSpeed(0.05);








    return 0;
}

int Game::initPlayer()
{
    if(m_Player != NULL) delete m_Player;

    m_Player = new Player;

    return 0;
}

/////////////////////////////////////////////////////////////
//  MAIN LOOP
void Game::mainLoop()
{
    bool drawaxis = true;



    //light1->getLightData().DiffuseColor = SColor(100,100,100,100);
    //light1->getLightData().SpecularColor = SColor(0,0,0,0);
    //light1->enableCastShadow(false);
    //m_SMgr->setAmbientLight( SColor(100,100,100,100));



/*
    //test
    int txtindex = 0;
    SMesh *squaremesh = getSquareMesh(UNIT_SCALE*8, UNIT_SCALE*8);
    IMeshSceneNode *mysquare = m_SMgr->addMeshSceneNode(squaremesh);
        mysquare->setPosition(vector3df(0,UNIT_SCALE*8 , 0));
        mysquare->setRotation(vector3df(180, 270, 0));
        mysquare->setMaterialTexture(0, m_Wall64TXT[txtindex]);
        mysquare->setMaterialFlag(video::EMF_BACK_FACE_CULLING, true);
        mysquare->setMaterialFlag(video::EMF_LIGHTING, false);
        //mycube->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
        //mycube->setMaterialFlag(video::EMF_BLEND_OPERATION, true);
        //mycube->setMaterialType(video::EMT_PARALLAX_MAP_SOLID);
        mysquare->updateAbsolutePosition();
    squaremesh->drop();
*/

    //test 2
    /*
    SMesh *mycubemesh = getCubeMesh(5);
    IMeshSceneNode *mycube = m_SMgr->addMeshSceneNode(mycubemesh);
    mycube->setPosition( vector3df(15,5,0));
    mycube->setScale(vector3df(2,2,2));
    mycube->setMaterialTexture(0, m_Wall64TXT[0]);
    mycube->setMaterialFlag(video::EMF_LIGHTING,false);
    mycubemesh->drop();
    */

    /*
    //create mesh
    SMesh *mycubemesh = getCubeMesh(128);
    //create scene node octree type
    IMeshSceneNode *mycube = m_SMgr->addOctreeSceneNode(mycubemesh, 0, IDFlag_IsPickable);
    mycube->setPosition( vector3df(-64,-128*2,64));
    mycube->setMaterialTexture(0, m_Wall64TXT[0]);
    mycube->setMaterialFlag(video::EMF_LIGHTING,false);
    mycubemesh->drop();

    //use selector to add to cube mesh
    scene::ITriangleSelector *selector = m_SMgr->createOctreeTriangleSelector(mycube->getMesh(), mycube, 128);
    mycube->setTriangleSelector(selector);

    const aabbox3d<f32>& mybbox = mycube->getBoundingBox();
    vector3df mybboxradius = mybbox.MaxEdge - mybbox.getCenter();

    //create collision response between selector and camera
    ISceneNodeAnimator* anim = m_SMgr->createCollisionResponseAnimator(selector, m_Camera, mybboxradius,vector3df(0,-10,0));
    selector->drop(); // As soon as we're done with the selector, drop it.
    m_Camera->addAnimator(anim);
    anim->drop();  // And likewise, drop the animator when we're done referring to it.
    */

    //collision testing
    /*
    int cubesize = 4;
    m_Camera->setPosition( vector3df(2,20,2));
    m_Camera->setRotation( vector3df(0,0,0));
    for(int i = 0; i < 64; i++)
    {
        for(int n = 0; n < 64; n++)
        {
            SMesh *mycubemesh = getSquareMesh(0,0,0,0);
            //create scene node octree type
            IMeshSceneNode *mycube = m_SMgr->addOctreeSceneNode(mycubemesh);
            mycube->setPosition( vector3df(i*cubesize,0,n*cubesize));
            mycube->setMaterialTexture(0, m_Wall64TXT[0]);
            mycube->setMaterialFlag(video::EMF_LIGHTING,false);
            mycubemesh->drop();
            configMeshSceneNode(mycube);
            registerForCollision(mycube);
        }
    }
    */

    //init collision
    /*
    const aabbox3d<f32> mybbox(0,0,0,UNIT_SCALE, UNIT_SCALE, UNIT_SCALE);
    vector3df mybboxradius = mybbox.MaxEdge - mybbox.getCenter();
    //create collision response between meta selector and camera
    ISceneNodeAnimator* anim = m_SMgr->createCollisionResponseAnimator(m_MetaTriangleSelector, m_Camera, mybboxradius,vector3df(0,GRAVITY,0));
    m_Camera->addAnimator(anim);
    //animator no longer needed, drop it
    anim->drop();
    */


    //texture testing
    /*
    std::vector<ITexture*> *testtextures = &m_ScrollEdgeTXT;
    int testtexturesindex = 0;
    m_Mouse->setTexture( (*testtextures)[testtexturesindex]);
    */



    //billboard test
    /*
    IBillboardSceneNode *mybillboard = m_SMgr->addBillboardSceneNode();
    mybillboard->setPosition( vector3df(244.48,15,133.649));
    mybillboard->setMaterialTexture(0, m_ObjectsTXT[0]);
    mybillboard->setSize( dimension2d<f32>(2,2));
    configBillboardSceneNode(mybillboard);
    //mybillboard->setMaterialFlag(EMF_COLOR_MASK, true);
    //mybillboard->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
    */


    //dimension2du d = core::dimension2du(320, 200);
    //m_Driver->OnResize(d);
    // now irrlicht internally ajusted device's resolution, but active camera will not update its aspect ratio automatically (which is OK, because you may no need it), so if you want the camera to be OK after this resize, you need to write also something like:
    //m_SMgr->getActiveCamera()->setAspectRatio((float)d.Width/d.Height);

    int lastFPS = -1;

    // In order to do framerate independent movement, we have to know
    // how long it was since the last frame
    u32 then = m_Device->getTimer()->getTime();

    //main loop
    while(m_Device->run())
    {
        //update mouse position
        //m_Mouse->updatePosition();

        //std::cout << "Mouse:" << m_MousePos.X << "," << m_MousePos.Y << std::endl;

        // Work out a frame delta time.
        const u32 now = m_Device->getTimer()->getTime();
        frameDeltaTime = (f32)(now - then) / 1000.f; // Time in seconds
        then = now;


        //processEvents();
        handleInputs();

        //update camera / collision
        updateCamera();


        //clear scene
        m_Driver->beginScene(true, true, SColor(255,0,0,0));
        //set 3d view position and size
        if(dbg_showmainui) m_Driver->setViewPort(rect<s32>(SCREEN_WORLD_POS_X, SCREEN_WORLD_POS_Y, SCREEN_WORLD_POS_X + SCREEN_WORLD_WIDTH, SCREEN_WORLD_POS_Y + SCREEN_WORLD_HEIGHT));

        /*
        //current floor plane
        plane3df myplane(vector3df(0,0,-25), vector3df(0,0,1));
        //line from camera to mouse cursor
        line3df myline = m_IMgr->getRayFromScreenCoordinates(m_MousePos);
        vector3df myint;
        //get intersection of camera_mouse_line to the floor plane
        myplane.getIntersectionWithLimitedLine(myline.end, myline.start, myint);
        */

        //test draw bounding box for grid 0 0
        //mymap[0][0]->setDebugDataVisible(irr::scene::EDS_BBOX);


        //make grid transparent if mouse is touching it
        /*
        for(int i = 0; i < int(mymap.size()); i++)
        {
            for(int n = 0; n < int(mymap[i].size()); n++)
            {
                if(mymap[i][n] == NULL) continue;

                if( mymap[i][n]->getTransformedBoundingBox().isPointInside(myint) )
                {
                    mymap[i][n]->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);

                    if(mouseLeftClicked)
                    {
                        mymap[i][n]->remove();
                        mymap[i][n] = NULL;
                    }

                }
                else mymap[i][n]->setMaterialType(video::EMT_SOLID);
            }
        }
        */


        //draw scene
        m_SMgr->drawAll();



        //draw axis
        if(drawaxis)
        {
            SMaterial mymat;
            mymat.setFlag(video::EMF_LIGHTING, false);
            m_Driver->setMaterial(mymat);
            m_Driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
            m_Driver->draw3DLine(vector3df(0,0,0), vector3df(0,0,100), SColor(255,0,0,255)); // z-axis blue
            m_Driver->draw3DLine(vector3df(0,0,0), vector3df(100,0,0), SColor(255,255,0,0)); // x-axis red
            m_Driver->draw3DLine(vector3df(0,0,0), vector3df(0,100,0), SColor(255,000,255,0)); // y-axis green

            //draw 3d line from camera to mouse click
            m_Driver->draw3DLine(m_Mouse->m_CameraMouseRay.start, m_Mouse->m_CameraMouseRay.end, SColor(255,255,0,0));
        }


        //draw gui
        if(dbg_showmainui)
        {
            m_Driver->setViewPort(rect<s32>(0,0,SCREEN_WIDTH, SCREEN_HEIGHT));

            //m_GUIEnv->drawAll();

            drawMainUI();

            if(dbg_dodrawpal) dbg_drawpal(&m_Palettes[0]);
        }




        /*
        m_Driver->setMaterial(SMaterial());
        m_Driver->setTransform(video::ETS_WORLD, IdentityMatrix);
        m_Driver->draw3DLine( vector3df(0,0,0), vector3df(100,0,0), SColor(0,255,0,0));
        */

        //draw mouse
        m_Mouse->draw();

        //done and display
        m_Driver->endScene();

        int fps = m_Driver->getFPS();

        if (lastFPS != fps)
        {
            core::stringw tmp(L"UWproj [");
            tmp += m_Driver->getName();
            tmp += L"] fps: ";
            tmp += fps;

            m_Device->setWindowCaption(tmp.c_str());
            lastFPS = fps;
        }

    }

    return;
}

void Game::handleInputs()
{
    if(m_InputContext == IMODE_PLAY)
    {
        //check if keys are held for movement
        if(m_Receiver->isKeyPressed(KEY_KEY_A))
        {
            m_CameraRot.Y -= frameDeltaTime * ROTATION_SPEED;
        }
        else if(m_Receiver->isKeyPressed(KEY_KEY_D))
        {
            m_CameraRot.Y += frameDeltaTime * ROTATION_SPEED;
        }

        if(m_Receiver->isKeyPressed(KEY_KEY_W))
        {
            m_CameraVel.Z = frameDeltaTime * MOVE_SPEED;
        }
        else if(m_Receiver->isKeyPressed(KEY_KEY_S) )
        {
            m_CameraVel.Z = -frameDeltaTime * MOVE_SPEED;
        }
        else m_CameraVel.Z = 0;

        if(m_Receiver->isKeyPressed(KEY_KEY_E))
        {
            m_CameraVel.Y = frameDeltaTime * MOVE_SPEED;
        }
        else if(m_Receiver->isKeyPressed(KEY_KEY_Q))
        {
            m_CameraVel.Y = -frameDeltaTime * MOVE_SPEED;
        }
        else m_CameraVel.Y = 0;
    }

}

void Game::processEvent(const SEvent *event)
{

    //input mode is scroll entry mode
    if(m_InputContext == IMODE_SCROLL_ENTRY)
    {
        if(event->EventType == EET_KEY_INPUT_EVENT)
        {
            if(event->KeyInput.PressedDown)
            {
                //note, some keys, like the period, are not recognized properly
                //need to investigate
                std::cout << "debug key char=" << int(event->KeyInput.Char) << std::endl;
                m_Scroll->addInputCharacter( int(event->KeyInput.Char) );
                /*
                int keyval = event->KeyInput.Key;

                if(keyval >= 65 && keyval <= 90)
                {
                    if(event->KeyInput.Shift) m_Scroll->addInputCharacter(keyval);
                    else m_Scroll->addInputCharacter( keyval + 32);
                }
                else m_Scroll->addInputCharacter(keyval);
                */
            }
        }
    }//end scroll entry event
    //else input mode is play mode
    else
    {
        //key event
        if(event->EventType == EET_KEY_INPUT_EVENT)
        {
            //key pressed
            if(event->KeyInput.PressedDown)
            {

                if(event->KeyInput.Key == KEY_ESCAPE) m_Device->closeDevice();
                else if(event->KeyInput.Key == KEY_OEM_3)
                {
                    m_Console->m_String = std::string("");
                    m_Scroll->startInputMode(&m_Console->m_String);
                }
                else if(event->KeyInput.Key == KEY_SPACE)
                {
                }
                else if(event->KeyInput.Key == KEY_KEY_W)
                {
                }
                else if(event->KeyInput.Key == KEY_KEY_A)
                {

                }
                else if(event->KeyInput.Key == KEY_KEY_D)
                {

                }
                else if(event->KeyInput.Key == KEY_KEY_E)
                {
                }
                else if(event->KeyInput.Key == KEY_KEY_Q)
                {
                }
                else if(event->KeyInput.Key == KEY_KEY_T)
                {
                }
                else if(event->KeyInput.Key == KEY_KEY_Z)
                {
                    /*
                    testtexturesindex--;
                    if(testtexturesindex < 0) testtexturesindex = int(testtextures->size()-1);
                    m_Mouse->setTexture( (*testtextures)[testtexturesindex]);
                    std::cout << "test texture index = " << testtexturesindex << std::endl;
                    */
                }
                else if(event->KeyInput.Key == KEY_KEY_X)
                {
                    /*
                    testtexturesindex++;
                    if(testtexturesindex >= int(testtextures->size()) ) testtexturesindex = 0;
                    m_Mouse->setTexture( (*testtextures)[testtexturesindex]);
                    std::cout << "test texture index = " << testtexturesindex << std::endl;
                    */
                }
                else if(m_Receiver->isKeyPressed(KEY_F1))
                {
                    m_CameraPos = m_Camera->getPosition();
                    Tile *ttile = mLevels[m_CurrentLevel].getTile(int(m_CameraPos.Z)/UNIT_SCALE, int(m_CameraPos.X)/UNIT_SCALE);
                    std::cout << "Camera Position : " << m_CameraPos.X << "," << m_CameraPos.Y << "," << m_CameraPos.Z << std::endl;
                    std::cout << "Camera ID = " << m_Camera->getID() << ", Camera Target ID = " << m_CameraTarget->getID() << std::endl;
                    if(ttile != NULL)
                    {
                        ttile->printDebug();
                    }
                    else std::cout << "Current tile = NULL!\n";
                }
                else if(m_Receiver->isKeyPressed(KEY_F2))
                {
                    dbg_noclip = !dbg_noclip;
                    std::cout << "debug no clip = " << dbg_noclip << std::endl;
                }
                else if(m_Receiver->isKeyPressed(KEY_F3))
                {
                    dbg_nolighting = !dbg_nolighting;
                    std::cout << "debug no lighting = " << dbg_nolighting << std::endl;
                    reconfigureAllLevelMeshes();
                    reconfigureAllLevelObjects();
                }
                else if(m_Receiver->isKeyPressed(KEY_F4))
                {
                    dbg_showboundingbox = !dbg_showboundingbox;
                    std::cout << "debug show bounding box = " << dbg_showboundingbox << std::endl;
                    reconfigureAllLevelMeshes();
                    reconfigureAllLevelObjects();
                }
                else if(m_Receiver->isKeyPressed(KEY_F5))
                {
                    m_CameraPos = m_Camera->getPosition();
                    std::cout << "CAMERA_POS:" << m_CameraPos.X << "," << m_CameraPos.Y << "," << m_CameraPos.Z << std::endl;
                }
                else if(m_Receiver->isKeyPressed(KEY_F6))
                {
                    dbg_showmainui = !dbg_showmainui;
                    std::cout << "show main ui = " << dbg_showmainui << std::endl;
                }
                else if(m_Receiver->isKeyPressed(KEY_F12))
                {
                    dbg_dodrawpal = !dbg_dodrawpal;
                    std::cout << "draw pal = " << dbg_dodrawpal << std::endl;
                }

            }
            //key released
            else
            {

            }


        }
        //mouse event
        if(event->EventType == EET_MOUSE_INPUT_EVENT)
        {
            //if mouse left button pressed
            if(event->MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
            {

                std::stringstream mouseclickss;
                mouseclickss << "mouse clicked @" << m_Mouse->getMousePositionX() << "," << m_Mouse->getMousePositionY();
                //print to scroll message window and console
                std::cout << mouseclickss.str() << std::endl;
                m_Scroll->addMessage(mouseclickss.str(), FONT_NORMAL, m_Palettes[0][SCROLL_FONT_PAL_GREEN]);

                //if mouse was clicked within world view
                //if main ui is displayed
                if(dbg_showmainui)
                {
                    if(m_Mouse->getMousePositionX() >= SCREEN_WORLD_POS_X && m_Mouse->getMousePositionX() <= (SCREEN_WORLD_POS_X + SCREEN_WORLD_WIDTH) &&
                       m_Mouse->getMousePositionY() >= SCREEN_WORLD_POS_Y && m_Mouse->getMousePositionY() <= (SCREEN_WORLD_POS_Y + SCREEN_WORLD_HEIGHT))
                    {
                        vector2di mousePosConverted = *m_Mouse->getMousePosition();
                        mousePosConverted.X = float(m_Mouse->getMousePositionX()-SCREEN_WORLD_POS_X) / (float(SCREEN_WORLD_WIDTH) / float(SCREEN_WIDTH));
                        mousePosConverted.Y = float(m_Mouse->getMousePositionY()-SCREEN_WORLD_POS_Y) / (float(SCREEN_WORLD_HEIGHT) / float(SCREEN_HEIGHT));

                        std::cout << "screen width = " << SCREEN_WIDTH << std::endl;
                        std::cout << "screen height = " << SCREEN_HEIGHT << std::endl;
                        std::cout << "world pos x = " << SCREEN_WORLD_POS_X << std::endl;
                        std::cout << "world pos y = " << SCREEN_WORLD_POS_Y << std::endl;
                        std::cout << "world width = " << SCREEN_WORLD_WIDTH << std::endl;
                        std::cout << "world height = " << SCREEN_WORLD_HEIGHT << std::endl;
                        std::cout << "mouse clicked converted @" << mousePosConverted.X << "," << mousePosConverted.Y << std::endl;

                        m_Mouse->m_CameraMouseRay = m_ColMgr->getRayFromScreenCoordinates(mousePosConverted, m_Camera);
                    }
                }
                //no main ui, world view is full screen
                else
                {

                    std::cout << "screen width = " << SCREEN_WIDTH << std::endl;
                    std::cout << "screen height = " << SCREEN_HEIGHT << std::endl;
                    std::cout << "world pos x = " << SCREEN_WORLD_POS_X << std::endl;
                    std::cout << "world pos y = " << SCREEN_WORLD_POS_Y << std::endl;
                    std::cout << "world width = " << SCREEN_WORLD_WIDTH << std::endl;
                    std::cout << "world height = " << SCREEN_WORLD_HEIGHT << std::endl;

                    m_Mouse->m_CameraMouseRay = m_ColMgr->getRayFromScreenCoordinates(*m_Mouse->getMousePosition(), m_Camera);

                }

                /*
                //check ray collision
                vector3df intersection;
                triangle3df triangle;
                ISceneNode *selectedSceneNode = m_ColMgr->getSceneNodeAndCollisionPointFromRay(m_CameraMouseRay, intersection, triangle, 0);
                if(selectedSceneNode != NULL)
                {
                    std::cout << "mouse ray hit something\n";
                    selectedSceneNode->setVisible(false);
                }
                else std::cout << "mouse ray did not hit anything\n";
                */

                //first try to get an object
                ISceneNode *selectedSceneNode = m_ColMgr->getSceneNodeFromRayBB(m_Mouse->m_CameraMouseRay, ID_IsObject);
                if(selectedSceneNode != NULL)
                {
                    std::cout << "OBJ HIT!\n";
                }
                else
                {
                    selectedSceneNode = m_ColMgr->getSceneNodeFromRayBB(m_Mouse->m_CameraMouseRay, ID_IsMap);
                    if(selectedSceneNode != NULL)
                    {
                        std::cout << "MAP HIT!\n";
                    }
                }
                if(selectedSceneNode != NULL) std::cout << "Name of hit node = " << selectedSceneNode->getName() << std::endl;

            }
            //else right mouse button pressed
            else if(event->MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
            {

            }
            //else mouse wheel moved
            else if(event->MouseInput.Event == EMIE_MOUSE_WHEEL)
            {
                //mouse wheel up
                if(event->MouseInput.Wheel > 0)
                {
                    m_Camera->setFOV( m_Camera->getFOV()*0.9);
                }
                //mouse wheel down
                else if(event->MouseInput.Wheel < 0)
                {
                    m_Camera->setFOV( m_Camera->getFOV()*1.1);
                }
            }
        }
    }//end play mode events
}

bool Game::setInputContext(int ncontext)
{
    if(ncontext < 0 || ncontext >= IMODE_TOTAL) return false;

    //if no change in context
    if(m_InputContext == ncontext) return false;

    //set previous to current
    m_PreviousInputContext = m_InputContext;
    m_InputContext = ncontext;

    std::cout << "Changing input context to " << m_InputContext << std::endl;

    return true;
}

int Game::drawMainUI()
{
    const static rect<s32> screen_rect( position2d<s32>(0,0), dimension2d<u32>(SCREEN_WIDTH, SCREEN_HEIGHT));
    const static rect<s32> scroll_edge_rect( position2d<s32>(0,0), dimension2d<u32>(m_ScrollEdgeTXT[0]->getSize()) );
    const static SColor m_ScrollFillColor = m_Palettes[0][SCROLL_PAL_INDEX];

    //draw main ui graphic
    m_Driver->draw2DImage( m_BitmapsTXT[2], position2d<s32>(0,0), screen_rect, NULL, SColor(255,255,255,255), true);

    //draw scroll components (edges and main scroll window)
    int scrolledgestate = m_Scroll->getScrollEdgeState();
    m_Driver->draw2DRectangle(m_ScrollFillColor, m_Scroll->getScrollRect());
    m_Driver->draw2DImage( m_ScrollEdgeTXT[scrolledgestate], position2d<s32>(SCROLL_EDGE_LEFT_X, SCROLL_EDGE_Y), scroll_edge_rect, NULL, SColor(255,255,255,255), true);
    m_Driver->draw2DImage( m_ScrollEdgeTXT[scrolledgestate+5], position2d<s32>(SCROLL_EDGE_RIGHT_X, SCROLL_EDGE_Y), scroll_edge_rect, NULL, SColor(255,255,255,255), true);

    //draw scroll messages
    m_Scroll->draw();

}

void Game::updateCamera()
{
    //adjust camera position with velocity vector
    //m_CameraPos += m_CameraVel;

    //apply gravity
    //m_CameraVel.Y -= GRAVITY_ACCEL * frameDeltaTime;
    //if(m_CameraVel.Y > -TERMINAL_GRAVITY) m_CameraVel.Y = TERMINAL_GRAVITY * frameDeltaTime;

    //capture current camera positions
    vector3df current_campos = m_CameraPos;
    vector3df current_camtgt = m_CameraTarget->getPosition();

    //create matrix
    matrix4 m;
    //rotate matrix by camera rotation
    m.setRotationDegrees(m_CameraRot);
    //create target position (camera target relative to camera - see camera init)
    vector3df ctarget = vector3df(0,0,1);
    vector3df cvelmod = m_CameraVel;
    //vector3df cpos = m_CameraVel;
    //transform camera target (relative to camera node) to take rotation into account
    m.transformVect(ctarget);
    m.transformVect(cvelmod);
    //m_CameraTarget->setPosition(m_CameraPos +m_CameraVel);
    m_CameraPos += cvelmod;
    //node->updateAbsolutePosition();

/*
    //m_CameraPos += m_CameraVel;
    m_Camera->setPosition(m_CameraPos);
    m_CameraTarget->setPosition(m_CameraPos + ctarget);
    //m_CameraTarget->setPosition( m_CameraTarget->getAbsolutePosition() + m_CameraVel);
    //m_CameraTarget->updateAbsolutePosition();
    //m_Camera->setRotation(m_CameraRot);
    m_Camera->updateAbsolutePosition();
    m_CameraTarget->updateAbsolutePosition();
    m_Camera->setTarget(m_CameraTarget->getPosition());
*/

    //if collision process properly
    if(processCollision(&m_CameraPos, &cvelmod))
    {
        //m_CameraPos.Y = ttile->getHeight()+3;

        m_Camera->setPosition(m_CameraPos);
        m_CameraTarget->setPosition(m_CameraPos + ctarget);
        m_Camera->updateAbsolutePosition();
        m_CameraTarget->updateAbsolutePosition();
        m_Camera->setTarget(m_CameraTarget->getPosition());
    }
    //else if it didn't (tile null or solid?), revert
    else
    {
        m_CameraPos = current_campos;
        m_Camera->setPosition(m_CameraPos);
        m_CameraTarget->setPosition(current_camtgt);
    }

    //update camera light
    m_CameraLight->setPosition(m_CameraPos);


    //m_Camera->setTarget(m_CameraTarget);
    /*
    std::cout << "camera pos:" << m_Camera->getPosition().X << "," << m_Camera->getPosition().Y << "," << m_Camera->getPosition().Z << std::endl;
    std::cout << "camera target pos:" << m_CameraTarget->getPosition().X << "," << m_CameraTarget->getPosition().Y << "," << m_CameraTarget->getPosition().Z << std::endl;
    std::cout << "camera rot:" << m_CameraRot.Y << std::endl;
    */

}

bool Game::processCollision(vector3df *pos, vector3df *vel)
{
    if(dbg_noclip) return true;
    else if(pos == NULL || vel == NULL) return false;

    //if no velocity, don't bother updating anything
    //if( *vel == vector3df(0,0,0)) return true;

    //calculate coordinates by tile
    vector2di tpos(pos->Z/UNIT_SCALE, pos->X/UNIT_SCALE);

    //calculate coordinates within tile
    vector2df tsubpos( ((pos->Z/UNIT_SCALE)-tpos.X)*TILE_UNIT , ((pos->X/UNIT_SCALE)-tpos.Y)*TILE_UNIT );

    //get tile coordinates are in
    Tile *ttile = mLevels[m_CurrentLevel].getTile(tpos.X, tpos.Y);

    //if tile is solid, return true
    if(ttile->getType() == TILETYPE_SOLID)
    {
        //std::cout << "current tile is solid, returning false!\n";
        return false;
    }
    //else std::cout << "vel:" << vel->X << "," << vel->Y << "," << vel->Z << std::endl;

    //get all adjacent tiles
    std::vector<Tile*> adjtiles = mLevels[m_CurrentLevel].getAdjacentTilesAt(tpos.X, tpos.Y);

    vector2df diagvel(0,0);

    //check north/south collision
    //if moving northward
    if(vel->X < 0)
    {
        //if current tile a diagonal open to
        if(ttile->getType() == TILETYPE_D_SE)
        {
            if( tsubpos.Y <= TILE_UNIT - tsubpos.X + TILE_UNIT*0.125)
            {
                diagvel = projectVectorAontoB(vector2df(vel->Z, vel->X), vector2df(1,-1));
            }
        }
        else if(ttile->getType() == TILETYPE_D_SW)
        {
            if(tsubpos.Y - TILE_UNIT*0.125 <= tsubpos.X)
            {
                diagvel = projectVectorAontoB(vector2df(vel->Z, vel->X), vector2df(1,1));
            }
        }
        //is position close enough to wall?
        else if(tsubpos.Y <= 1)
        {

            //is there a tile in direction?
            if(adjtiles[NORTH])
            {
                //is that tile solid?
                if(adjtiles[NORTH]->getType() == TILETYPE_SOLID)
                {
                    //kill vel
                    pos->X += -vel->X;
                    vel->X = 0;
                    std::cout << "north wall collision!\n";
                }
            }
        }
    }
    //moving southward?
    else if(vel->X > 0)
    {
        //if current tile a diagonal open to
        if(ttile->getType() == TILETYPE_D_NE)
        {
            if( tsubpos.Y >= tsubpos.X - TILE_UNIT*0.125)
            {
                diagvel = projectVectorAontoB(vector2df(vel->Z, vel->X), vector2df(1,1));
            }
        }
        else if(ttile->getType() == TILETYPE_D_NW)
        {
            std::cout << "JOHN TEST\n";
            if(tsubpos.Y >= TILE_UNIT - tsubpos.X + TILE_UNIT*0.125)
            {
                diagvel = projectVectorAontoB(vector2df(vel->Z, vel->X), vector2df(1,-1));
            }

        }
        //is position close enough to wall?
        if(tsubpos.Y >= TILE_UNIT-1)
        {
            //is there a tile in direction?
            if(adjtiles[SOUTH])
            {
                //is that tile solid?
                if(adjtiles[SOUTH]->getType() == TILETYPE_SOLID)
                {
                    //kill vel
                    pos->X -= vel->X;
                    vel->X = 0;
                    std::cout << "south wall collision!\n";
                }
            }
        }
    }

    //check east/west collision
    //if moving westward
    if(vel->Z < 0)
    {
        if(ttile->getType() == TILETYPE_D_NE)
        {
            if(tsubpos.X <= tsubpos.Y + TILE_UNIT*0.125)
            {
                diagvel = projectVectorAontoB(vector2df(vel->Z, vel->X), vector2df(1,1));
            }
        }
        else if(ttile->getType() == TILETYPE_D_SE)
        {
            if(tsubpos.X <=  TILE_UNIT - tsubpos.Y + TILE_UNIT*0.125)
            {
                diagvel = projectVectorAontoB(vector2df(vel->Z, vel->X), vector2df(1,-1));
            }
        }
        //is position close enough to wall?
        else if(tsubpos.X <= 1)
        {
            //is there a tile in direction?
            if(adjtiles[WEST])
            {
                //is that tile solid?
                if(adjtiles[WEST]->getType() == TILETYPE_SOLID)
                {
                    //kill vel
                    pos->Z += -vel->Z;
                    vel->Z = 0;
                    std::cout << "north wall collision!\n";
                }
            }
        }
    }
    //moving eastward
    else if(vel->Z > 0)
    {
        if(ttile->getType() == TILETYPE_D_NW)
        {
            if(tsubpos.X >= TILE_UNIT - tsubpos.Y - TILE_UNIT*0.125)
            {
                diagvel = projectVectorAontoB(vector2df(vel->Z, vel->X), vector2df(1,-1));
            }
        }
        else if(ttile->getType() == TILETYPE_D_SW)
        {
            if(tsubpos.X >=  tsubpos.Y - TILE_UNIT*0.125)
            {
                diagvel = projectVectorAontoB(vector2df(vel->Z, vel->X), vector2df(1,1));
            }
        }
        //is position close enough to wall?
        else if(tsubpos.X >= TILE_UNIT-1)
        {
            //is there a tile in direction?
            if(adjtiles[EAST])
            {
                //is that tile solid?
                if(adjtiles[EAST]->getType() == TILETYPE_SOLID)
                {
                    //kill vel
                    pos->Z += -vel->Z;
                    vel->Z = 0;
                    std::cout << "east wall collision!\n";
                }
            }
        }
    }

    if(diagvel != vector2df(0,0))
    {
        pos->Z -= vel->Z;
        pos->X -= vel->X;

        vel->Z = diagvel.X;
        vel->X = diagvel.Y;

        pos->Z += vel->Z;
        pos->X += vel->X;
    }

    //calculate floor height for slope
    //note tile's height + STANDING_HEIGHT is where player should be when standing
    //get current floor height value
    float cheight = ttile->getHeight();

    //if on a ramp, calculate height
    if(ttile->getType() >= 6 && ttile->getType() <= 9)
    {
        float m = 0.125;
        switch(ttile->getType())
        {
        //ramp sloping up to the south...
        case TILETYPE_SL_S:
            pos->Y = m*(tsubpos.Y) + cheight + STANDING_HEIGHT;
            break;
        case TILETYPE_SL_N:
            pos->Y = -m*(tsubpos.Y) + 1 + cheight + STANDING_HEIGHT;
            break;
        case TILETYPE_SL_W:
            pos->Y = -m*(tsubpos.X) + 1 + cheight + STANDING_HEIGHT;
            break;
        case TILETYPE_SL_E:
            pos->Y = m*(tsubpos.X) + cheight + STANDING_HEIGHT;
            break;
        default:
            std::cout << "Error calculating slope height, undefined slope!\n";
            pos->Y = ttile->getHeight()+STANDING_HEIGHT;
            break;

        }
    }
    else pos->Y = ttile->getHeight()+STANDING_HEIGHT;


    return true;
}


Object *Game::getObject(int id)
{
    if(id < 0 || id > int(m_Objects.size()) ) return NULL;

    return m_Objects[id];
}



int Game::initObjects()
{
    //std::cout << "object graphics count : " << m_ObjectsTXT.size() << std::endl;
    //std::cout << "object strings        : " << m_StringBlocks[3].strings.size() << std::endl;

    //supports up to 512 objects
    /*
   0000-001f  Weapons and missiles
   0020-003f  Armour and clothing
   0040-007f  Monsters
   0080-008f  Containers
   0090-0097  Light sources
   0098-009f  Wands
   00a0-00af  Treasure
   00b0-00bf  Comestibles
   00c0-00df  Scenery and junk
   00e0-00ff  Runes and bits of the Key of Infinity
   0100-010f  Keys, lockpick, lock
   0110-011f  Quest items
   0120-012f  Inventory items, misc stuff
   0130-013f  Books and scrolls
   0140-014f  Doors
   0150-015f  Furniture
   0160-016f  Pillar, some decals, force field, special tmap obj
   0170-017f  Switches
   0180-019f  Traps
   01a0-01bf  Triggers
   01c0-01cf  Explosions/splats, fountain, silver tree, moving things
   */

   for(int i = 0; i < 512; i++)
   {
       Object *newobject = new Object;
       if(i < int(m_ObjectsTXT.size()) ) newobject->setTexture( m_ObjectsTXT[i]);

       if(i < int(m_StringBlocks[3].strings.size())) newobject->setDescription( m_StringBlocks[3].strings[i]);

       m_Objects.push_back(newobject);
   }


    return int(m_Objects.size());
}

int Game::initMouse()
{
    //create mouse and link to game
    m_Mouse = new Mouse(this);

    //set mouse texture to cursor
    m_Mouse->setTexture(m_CursorsTXT[0]);

    return 0;
}

int Game::initMainUI()
{
    //check scroll palette
    if(m_Palettes.empty()) return -1;
    if( SCROLL_PAL_INDEX >= m_Palettes[0].size() || SCROLL_PAL_INDEX < 0) return -2;

    //check scroll textures
    if(m_ScrollEdgeTXT.empty()) return -3;
    if( int(m_ScrollEdgeTXT.size()) < 10) return -4;

    //create scroll object
    m_Scroll = new Scroll(this);

    return 0;
}

bool Game::configMeshSceneNode(IMeshSceneNode *tnode)
{
    if(tnode == NULL) return false;

    tnode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, true);
    if(dbg_nolighting) tnode->setMaterialFlag(video::EMF_LIGHTING, false);
    else tnode->setMaterialFlag(video::EMF_LIGHTING, true);
    //tnode->setMaterialFlag(video::EMF_TEXTURE_WRAP, true);
    //tnode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
    //tnode->setMaterialFlag(video::EMF_BLEND_OPERATION, true);
    //tnode->setMaterialType(video::EMT_PARALLAX_MAP_SOLID);
    tnode->setMaterialFlag(video::EMF_BILINEAR_FILTER, false );
    tnode->setMaterialFlag(video::EMF_TRILINEAR_FILTER, false );
    tnode->setMaterialFlag(video::EMF_ANISOTROPIC_FILTER, false );

    //automatic culling
    tnode->setAutomaticCulling(EAC_FRUSTUM_SPHERE ) ;

    //show bounding box debug data
    if(dbg_showboundingbox) tnode->setDebugDataVisible(EDS_BBOX);
    else tnode->setDebugDataVisible(EDS_OFF);

    //texture repeating
    tnode->getMaterial(0).getTextureMatrix(0).setScale(1);
    tnode->getMaterial(0).TextureLayer->TextureWrapU = video::ETC_REPEAT;
    tnode->getMaterial(0).TextureLayer->TextureWrapV = video::ETC_REPEAT;

    //update
    tnode->updateAbsolutePosition();
    tnode->setID(ID_IsMap);

    //register collision stuff
    if(CONFIG_FOR_COLLISION)
    {
        if(tnode->getTriangleSelector() != NULL)
        {
            std::cout << "node already has triangle selector.\n";
        }

        if(USE_OCTREE)
        {
            m_TriangleSelector = m_SMgr->createOctreeTriangleSelector(tnode->getMesh(), tnode,128);
            tnode->setTriangleSelector(m_TriangleSelector);
            m_TriangleSelector->drop();
        }
        else
        {
            m_TriangleSelector = m_SMgr->createTriangleSelector(tnode->getMesh(), tnode);
            tnode->setTriangleSelector(m_TriangleSelector);
            m_TriangleSelector->drop();
        }
    }

    return true;
}

bool Game::configBillboardSceneNode(IBillboardSceneNode *tnode)
{
    if(tnode == NULL) return false;

     //tnode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, true);
    if(dbg_nolighting) tnode->setMaterialFlag(video::EMF_LIGHTING, false);
    else tnode->setMaterialFlag(video::EMF_LIGHTING, true);
    tnode->setMaterialFlag(video::EMF_BILINEAR_FILTER, false );
    tnode->setMaterialFlag(video::EMF_TRILINEAR_FILTER, false );
    tnode->setMaterialFlag(video::EMF_ANISOTROPIC_FILTER, false );

    //automatic culling
    tnode->setAutomaticCulling(EAC_FRUSTUM_SPHERE ) ;

    //alpha
    tnode->setMaterialFlag(EMF_COLOR_MASK, true);
    tnode->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);

    //show bounding box debug data
    if(dbg_showboundingbox) tnode->setDebugDataVisible(EDS_BBOX);
    else tnode->setDebugDataVisible(EDS_OFF);

    //texture repeating
    tnode->getMaterial(0).getTextureMatrix(0).setScale(1);
    tnode->getMaterial(0).TextureLayer->TextureWrapU = video::ETC_REPEAT;
    tnode->getMaterial(0).TextureLayer->TextureWrapV = video::ETC_REPEAT;

    //update
    tnode->updateAbsolutePosition();

    //register collision stuff
    //if(CONFIG_FOR_COLLISION) configureForCollision(tnode);

    return true;
}

bool Game::updateObject(ObjectInstance *tobj, Tile *ttile)
{
    if(tobj == NULL) return false;

    //get billboard object
    IBillboardSceneNode *tbb = tobj->getBillboard();

    //if tile is null, object is not on a tile, so if billboard is not null, drop it
    if(ttile == NULL && tbb != NULL)
    {
        tbb->drop();
        tbb = NULL;
    }

    //if billboard has not been created, but needs to be
    if(tbb == NULL && ttile != NULL)
    {
        //create billboard node
        //std::cout << "Creating billboard scene node...\n";
        tbb = m_SMgr->addBillboardSceneNode();

        //set id to pickable id
        tbb->setID(ID_IsObject);
        std::stringstream objnamess;
        objnamess << "OBJ_" << tobj->getInstanceID();
        tbb->setName(objnamess.str().c_str());

        //link billboard node to object
        //std::cout << "Setting billboard scene node to object...\n";
        tobj->setBillboard(tbb);

        //set billboard texture to object id
        //std::cout << "Setting billboards texture from object id...\n";
        tbb->setMaterialTexture(0, tobj->getTexture());
    }

    //if billboard is not null
    if(tbb != NULL)
    {
        vector2di tilepos = ttile->getPosition();
        vector3di objpos = tobj->getPosition();

        tbb->setSize(dimension2d<f32>(OBJECT_SCALE,OBJECT_SCALE) );

        //configure billboard common settings
        //std::cout << "Configuring billboard node...\n";
        configBillboardSceneNode(tbb);

        const float conversion = float(UNIT_SCALE)/float(TILE_UNIT);

        vector3df debugpos( (tilepos.Y*UNIT_SCALE) + conversion*float(TILE_UNIT-objpos.Y),
                            ( float(objpos.Z) / TILE_UNIT)+(float(OBJECT_SCALE)/2),
                            (tilepos.X*UNIT_SCALE) + conversion*float(objpos.X) );
        tbb->setPosition( debugpos);
    }

    return true;
}


SMesh *Game::getCubeMesh(f32 cubesize)
{

        SMesh* Mesh = new SMesh();

        int vcount = 36;

        SMeshBuffer *buf = new SMeshBuffer();
        Mesh->addMeshBuffer(buf);
        buf->drop();

        buf->Vertices.reallocate(vcount);
        buf->Vertices.set_used(vcount);

        //top
        buf->Vertices[0] = S3DVertex(0,0,0, 0,0,1,    video::SColor(255,255,255,255), 0, 0);
        buf->Vertices[1] = S3DVertex(1*cubesize,0,0, 0,0,1,  video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[2] = S3DVertex(0,1*cubesize,0, 0,0,1,    video::SColor(255,255,255,255), 0, 1);
        buf->Vertices[3] = S3DVertex(1*cubesize,0,0, 0,0,1,    video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[4] = S3DVertex(1*cubesize,1*cubesize,0, 0,0,1,    video::SColor(255,255,255,255), 1, 1);
        buf->Vertices[5] = S3DVertex(0,1*cubesize,0, 0,0,1,    video::SColor(255,255,255,255), 0, 1);

        //front
        buf->Vertices[6] = S3DVertex(0,1*cubesize,0, 0,1,0,    video::SColor(255,255,255,255), 0, 0);
        buf->Vertices[7] = S3DVertex(1*cubesize,1*cubesize,0, 0,1,0,    video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[8] = S3DVertex(0,1*cubesize,-1*cubesize, 0,1,0,    video::SColor(255,255,255,255), 0, 1);
        buf->Vertices[9] = S3DVertex(1*cubesize,1*cubesize,0, 0,1,0,    video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[10] = S3DVertex(1*cubesize,1*cubesize,-1*cubesize, 0,1,0,    video::SColor(255,255,255,255), 1, 1);
        buf->Vertices[11] = S3DVertex(0,1*cubesize,-1*cubesize, 0,1,0,    video::SColor(255,255,255,255), 0, 1);

        //right
        buf->Vertices[12] = S3DVertex(1*cubesize,1*cubesize,0, 1,0,0,    video::SColor(255,255,255,255), 0, 0);
        buf->Vertices[13] = S3DVertex(1*cubesize,0,0, 1,0,0,    video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[14] = S3DVertex(1*cubesize,1*cubesize,-1*cubesize, 1,0,0,    video::SColor(255,255,255,255), 0, 1);
        buf->Vertices[15] = S3DVertex(1*cubesize,0,0, 1,0,0,    video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[16] = S3DVertex(1*cubesize,0,-1*cubesize, 1,0,0,    video::SColor(255,255,255,255), 1, 1);
        buf->Vertices[17] = S3DVertex(1*cubesize,1*cubesize,-1*cubesize, 1,0,0,    video::SColor(255,255,255,255), 0, 1);

        //left
        buf->Vertices[18] = S3DVertex(0,0,0, -1,0,0,    video::SColor(255,255,255,255), 0, 0);
        buf->Vertices[19] = S3DVertex(0,1*cubesize,0, -1,0,0,    video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[20] = S3DVertex(0,0,-1*cubesize, -1,0,0,    video::SColor(255,255,255,255), 0, 1);
        buf->Vertices[21] = S3DVertex(0,1*cubesize,0, -1,0,0,    video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[22] = S3DVertex(0,1*cubesize,-1*cubesize, -1,0,0,    video::SColor(255,255,255,255), 1, 1);
        buf->Vertices[23] = S3DVertex(0,0,-1*cubesize, -1,0,0,    video::SColor(255,255,255,255), 0, 1);

        //back
        buf->Vertices[24] = S3DVertex(1*cubesize,0,0, 0,-1,0,    video::SColor(255,255,255,255), 0, 0);
        buf->Vertices[25] = S3DVertex(0,0,0, 0,-1,0,    video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[26] = S3DVertex(1*cubesize,0,-1*cubesize, 0,-1,0,    video::SColor(255,255,255,255), 0, 1);
        buf->Vertices[27] = S3DVertex(0,0,0, 0,-1,0,    video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[28] = S3DVertex(0,0,-1*cubesize, 0,-1,0,    video::SColor(255,255,255,255), 1, 1);
        buf->Vertices[29] = S3DVertex(1*cubesize,0,-1*cubesize, 0,-1,0,    video::SColor(255,255,255,255), 0, 1);

        //bottom
        buf->Vertices[30] = S3DVertex(0,1*cubesize,-1*cubesize, 0,0,-1,    video::SColor(255,255,255,255), 0, 0);
        buf->Vertices[31] = S3DVertex(1*cubesize,1*cubesize,-1*cubesize, 0,0,-1,  video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[32] = S3DVertex(0,0,-1*cubesize, 0,0,-1,    video::SColor(255,255,255,255), 0, 1);
        buf->Vertices[33] = S3DVertex(1*cubesize,1*cubesize,-1*cubesize, 0,0,-1,    video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[34] = S3DVertex(1*cubesize,0,-1*cubesize, 0,0,-1,    video::SColor(255,255,255,255), 1, 1);
        buf->Vertices[35] = S3DVertex(0,0,-1*cubesize, 0,0,-1,    video::SColor(255,255,255,255), 0, 1);

        buf->Indices.reallocate(vcount);
        buf->Indices.set_used(vcount);

        for(int i = 0; i < vcount; i++) buf->Indices[i] = i;
        /*
        buf->Indices[0]=0;
        buf->Indices[1]=1;
        buf->Indices[2]=2;
        buf->Indices[3]=3;
        buf->Indices[4]=4;
        buf->Indices[5]=5;
        */

        Mesh->setBoundingBox( aabbox3df(0,0,-cubesize,cubesize,cubesize,0));
        buf->recalculateBoundingBox();


        return Mesh;

}

SMesh *Game::getSquareMesh(int ul, int ur, int br, int bl)
{
    SMesh *mesh = NULL;
    SMeshBuffer *buf = NULL;

    int vcount = 6;
    int scale = UNIT_SCALE/4;

    //FLOOR MESH
    mesh = new SMesh();
    buf = new SMeshBuffer();

    mesh->addMeshBuffer(buf);
    buf->drop();

    buf->Vertices.reallocate(vcount);
    buf->Vertices.set_used(vcount);

    //triangle 1
    buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,ul*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 0); //TL
    buf->Vertices[1] = S3DVertex(0*UNIT_SCALE,ur*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
    buf->Vertices[2] = S3DVertex(1*UNIT_SCALE,bl*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1);// BL
    //triangle 2
    buf->Vertices[3] = S3DVertex(0*UNIT_SCALE,ur*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
    buf->Vertices[4] = S3DVertex(1*UNIT_SCALE,br*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 1); //BR
    buf->Vertices[5] = S3DVertex(1*UNIT_SCALE,bl*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1); // BL


    //finalize vertices
    buf->Indices.reallocate(vcount);
    buf->Indices.set_used(vcount);
    for(int i = 0; i < vcount; i++) buf->Indices[i] = i;
    //mesh->setBoundingBox( aabbox3df(0,ul*scale-CEIL_HEIGHT,0,1*UNIT_SCALE,br*scale,1*UNIT_SCALE));
    mesh->setBoundingBox( aabbox3df(0,-1*UNIT_SCALE,0,1*UNIT_SCALE,0,1*UNIT_SCALE));
    buf->recalculateBoundingBox();

    return mesh;

}

void Game::addMessage(std::string msgstring, int fonttype, SColor fcolor)
{
    m_Scroll->addMessage(msgstring, fonttype, fcolor);
}

void Game::sendToConsole(std::string nstring)
{
    m_Console->parse(nstring);
}

///////////////////////////////////////////////////////////////////////////////////
//  DEBUG
void Game::reconfigureAllLevelMeshes()
{
    std::cout << "Reconfiguring all level meshes...";
    std::vector<IMeshSceneNode*> lmeshes = mLevels[m_CurrentLevel].getMeshes();

    for(int i = 0; i < int(lmeshes.size()); i++)
    {
        if(!configMeshSceneNode(lmeshes[i]))
        {
            std::cout << "Error configuring mesh #" << i << std::endl;
        }
    }

    std::cout << "done.\n";
}

void Game::reconfigureAllLevelObjects()
{
    std::cout << "Reconfiguring all level objects...\n";
    for(int i = 0; i < TILE_ROWS; i++)
    {
        for(int n = 0; n < TILE_COLS; n++)
        {
            Tile *ttile = mLevels[m_CurrentLevel].getTile(n,i);

            std::vector<ObjectInstance*> objs = ttile->getObjects();

            for(int k = 0; k < int(objs.size()); k++)
            {
                updateObject(objs[k], ttile);
            }
        }
    }
}

void Game::dbg_drawpal(std::vector<SColor> *tpal)
{
    const int palsize = 4;

    rect<s32> palwin( position2d<s32>(0,0), dimension2d<u32>(palsize*16*SCREEN_SCALE, palsize*16*SCREEN_SCALE));

    for(int i = 0; i < 16; i++)
    {
        for(int n = 0; n < 16; n++)
        {
            int palindex = i*16 + n;
            core::rect<s32> prect( position2d<s32>(n*SCREEN_SCALE*palsize, i*SCREEN_SCALE*palsize), dimension2d<u32>(palsize*SCREEN_SCALE, palsize*SCREEN_SCALE));
            m_Driver->draw2DRectangle( (*tpal)[palindex], prect);
        }
    }

    //draw normal font too
    m_Driver->draw2DImage(m_FontNormal.m_Texture, position2d<s32>(palsize*16*SCREEN_SCALE, 0));

    //if mouse is inside pal window rect
    position2d<s32> mpos( int(m_Mouse->getMousePositionX()), int(m_Mouse->getMousePositionY()) );
    if(palwin.isPointInside(mpos))
    {
        int mx = mpos.X / (16);
        int my = mpos.Y / (16);
        int mindex = my*16 + mx;

        std::stringstream mss;
        mss << "#" << mindex;

        drawFontString(&m_FontNormal, mss.str(), *m_Mouse->getMousePosition() + vector2di(16,16));
    }
}

void Game::dbg_stringdump()
{
    std::ofstream ofile;

    ofile.open("stringdump.txt");

    for(int i = 0; i < int(m_StringBlocks.size()); i++)
    {
        ofile << "index        #" << i << std::endl;
        ofile << "id           #" << m_StringBlocks[i].id << std::endl;
        ofile << "string count =" << m_StringBlocks[i].strings.size() << std::endl;
        for(int n = 0; n < int(m_StringBlocks[i].strings.size()); n++)
        {
            ofile << "string " << n << std::endl;
            ofile << m_StringBlocks[i].strings[n] << std::endl;
        }
        ofile << std::endl;
    }

    ofile.close();
}
