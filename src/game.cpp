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

    m_CurrentLevel = 0;

    dbg_noclip = false;
    dbg_nolighting = false;
    dbg_showboundingbox = false;

    //debug
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
//
int Game::start()
{
    int errorcode = 0;

    std::cout << "Game started.\n";

    //init irrlicht
    std::cout << "Initialzing irrlicht...";
        errorcode = initIrrlicht();
        if(errorcode) {std::cout << "Error initializing irrlicht!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "done.\n";

    std::cout << "Initializing camera...";
        errorcode = initCamera();
        if(errorcode) {std::cout << "Error initializing camera!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "done.\n";
        std::cout << std::endl;

    //load UW data
    std::cout << "Loading strings...";
        errorcode = loadStrings();
        if(errorcode < 0) {std::cout << "Error loading string data!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "done.\n";
        std::cout << "Loaded " << errorcode << " strings.\n";
        //print test string, should = "Hey, its all the game strings"
        std::cout << m_StringBlocks[0].strings[0] << std::endl;

    std::cout << "Loading palette data...\n";
        errorcode = loadPalette();
        if(errorcode) { std::cout << "Error loading palette!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "....." << m_Palettes.size() << " palettes loaded.\n";
        errorcode = loadAuxPalette();
        if(errorcode) { std::cout << "Error loading aux palette!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << "....." << m_AuxPalettes.size() << " aux palettes loaded.\n";

    std::cout << "Loading textures...\n";
        errorcode = loadTexture("UWDATA\\w64.tr", &m_Wall64TXT);
        if(errorcode) { std::cout << "Error loading textures!  ERROR CODE " << errorcode << "\n"; return -1;}
            else std::cout << "....." << m_Wall64TXT.size() << " wall64 textures loaded.\n";
        errorcode = loadTexture("UWDATA\\f32.tr", &m_Floor32TXT);
        if(errorcode) { std::cout << "Error loading textures!  ERROR CODE " << errorcode << "\n"; return -1;}
            else std::cout << "....." << m_Floor32TXT.size() << " floor32 textures loaded.\n";
        std::cout << std::endl;

    std::cout << "Loading graphics...\n";
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
        std::cout << std::endl;

    std::cout << "Loading bitmaps...";
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
        errorcode = initObjects();
        if(errorcode < 0) {std::cout << "Error initializing objects!  ERROR CODE " << errorcode << "\n"; return-1;}
        else std::cout << errorcode << " objects initialized.\n";

    std::cout << "Loading level data...";
        errorcode = loadLevel();
        if(errorcode) {std::cout << "Error loading level data!  ERROR CODE " << errorcode << "\n"; return -1;}
        else std::cout << mLevels.size() << " levels loaded.\n";

    //mLevels[0].printDebug();

    if(true)
    {
    //generate level geometry
    std::cout << "Generating level geometry...\n";
        errorcode = mLevels[0].buildLevelGeometry();
        if(!errorcode) { std::cout << "Error generating level geometry!!  ERROR CODE " << errorcode << "\n"; return -1;}
        std::cout << mLevels[m_CurrentLevel].getMeshes().size() << " meshes generated for level " << m_CurrentLevel << std::endl;
        std::cout << std::endl;

    //start main loop
    std::cout << "Starting main loop...\n";
    mainLoop();
    }





    return 0;
}

int Game::initIrrlicht()
{
    //already initialized!!
    if(m_Device != NULL) return -1; // error, already initialized

    //init device
    m_Device = createDevice( video::EDT_OPENGL, dimension2d<u32>(SCREEN_WIDTH, SCREEN_HEIGHT), 16, FULLSCREEN, false, false, &m_Receiver);
    if(!m_Device) return -2; // error device unable to be created successfully
    m_Device->setWindowCaption(L"UWproj");

    //get video
    m_Driver = m_Device->getVideoDriver();
    m_Driver->setTextureCreationFlag(irr::video::ETCF_ALWAYS_32_BIT,true);
    m_Driver->setTextureCreationFlag(irr::video::ETCF_ALWAYS_16_BIT,false);
    m_Driver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS,false);

    //get scene manager
    m_SMgr = m_Device->getSceneManager();
    //get gui environment
    m_GUIEnv = m_Device->getGUIEnvironment();
    //get collision manager
    m_IMgr = m_SMgr->getSceneCollisionManager();

    //draw screen
    m_Driver->endScene();

    //create meta triangle selector
    m_MetaTriangleSelector = m_SMgr->createMetaTriangleSelector();

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

    //create an empty scene node in front of the camera as "target"
    m_CameraTarget = m_SMgr->addEmptySceneNode();

    //capture default FOV
    m_CameraDefaultFOV = m_Camera->getFOV();

    //additional camera settings
    m_Camera->setUpVector(vector3df(0,1,0));
    m_Camera->setFarValue(UNIT_SCALE*20);
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

/////////////////////////////////////////////////////////////
//  MAIN LOOP
void Game::mainLoop()
{
    bool drawaxis = true;

    //add light test
    scene::ILightSceneNode* light1 = m_SMgr->addLightSceneNode(0, vector3df(0,0,0), SColorf(1.0f, 1.0f, 1.0f, 1.f), 4.0f);
    light1->setLightType(video::ELT_POINT);
    //light1->setRotation(vector3df(0,180,0));
    //light1->getLightData().Falloff = 5;
    light1->enableCastShadow(false);
    light1->setRadius(2);

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


    IGUIImage *myguiimage = m_GUIEnv->addImage(m_BitmapsTXT[2], position2d<s32>(0,0), true);

    std::vector<ITexture*> *texturestotest = &m_CursorsTXT;
    int texturestotestsize = int(texturestotest->size());
    int texturestotestindex = 0;
    IGUIImage *mymousecursor = m_GUIEnv->addImage( (*texturestotest)[texturestotestindex], position2d<s32>(0,0), true);


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

    //flag to store if mouse button was clicked on this frame
    bool mouseLeftClicked = false;

    //main loop
    while(m_Device->run())
    {

        const SEvent *event = NULL;
        mouseLeftClicked = false;

        m_MousePos = m_Device->getCursorControl()->getPosition();
        //std::cout << "Mouse:" << m_MousePos.X << "," << m_MousePos.Y << std::endl;

        // Work out a frame delta time.
        const u32 now = m_Device->getTimer()->getTime();
        frameDeltaTime = (f32)(now - then) / 1000.f; // Time in seconds
        then = now;


        //check if keys are held for movement
        if(m_Receiver.isKeyPressed(KEY_KEY_A))
        {
            m_CameraRot.Y -= frameDeltaTime * ROTATION_SPEED;
        }
        else if(m_Receiver.isKeyPressed(KEY_KEY_D))
        {
            m_CameraRot.Y += frameDeltaTime * ROTATION_SPEED;
        }

        if(m_Receiver.isKeyPressed(KEY_KEY_W))
        {
            m_CameraVel.Z = frameDeltaTime * MOVE_SPEED;
        }
        else if(m_Receiver.isKeyPressed(KEY_KEY_S) )
        {
            m_CameraVel.Z = -frameDeltaTime * MOVE_SPEED;
        }
        else m_CameraVel.Z = 0;

        if(m_Receiver.isKeyPressed(KEY_KEY_E))
        {
            m_CameraVel.Y = frameDeltaTime * MOVE_SPEED;
        }
        else if(m_Receiver.isKeyPressed(KEY_KEY_Q))
        {
            m_CameraVel.Y = -frameDeltaTime * MOVE_SPEED;
        }
        else m_CameraVel.Y = 0;

        //process events in que
        while(m_Receiver.processEvents(event))
        {
            //key event
            if(event->EventType == EET_KEY_INPUT_EVENT)
            {
                //key pressed
                if(event->KeyInput.PressedDown)
                {
                    if(event->KeyInput.Key == KEY_ESCAPE) m_Device->closeDevice();
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
                        texturestotestindex--;
                        if(texturestotestindex < 0) texturestotestindex = texturestotestsize-1;
                        mymousecursor->setVisible(false);
                        mymousecursor = m_GUIEnv->addImage( (*texturestotest)[texturestotestindex], position2d<s32>(0,0), true);
                        mymousecursor->setImage( (*texturestotest)[texturestotestindex]);
                        std::cout << "test texture index = " << texturestotestindex << std::endl;
                    }
                    else if(event->KeyInput.Key == KEY_KEY_X)
                    {
                        texturestotestindex++;
                        if(texturestotestindex >= texturestotestsize) texturestotestindex = 0;
                        mymousecursor->setVisible(false);
                        mymousecursor = m_GUIEnv->addImage( (*texturestotest)[texturestotestindex], position2d<s32>(0,0), true);
                        mymousecursor->setImage( (*texturestotest)[texturestotestindex]);
                        std::cout << "test texture index = " << texturestotestindex << std::endl;
                    }
                    else if(m_Receiver.isKeyPressed(KEY_F1))
                    {
                        m_CameraPos = m_Camera->getPosition();
                        Tile *ttile = mLevels[m_CurrentLevel].getTile(int(m_CameraPos.Z)/UNIT_SCALE, int(m_CameraPos.X)/UNIT_SCALE);
                        std::cout << "Camera Position : " << m_CameraPos.X << "," << m_CameraPos.Y << "," << m_CameraPos.Z << std::endl;
                        if(ttile != NULL)
                        {
                            ttile->printDebug();
                        }
                        else std::cout << "Current tile = NULL!\n";
                    }
                    else if(m_Receiver.isKeyPressed(KEY_F2))
                    {
                        dbg_noclip = !dbg_noclip;
                        std::cout << "debug no clip = " << dbg_noclip << std::endl;
                    }
                    else if(m_Receiver.isKeyPressed(KEY_F3))
                    {
                        dbg_nolighting = !dbg_nolighting;
                        std::cout << "debug no lighting = " << dbg_nolighting << std::endl;
                        reconfigureAllLevelMeshes();
                        reconfigureAllLevelObjects();
                    }
                    else if(m_Receiver.isKeyPressed(KEY_F4))
                    {
                        dbg_showboundingbox = !dbg_showboundingbox;
                        std::cout << "debug show bounding box = " << dbg_showboundingbox << std::endl;
                        reconfigureAllLevelMeshes();
                        reconfigureAllLevelObjects();
                    }
                    else if(m_Receiver.isKeyPressed(KEY_F5))
                    {
                        m_CameraPos = m_Camera->getPosition();
                        std::cout << "CAMERA_POS:" << m_CameraPos.X << "," << m_CameraPos.Y << "," << m_CameraPos.Z << std::endl;
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
                    mouseLeftClicked = true;

                    std::cout << "mouse clicked @" << m_MousePos.X << "," << m_MousePos.Y << std::endl;
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
        }

        //update actor and camera
        //apply gravity to camera

        updateCamera();
        light1->setPosition(m_CameraPos);
        //m_CameraPos = m_Camera->getPosition();
        //m_Camera->setPosition( vector3df(m_CameraPos.X, m_CameraPos.Y-0.1*frameDeltaTime, m_CameraPos.Z));

        //update mouse cursor graphic
        mymousecursor->setRelativePosition( position2d<s32>(m_MousePos.X, m_MousePos.Y));


        //clear scene
        m_Driver->beginScene(true, true, SColor(255,0,0,0));
        //set 3d view position and size
        if(SHOW_MAIN_UI) m_Driver->setViewPort(rect<s32>(SCREEN_WORLD_POS_X, SCREEN_WORLD_POS_Y, SCREEN_WORLD_WIDTH, SCREEN_WORLD_HEIGHT));

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
        }


        //draw gui
        if(SHOW_MAIN_UI)
        {
            m_Driver->setViewPort(rect<s32>(0,0,SCREEN_WIDTH, SCREEN_HEIGHT));

            m_GUIEnv->drawAll();
        }

        /*
        m_Driver->setMaterial(SMaterial());
        m_Driver->setTransform(video::ETS_WORLD, IdentityMatrix);
        m_Driver->draw3DLine( vector3df(0,0,0), vector3df(100,0,0), SColor(0,255,0,0));
        */

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

int Game::loadStrings()
{
    /* STRING BLOCKS
   block   description
   0001    general UI strings
   0002    character creation strings, mantras (?)
   0003    wall text/scroll/book/book title strings (*)
   0004    object descriptions (*)
   0005    object "look" descriptions, object quality states
   0006    spell names
   0007    conversation partner names, starting at string 17 for conv 1
   0008    text on walls, signs
   0009    text trap messages
   000a    wall/floor description text
   0018    debugging strings (not used ingame)
   0c00    intro cutscene text
   0c01    ending cutscene text
   0c02    tyball cutscene text
   0c03    arial cuscene text (?)
   0c18    dream cutscene 1 text "arrived"
   0c19    dream cutscene 2 text "talismans"
   ...17
   0c1a-0c21  garamon cutscene texts
   +7
   0e01-0f3a  conversation strings
   313
   */

    //struct used to store huffman tree string nodes
    struct hnode
    {
        //1 byte each
        int chardata;
        int parent;
        int left;
        int right;
    };

    struct block
    {
        //block header, total 6 bytes
        int blocknum; // 2 bytes
        std::streampos offset; // 4 bytes

        //string offset is relative position after end of block header
        int stringcount; // 2 bytes
        std::vector<std::streampos> stringoffsets; // 2 bytes
        std::vector<std::string> strings;
    };

    //  strings are stored in a huffman tree, using this struct to store branch and leaf ddata
    hnode *head = NULL;

    std::ifstream ifile;
    const std::string tfile("UWDATA\\strings.pak");

    //  load string file
    ifile.open(tfile.c_str(), std::ios::binary);

    //  was file able to be openend?
    if(!ifile.is_open()) return -1; // error opening file

    //get node count
    unsigned char nodecntbuf[2];
    int nodecount = 0;
    readBin(&ifile, nodecntbuf, 2);
    nodecount = lsbSum(nodecntbuf, 2);
    //init htree count
    hnode htree[nodecount];


    //  read in all nodes
    //  note : last node is head of tree
    for(int i = 0; i < nodecount; i++)
    {
        unsigned char nodedatabuf[4];
        if(!readBin(&ifile, nodedatabuf, 4)) std::cout << "     error at " << i << std::endl;;

        hnode newnode;
        newnode.chardata = int(nodedatabuf[0]);
        newnode.parent   = int(nodedatabuf[1]);
        newnode.left     = int(nodedatabuf[2]);
        newnode.right    = int(nodedatabuf[3]);

       htree[i] = newnode;

    }

    //set huffman tree head to last node in list
    head = &htree[nodecount-1];


    unsigned char blockcntbuf[2];
    int blockcnt = 0;
    readBin(&ifile, blockcntbuf, 2);
    blockcnt = lsbSum(blockcntbuf,2);
    std::streampos endoffile = 0;

    //read in block offsets
    block blocks[blockcnt];

    for(int i = 0; i < blockcnt; i++)
    {
        block newblock;

        unsigned char blocknumbuf[2];
        unsigned char offsetbuf[4];

        readBin(&ifile, blocknumbuf, 2);
        readBin(&ifile, offsetbuf, 4);

        newblock.blocknum = lsbSum(blocknumbuf, 2);
        newblock.offset = std::streampos(lsbSum(offsetbuf, 4));

        blocks[i] = newblock;

    }

    //get end of file position
    ifile.seekg(0, ifile.end);
    endoffile = ifile.tellg();

    //for debug purposes
    std::vector<unsigned char> teststring;

    //for each block, read in string count and string relative offsets
    //read in strings
    for(int i = 0; i < blockcnt; i++)
    {
        //jump to block offset
        ifile.seekg( blocks[i].offset);

        //get string count
        unsigned char stringcntbuf[2];
        readBin(&ifile, stringcntbuf, 2);
        blocks[i].stringcount = lsbSum(stringcntbuf, 2);
        //resize block string list
        blocks[i].strings.resize(blocks[i].stringcount);
        //resize string offsets container
        blocks[i].stringoffsets.resize(blocks[i].stringcount);

        //get string relative offsets (relative to end of block header to start of string)
        for(int n = 0; n < blocks[i].stringcount; n++)
        {
            unsigned char stringoffbuf[2];
            readBin(&ifile, stringoffbuf, 2);

            blocks[i].stringoffsets[n] = std::streampos( lsbSum(stringoffbuf, 2));

        }

        //read in strings
        for(int n = 0; n < blocks[i].stringcount; n++)
        {
            //jump to string offsets (block offset + 6 bytes + relative offset)
            ifile.seekg( blocks[i].offset + std::streampos(2) + blocks[i].stringcount*2+ blocks[i].stringoffsets[n]);

            //if(i == 0 && n == 0) std::cout << "TEST STRING OFFSET = " << std::hex << blocks[i].offset + std::streampos(6) + blocks[i].stringoffsets[n] << std::dec << std::endl;

            //read in byte one at a time, popping off bits big-endian to navigate huffman tree nodes
            //if reaching a '|' character (0x7c), end of string found.  All following bits of current
            //byte are unused.

            //init current node to head
            hnode *curnode = head;
            bool stringdone = false;

            //decoding loop
            while(!stringdone)
            {
                //read in a byte
                unsigned char bbuf[1];
                readBin(&ifile, bbuf, 1);

                int val = int(bbuf[0]);

                //check each bit, big endian
                for(int k = 7; k >= 0; k--)
                {
                    //pop bit off, determine direction
                    //if bin val == 1, take a right
                    if( (val >> k) & 0x01)
                    {
                        //if(i == testblocknum && n == teststringnum) std::cout << "1";
                        curnode = &htree[curnode->right];
                    }
                    else
                    {
                        //if(i == testblocknum && n == teststringnum) std::cout << "0";
                        curnode = &htree[curnode->left];
                    }

                    //leaf found when when left and right children are 0xff
                    if(curnode->left == 0xff && curnode->right == 0xff)
                    {
                        //as long as string terminator is not found, add char
                        if(curnode->chardata != 0x7c)
                            blocks[i].strings[n].push_back( char( curnode->chardata));
                        //else string is done
                        else stringdone = true;

                        //testing
                        //if(i == testblocknum && n == teststringnum) std::cout << "\n         = " << std::hex << curnode->chardata << std::dec << "(" << char(curnode->chardata) << ")\n";

                        //set current node back to the head
                        curnode = head;
                    }
                }
            }

        }

    }

    //copy string block info into class member
    //string counter for feedback info
    int stringcounter = 0;
    m_StringBlocks.resize(blockcnt);
    for(int i = 0; i < blockcnt; i++)
    {
        m_StringBlocks[i].id = blocks[i].blocknum;
        m_StringBlocks[i].strings = blocks[i].strings;
        stringcounter += int(m_StringBlocks[i].strings.size());
    }

    ifile.close();
    return stringcounter;
}

int Game::loadLevel()
{
    //read in level archive
    std::ifstream ifile;
    //const std::string tfile("UWDATA\\lev.ark");
    const std::string tfile("SAVE01\\lev.ark");

    //blocks in level archive (read from header)
    uint16_t blockcount = 0;

    //texture mapping storage
    //texture map blocks indexes that link tile data floor/wall values to actual w64 and f32 textures indices
    std::vector< std::vector<int> > texturemap; // 9 blocks of texture mappings
    texturemap.resize(9);

    //offset locations for each block
    std::vector<std::streampos> blockoffsets;

    //attempt to open level archive
    ifile.open(tfile.c_str(), std::ios_base::binary);
    if(!ifile.is_open()) return -1; // error unable to open file

    //read block count from header
    unsigned char blockcountbuf[2];
    if(!readBin(&ifile, blockcountbuf, 2)) return -2; // error unable to read block count
    blockcount = uint16_t(lsbSum(blockcountbuf, 2));
    //std::cout << std::dec << "Block count:" << blockcount << std::endl;

    //read block offsets
    for(int i = 0; i < int(blockcount); i++)
    {
        unsigned char bobuf[4];
        readBin(&ifile, bobuf, 4);

        blockoffsets.push_back( std::streampos(lsbSum(bobuf, 4)));
        //std::cout << std::dec << "block " << i << "[" << char((i/9)+97) << "]: offset = 0x" << std::hex << blockoffsets.back() << std::endl;
    }

    //load texture map blocks
    const int txtmapwalls = 48; // 2bytes
    const int txtmapfloors = 10; // 2bytes
    const int txtmapdoors = 6; // 1byte

    for(int i = 0; i < 9; i++)
    {
        //texture maps are 18 (9*3) blocks in the file
        //jump to texture map block
        ifile.seekg(blockoffsets[(9*2)+i]);

        //read in texture mapping for walls
        for(int n = 0; n < txtmapwalls; n++)
        {
            unsigned char wallmap[2];
            readBin(&ifile, wallmap, 2);

            texturemap[i].push_back( lsbSum(wallmap, 2));
        }

        //read in texture mapping for floors
        for(int n = 0; n < txtmapfloors; n++)
        {
            unsigned char floormap[2];
            readBin(&ifile, floormap, 2);

            texturemap[i].push_back( lsbSum(floormap, 2));
        }

        //read in texture mapping for doors
        for(int n = 0; n < txtmapdoors; n++)
        {
            unsigned char doormap[1];
            readBin(&ifile, doormap, 1);

            texturemap[i].push_back( lsbSum(doormap, 1));
        }

        /*
        if(i == 0)
        {
            std::cout << "starting at block:" << std::hex << blockoffsets[2*9+i] << std::endl;
            for(int n = 0; n < int(texturemap[0].size()); n++)
            {
                std::cout << std::dec << "texturemap index " << n << ":" << texturemap[0][n] << std::endl;
            }
        }
        */
    }

    //read in level data
    // note : UW1 only has 9 levels
    // note : UW1 maps are 64x64 tiles * 4 bytes
    for(int i = 0; i < 9; i++)
    {
        //create level
        mLevels.push_back(Level());

        //save texture map data for shits and giggles
        mLevels.back().mTextureMapping = texturemap[i];

        //set ceiling texture index from texture map (level uses one for whole map)
        mLevels.back().setCeilingTextureIndex( texturemap[i][txtmapwalls+txtmapfloors-1]);

        //jump file position pointer to offset to begin reading in level data
        ifile.seekg(blockoffsets[i]);

        //read in 64 x 64 map tiles
        //note: uw tiles are flipped on y axis
        for(int n = TILE_ROWS-1; n >=0; n--)
        {
            for(int p = 0; p < TILE_COLS; p++)
            {
                //get tile
                Tile *tile = mLevels.back().getTile(p, n);

                //first two bytes
                unsigned char ldatabuf1[2];
                readBin(&ifile, ldatabuf1, 2);
                int tiledata1 = lsbSum(ldatabuf1, 2);

                //set tile type
                tile->setType( getBitVal(tiledata1, 0, 4));

                //set tile height
                tile->setHeight( getBitVal(tiledata1, 4, 4));

                //set unknown bit 1
                if( getBitVal(tiledata1, 8, 1)) tile->setUnk1(true);
                else tile->setUnk1(false);

                //set unknown bit 2
                if( getBitVal(tiledata1, 9, 1)) tile->setUnk2(true);
                else tile->setUnk2(false);

                //set floor texture index
                //match up texture map block index to actual floor texture index
                tile->setFloorTXT( texturemap[i][txtmapwalls + getBitVal(tiledata1, 10, 4)] );

                //set magic illegal flag
                if( getBitVal(tiledata1, 14, 1)) tile->setMagicIllegal(true);
                else tile->setMagicIllegal(false);

                //set has door flag
                //match up texture map block index to actual door texture index
                if( texturemap[i][txtmapwalls + txtmapfloors + getBitVal(tiledata1, 15, 1)] ) tile->setHasDoor(true);
                else tile->setHasDoor(false);

                //last two bytes
                unsigned char ldatabuf2[2];
                readBin(&ifile, ldatabuf2, 2);
                int tiledata2 = lsbSum(ldatabuf2, 2);

                //set wall texture index
                //match up texture map block index to actual floor texture index
                tile->setWallTXT( texturemap[0][getBitVal(tiledata2, 0, 6)] );

                //set first object in tile
                tile->setFirstObjectIndex( getBitVal(tiledata2, 6, 10) );
            }
        }

        //read in level objects

        //jump to master list for mobile objects in block (offset 0x4000)
        //total of 1024 objects (256 mobile(npc), and 768 static objects)
        //build master list index starting with mobile objects
        ifile.seekg( blockoffsets[i] + std::streampos(0x4000));

        //master object list
        for(int n = 0; n < 1024; n++)
        {
            //both mobile and static objects have general object information header

            //each object has general object header, contains 8 bytes of information
            //object info
            unsigned char objinfobuf[2];
            int objinfo;
            readBin(&ifile, objinfobuf, 2);
            objinfo = lsbSum(objinfobuf, 2);
            int objid = getBitVal(objinfo, 0, 8);

            //create new object instance of object id
            ObjectInstance *newobj = new ObjectInstance(m_Objects[objid]);
            //add new object to master object list
            mLevels.back().addObject(newobj);

            //populate the rest of object flags/info
            newobj->setFlags( getBitVal(objinfo, 8, 4));
            newobj->setEnchanted( getBitVal(objinfo, 12, 1));
            newobj->setDoorDir( getBitVal(objinfo, 13, 1));
            newobj->setInvisible( getBitVal(objinfo, 14, 1));
            newobj->setIsQuantity( getBitVal(objinfo, 15, 1));

            //object position
            unsigned char objposbuf[2];
            readBin(&ifile, objposbuf, 2);
            int objpos = lsbSum(objposbuf, 2);
            newobj->setAngle( getBitVal(objpos, 7, 3));
            vector3di opos;
            opos.Z = getBitVal(objpos, 0, 7);
            opos.Y = getBitVal(objpos, 10, 3);
            opos.X = getBitVal(objpos, 13, 3);
            newobj->setPosition(opos);


            //object quality / chain
            unsigned char objqualbuf[2];
            readBin(&ifile, objqualbuf, 2);
            int objqualdat = lsbSum(objqualbuf, 2);
            newobj->setQuality( getBitVal(objqualdat, 0, 6) );
            newobj->setNext( getBitVal(objqualdat, 6, 10) );

            //object link / special
            unsigned char objlinkbuf[2];
            readBin(&ifile, objlinkbuf, 2);
            int objectlinkdat = lsbSum(objlinkbuf, 2);
            newobj->setOwner( getBitVal(objectlinkdat, 0, 6));
            newobj->setQuantity( getBitVal(objectlinkdat, 6, 10));

            //mobile (npc) objects have an additional 19 bytes of data
            //mobile objects are the first 256 object in master list
            if( n < 256)
            {
                //temp mob dat
                unsigned char mobdata[19];
                readBin(&ifile, mobdata, 19);
            }
        }

        //get master objects list
        std::vector<ObjectInstance*> *objlist = mLevels.back().getObjectsMaster();

        //add objects to tiles
        for(int n = 0; n < TILE_ROWS; n++)
        {
            for(int p = 0; p < TILE_COLS; p++)
            {

                //for debug purposes, only process one tile

                //get tile
                Tile *ttile = mLevels.back().getTile(p, n);

                //get first object index from tile
                int objindex = ttile->getFirstObjectIndex();

                //note : object 0 means empty, no objects on tile
                //add each linked object to tile object list
                while(objindex != 0 && i == m_CurrentLevel) // for now, only process current level
                {

                        //retrieve object from master list
                        ObjectInstance *tobj = (*objlist)[objindex];

                        //add object to tile objects list
                        ttile->addObject(tobj);

                        //update tile's object
                        updateObject(tobj, ttile);

                        //get next linked object
                        objindex = tobj->getNext();
                }
            }
        }

    }



    //print level 1 debug
    //mLevels[0].printDebug();

    ifile.close();

    std::cout << std::dec;
    return 0;
}

int Game::loadPalette()
{
    std::string palfile = "UWDATA\\pals.dat";
    std::ifstream pfile;
    pfile.open(palfile.c_str(), std::ios_base::binary);

    //check if file loaded
    if(!pfile.is_open()) return -1; // error unable to open file

    //resize palettes for 256
    m_Palettes.resize(8);
    for(int i = 0; i < int(m_Palettes.size()); i++) m_Palettes[i].resize(256);

    //UW palette intensities are 64, mult by 4 to get 256 intensity
    // (3x 8-bit r, g, b) = 1 index
    // each palette has 256 indices
    // pal file has 8 palettes

    for(int i = 0; i < int(m_Palettes.size()); i++)
    {
        for(int p = 0; p < int(m_Palettes[i].size()); p++)
        {
            unsigned char rgb[3];

            //read in color data (0-63 intensity for red, green , and blue)
            readBin(&pfile, rgb, 3);

            //index 0 always = transparent
            if(p == 0) m_Palettes[i][p] = SColor(TRANSPARENCY_COLOR );
            else m_Palettes[i][p] = SColor(255, (int(rgb[0])+1)*4-1, (int(rgb[1])+1)*4-1, (int(rgb[2])+1)*4-1 );

        }
    }

    pfile.close();

    return 0;
}

int Game::loadAuxPalette()
{
    std::string palfile = "UWDATA\\allpals.dat";
    std::ifstream pfile;
    pfile.open(palfile.c_str(), std::ios_base::binary);

    // aux palettes are indices into palette #0, so make sure it exists first!
    if(m_Palettes.empty()) return -2;

    //check if file loaded
    if(!pfile.is_open()) return -1; // error unable to open file

    //resize palettes for 16 colors x 31 palettes
    m_AuxPalettes.resize(31);
    for(int i = 0; i < int(m_AuxPalettes.size()); i++) m_AuxPalettes[i].resize(16);

    //UW palette intensities are 64, mult by 4 to get 256 intensity
    // (3x 8-bit r, g, b) = 1 index
    // each palette has 256 indices
    // pal file has 8 palettes

    for(int i = 0; i < int(m_AuxPalettes.size()); i++)
    {
        for(int p = 0; p < int(m_AuxPalettes[i].size()); p++)
        {
            unsigned char palindex[1];

            //read in color data (0-63 intensity for red, green , and blue)
            if(!readBin(&pfile, palindex, 1)) return -3; // error reading aux pal byte

            //index 0 always = transparent
            m_AuxPalettes[i][p] = m_Palettes[0][int(palindex[0])];
        }
    }

    pfile.close();

    return 0;
}

int Game::loadTexture(std::string tfilename, std::vector<ITexture*> *tlist)
{
    if(tlist == NULL) return -1; //error texture list is null

    //open texture file
    std::ifstream ifile;
    ifile.open(tfilename.c_str(), std::ios_base::binary);

    //check if texture file loaded properly
    if(!ifile.is_open()) return -2; // error unable to open file


    //temp variables
    unsigned char headerbuf[2];
    unsigned char txtcountbuf[2];
    std::vector<std::streampos> offsets;
    int txtdim = 0;
    int txtcount = 0;

    //read texture header
    readBin(&ifile, headerbuf, 2);
    readBin(&ifile, txtcountbuf, 2);

    //set variable data from header
    txtdim = int(headerbuf[1]);
    txtcount = lsbSum(txtcountbuf, 2);

    //read in texture offsets
    for(int i = 0; i < txtcount; i++)
    {
        unsigned char offsetbuf[4];
        readBin(&ifile, offsetbuf, 4);

        //store texture offsets into indexed list
        offsets.push_back(std::streampos(lsbSum(offsetbuf, 4)) );

        //std::cout << std::dec << "texture offset " << i << ": 0x" << std::hex << offsets.back() << std::endl;
    }

    //read each texture from file offset into an opengl texture
    for(int i = 0; i < txtcount; i++)
    {
        ITexture *newtxt = NULL;
        IImage *newimg = NULL;
        int palSel = 0; //  wall/floor textures always use palette 0

        //set the stream position to offset
        ifile.seekg(offsets[i]);

        //create image using texture dimension size
        newimg = m_Driver->createImage(ECF_A1R5G5B5, dimension2d<u32>(txtdim, txtdim));

        //offset points to dim^2 bytes long data where each byte points to palette index
        for(int n = 0; n < txtdim; n++)
        {
            for(int p = 0; p < txtdim; p++)
            {
                unsigned char pindex[1];

                //error reading?
                if(!readBin(&ifile, pindex, 1))
                {
                    std::cout << "Error reading texture at 0x" << std::hex << offsets[i] << std::endl;
                    return -8; // error reading texture at offset
                }


                //set the pixel at x,y using current selected palette with read in palette index #
                newimg->setPixel(p, n, m_Palettes[palSel][int(pindex[0])]);
            }
        }

        //create texture name
        std::stringstream texturename;
        texturename << "txt_" << i;

        //create texture from image
        newtxt = m_Driver->addTexture( texturename.str().c_str(), newimg );

        //push texture into texture list
        tlist->push_back(newtxt);

        //drop image, no longer needed
        newimg->drop();
    }
    ifile.close();

    return 0;
}

int Game::loadGraphic(std::string tfilename, std::vector<ITexture*> *tlist)
{
    if(tlist == NULL) return false;

    //open graphic (.gr) file
    std::ifstream ifile;
    ifile.open(tfilename.c_str(), std::ios_base::binary);

    //check if graphic file loaded properly
    if(!ifile.is_open()) return -1; // error loading file

    //check if palettes have been loaded first
    if(m_Palettes.empty() || m_AuxPalettes.empty()) return -13; // error palettes are empty

    //temp vars
    unsigned char fformatbuf[1];
    unsigned char bitmapcntbuf[2];
    std::vector<std::streampos> offsets;
    //int fformat = 0;
    int bitmapcnt = 0;

    //read header data
    if(!readBin(&ifile, fformatbuf, 1)) return -2; // error reading header format
    //fformat = int(fformatbuf[0]);
    if(!readBin(&ifile, bitmapcntbuf, 2) ) return -3; // error reading header bitmap count
    bitmapcnt = lsbSum(bitmapcntbuf, 2);

    //for each bitmap count, read in offsets
    for(int i = 0; i < bitmapcnt; i++)
    {

        unsigned char offsetbuf[4];
        if(!readBin(&ifile, offsetbuf, 4)) return -4; // error reading offset

        offsets.push_back( std::streampos( lsbSum(offsetbuf, 4)));

        //std::cout << "Offset " << i << " = 0x" << std::hex << offsets.back() << std::dec << std::endl;
    }

    //if bitmap count and offset count do not match, something went wrong!
    if(bitmapcnt != int(offsets.size()) )
    {
        std::cout << std::dec << "Graphics file bitmap count (" << bitmapcnt << ") != offset count (" << offsets.size() << ") !!\n";
        ifile.close();
        return false;
    }

    //read in each bitmap at offset
    for(int i = 0; i < bitmapcnt; i++)
    {

        //each bitmap at offset has its own header
        unsigned char btypebuf[1];
        unsigned char bwidthbuf[1];
        unsigned char bheightbuf[1];
        unsigned char bauxpalbuf[1];
        unsigned char bsizebuf[2];
        int btype;
        int bwidth;
        int bheight;
        int bauxpal;
        int bsize;

        //texture
        ITexture *newtxt = NULL;
        IImage *newimg = NULL;
        int palSel = 0; //  note : 4-bit images use aux pals, standard images use pal 0

        //jump to offset
        ifile.seekg(offsets[i]);

        //read in header and set data
        //bitmap data is read differently depending on what bitmap type it is
        // types are:
        //          0x04 = 8bit uncompressed
        //          0x08 = 4bit run length
        //          0x0A = 4bit uncompressed
        if(!readBin(&ifile, btypebuf, 1)) return -6; // error reading bitmap type
        btype = int(btypebuf[0]);

        if(!readBin(&ifile, bwidthbuf, 1))
        {
            std::cout << "Error reading binary file " << tfilename << " at offset " << std::hex << "0x" << offsets[i] << std::endl;
            std::cout << "Ignoring...\n";
            ifile.clear();
            ifile.close();
            std::cout << std::dec;
            return 0;
            return -7; // error reading bitmap width
        }
        bwidth = int(bwidthbuf[0]);

        if(!readBin(&ifile, bheightbuf, 1)) return -8; // error reading bitmap height
        bheight = int(bheightbuf[0]);


        //create new image using bitmap dimensions
        newimg = m_Driver->createImage(ECF_A1R5G5B5, dimension2d<u32>(bwidth, bheight));
        if(newimg == NULL) return -5; // error creating image

        //NOTE 4-bit images also have an auxillary palette selection byte
        //if 4-bit uncompressed, read in aux pal byte
        if(btype == 0x0a || btype == 0x08)
        {
            if(!readBin(&ifile, bauxpalbuf, 1)) return -9; // error reading auxillary palette
            bauxpal = int(bauxpalbuf[0]);
        }

        //get size
        // note : for 4 bit, this is nibble count, not byte count
        //if not uncompressed, read in size
        if(!readBin(&ifile, bsizebuf, 2)) return -10; // error reading size
        bsize = lsbSum(bsizebuf, 2);

        //read bitmap data
        //if uncompressed format - 8 bit
        if(btype == 0x04)
        {
            for(int n = 0; n < bheight; n++)
            {
                for(int p = 0; p < bwidth; p++)
                {
                    //read in one byte at a time
                    unsigned char bbyte[1];

                    if(!readBin(&ifile, bbyte, 1)) return -11; // error reading image data

                        //set the pixel at x,y using current selected palette with read in palette index #
                        newimg->setPixel(p, n, m_Palettes[palSel][int(bbyte[0])]);

                }
            }
        }
        //else if uncompressed format - 4bit
        else if(btype == 0x0a)
        {
            //read in 4-bit stream using size for nibble count
            std::vector<int> nibbles;
            nibbles.resize(bwidth*bheight);

            //read in entire stream
            unsigned char bstream[bsize];
            readBin(&ifile, bstream, bsize);

            //parse each byte by nibble, high nibble first, then lo
            for(int k = 0; k < bsize; k++)
            {
                int lobyte = getBitVal( int(bstream[k]), 0, 4);
                int hibyte = getBitVal( int(bstream[k]), 4, 4);
                nibbles[k*2] = hibyte;
                nibbles[k*2+1] = lobyte;
            }

            //set image pixel using image height and width to pull from nibble index
            for(int n = 0; n < bheight; n++)
            {
                for(int p = 0; p < bwidth; p++)
                {
                    newimg->setPixel(p, n, m_AuxPalettes[bauxpal][  nibbles[ (n*bwidth) + p]  ]);
                }
            }
        }
        // compressed bitmap
        else if(btype == 0x08)
        {
            std::vector<int> nibbledata;
            std::vector<int> pixeldata;

            int nibindex = 0;
            int nibsize = 4;


            for(int n = 0; n < bsize; n++)
            {
                unsigned char nbuf[1];
                if(!readBin(&ifile, nbuf, 1)) {ifile.close(); return -20;}

                nibbledata.push_back( getBitVal(int(nbuf[0]), 4, 4 ) );
                n++;
                if(n < bsize) nibbledata.push_back( getBitVal(int(nbuf[0]), 0, 4 ) );
            }


            //debug
            /*
            for(int n = 0; n < int(nibbledata.size()); n++)
            {
                std::cout << n << " : " << nibbledata[n] << std::endl;
            }
            */

            bool dorunrecord = false;

            do
            {
                //get count
                int rcount = getCount(nibbledata, &nibindex);
                /*
                if(!dorunrecord) std::cout << "repeat -- ";
                else std::cout << "run    -- ";
                std::cout << "index = " << nibindex << "   --   count = " << rcount << std::endl;
                */

                //if count is 2, do repeat
                if(rcount == 2 && !dorunrecord)
                {
                    //get repeat count
                    nibindex++;
                    int repeatcount = getCount(nibbledata, &nibindex);

                    //do repeat
                    for(int n = 0; n < repeatcount; n++)
                    {
                        nibindex++;
                        int altcount = getCount(nibbledata, &nibindex);

                        nibindex++;
                        for(int k = 0; k < altcount; k++) pixeldata.push_back(nibbledata[nibindex]);
                    }
                }
                //process count
                else if(rcount > 1 && !dorunrecord)
                {
                    nibindex++;
                    //repeat nibble by count times
                    for(int n = 0; n < rcount; n++) pixeldata.push_back(nibbledata[nibindex]);
                }
                else if(dorunrecord)
                {
                    //read in raw pixel data
                    for(int n = 0; n < rcount; n++)
                    {
                        nibindex++;
                        pixeldata.push_back( nibbledata[nibindex]);
                    }
                }
                //else if count == 1, ignore this record

                //change modes between run / repeat record
                dorunrecord = !dorunrecord;

                nibindex++;
            }while(nibindex < bsize); // done parsing nibbles


            //create image data
            for(int n = 0; n < bheight; n++)
            {
                for(int k = 0; k < bwidth; k++)
                {
                    newimg->setPixel(k, n, m_AuxPalettes[bauxpal][  pixeldata[ (n*bwidth) + k]  ]);
                }
            }


        }
        else
        {
            std::cout << "Unrecognized graphic type : " << std::hex << "0x" << btype << std::dec << std::endl;
            ifile.close();
            return -14;
        }


        //create texture name
        std::stringstream texturename;
        texturename << "txt_" << i;



        //create texture from image
        IImage *stretchedimage = m_Driver->createImage(ECF_A1R5G5B5, dimension2d<u32>(bwidth*SCREEN_SCALE, bheight*SCREEN_SCALE));
        newimg->copyToScaling(stretchedimage);
        newtxt = m_Driver->addTexture( texturename.str().c_str(), stretchedimage );
        //set transparency color (pink, 255,0,255)
        //note : this is palette index #0, set automatically when
        //       loading in palettes (see loadPalette())
        m_Driver->makeColorKeyTexture(newtxt,  SColor(TRANSPARENCY_COLOR));

        if(newtxt == NULL) return -12; // error creating texture
        //push texture into texture list
        tlist->push_back(newtxt);

        //drop image, no longer needed
        newimg->drop();
        stretchedimage->drop();
    }

    ifile.close();

    std::cout << std::dec;

    return 0;
}

int Game::loadBitmap(std::string tfilename, std::vector<ITexture*> *tlist, int tpalindex)
{
    const int bitmap_width = 320;
    const int bitmap_height = 200;

    if(tlist == NULL) return -1; // vector list is null

    //open bitmap (.byt) file
    std::ifstream ifile;
    ifile.open(tfilename.c_str(), std::ios_base::binary);

    //check if bitmap file loaded properly
    if(!ifile.is_open()) return -2; // error unable to open file

    //check if palette is valid
    if(tpalindex < 0 || tpalindex >= int(m_Palettes.size()) )
    {
        std::cout << "Error loading bitmap : Invalid palette index # - " << tpalindex << std::endl;
        return -3;
    }

    //texture
    ITexture *newtxt = NULL;
    IImage *newimg = NULL;
    //create new image using bitmap dimensions
    newimg = m_Driver->createImage(ECF_A1R5G5B5, dimension2d<u32>(bitmap_width, bitmap_height));

    //read in each bitmap byte
    for(int i = 0; i < bitmap_height; i++)
    {
        for(int n = 0; n < bitmap_width; n++)
        {
            unsigned char bbyte[1];

            readBin(&ifile, bbyte, 1);

            newimg->setPixel(n, i, m_Palettes[tpalindex][int(bbyte[0])]);
        }
    }

    //create texture name
    std::stringstream texturename;
    texturename << "txt_" << tfilename;

    //create texture from image
    IImage *stretchedimage = m_Driver->createImage(ECF_A1R5G5B5, dimension2d<u32>(bitmap_width*SCREEN_SCALE, bitmap_height*SCREEN_SCALE));
    newimg->copyToScaling(stretchedimage);
    newtxt = m_Driver->addTexture( texturename.str().c_str(), stretchedimage );
    //set transparency color (pink, 255,0,255)
    //note : this is palette index #0, set automatically when
    //       loading in palettes (see loadPalette())
    m_Driver->makeColorKeyTexture(newtxt,  SColor(TRANSPARENCY_COLOR));

    //push texture into texture list
    tlist->push_back(newtxt);

    //drop image, no longer needed
    newimg->drop();
    stretchedimage->drop();

    ifile.close();

    return 0;
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
        std::cout << "Configuring billboard node...\n";
        configBillboardSceneNode(tbb);

        const float conversion = float(UNIT_SCALE)/float(TILE_UNIT);

        vector3df debugpos( (tilepos.Y*UNIT_SCALE) + conversion*float(TILE_UNIT-objpos.Y),
                            ( float(objpos.Z) / TILE_UNIT)+(float(OBJECT_SCALE)/2),
                            (tilepos.X*UNIT_SCALE) + conversion*float(objpos.X) );
        tbb->setPosition( debugpos);
    }

    return true;
}

bool Game::registerForCollision(IMeshSceneNode *tnode)
{
    //register collision stuff
    ITriangleSelector *selector = m_SMgr->createOctreeTriangleSelector(tnode->getMesh(), tnode);
    tnode->setTriangleSelector(selector);

    //calculate collision response elipsoid
    //const aabbox3d<f32>& mybbox = tnode->getBoundingBox();
    //vector3df mybboxradius = mybbox.MaxEdge - mybbox.getCenter();

    //add triangle to meta selector
    m_MetaTriangleSelector->addTriangleSelector(selector);

    //drop selector no longer needed and add animator to camera
    selector->drop();

/*
    //create collision response between meta selector and camera
    ISceneNodeAnimator* anim = m_SMgr->createCollisionResponseAnimator(m_MetaTriangleSelector, m_Camera, mybboxradius,vector3df(0,GRAVITY,0));

    m_Camera->addAnimator(anim);
    //animator no longer needed, drop it
    anim->drop();
*/

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
