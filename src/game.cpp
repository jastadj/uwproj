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
    std::cout << "Game started.\n";

    //init irrlicht
    std::cout << "Initialzing irrlicht...\n";
    if(!initIrrlicht()) {std::cout << "Error initializing irrlicht!\n"; return -1;}

    std::cout << "Initializing camera...\n";
    if(!initCamera()) {std::cout << "Error initializing camera!\n"; return -1;}


    //load UW data
    std::cout << "Loading level data...\n";
    if(!loadLevel()) {std::cout << "Error loading level data!\n"; return -1;}
    std::cout << "Loading palette data...\n";
    if(!loadPalette()) { std::cout << "Error loading palette!\n"; return -1;}
    std::cout << "Loading textures...\n";
    if(!loadTexture("UWDATA\\w64.tr", &m_Wall64TXT)) { std::cout << "Error loading textures!\n"; return -1;}
        else std::cout << "Loaded " << m_Wall64TXT.size() << " wall64 textures.\n";
    if(!loadTexture("UWDATA\\f32.tr", &m_Floor32TXT)) { std::cout << "Error loading textures!\n"; return -1;}
        else std::cout << "Loaded " << m_Floor32TXT.size() << " floor32 textures.\n";
    std::cout << "Loading graphics...\n";
    //note : this needs to be fixed, throws bad alloc, need to investigate parsing for graphics load
    //if(!loadGraphic("UWDATA\\charhead.gr", &m_CharHeadTXT)) {std::cout << "Error loading charhead graphic!\n"; return -1;}
    std::cout << "Loading bitmaps...\n";
    if(!loadBitmap("UWDATA\\pres1.byt", &m_Bitmaps, 5)) {std::cout << "Error loading bitmap!\n"; return -1;}
    if(!loadBitmap("UWDATA\\pres2.byt", &m_Bitmaps, 5)) {std::cout << "Error loading bitmap!\n"; return -1;}
    if(!loadBitmap("UWDATA\\main.byt", &m_Bitmaps, 0)) {std::cout << "Error loading bitmap!\n"; return -1;}
    if(!loadBitmap("UWDATA\\opscr.byt", &m_Bitmaps, 2)) {std::cout << "Error loading bitmap!\n"; return -1;}


    //mLevels[0].printDebug();

    //generate level geometry
    std::cout << "Generating level geometry...\n";
    if(!mLevels[0].buildLevelGeometry()) { std::cout << "Error generating level geometry!!\n"; return -1;}

    //start main loop
    std::cout << "Starting main loop...\n";
    mainLoop();

    return 0;
}

bool Game::initIrrlicht()
{
    //already initialized!!
    if(m_Device != NULL) return false;

    //init device
    bool fullscreen = false;
    m_Device = createDevice( video::EDT_OPENGL, dimension2d<u32>(SCREEN_WIDTH, SCREEN_HEIGHT), 16, fullscreen, false, false, &m_Receiver);
    if(!m_Device) {std::cout << "Error creating device.\n";return false;}
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

    return true;
}

bool Game::initCamera()
{
    //already initialized!!
    if(m_Camera != NULL) return false;

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








    return true;
}

/////////////////////////////////////////////////////////////
//  MAIN LOOP
void Game::mainLoop()
{
    bool drawaxis = true;

    //add light test
    scene::ILightSceneNode* light1 = m_SMgr->addLightSceneNode(0, vector3df(0,0,0), SColorf(1.0f, 0.5f, 0.5f, 0.f), 100.0f);
    light1->setLightType(video::ELT_SPOT);
    light1->setRotation(vector3df(0,180,0));
    light1->getLightData().Falloff = 5;
    //light1->getLightData().DiffuseColor = SColor(100,100,100,100);
    //light1->getLightData().SpecularColor = SColor(0,0,0,0);
    //light1->enableCastShadow(false);
    m_SMgr->setAmbientLight( SColor(100,100,100,100));



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


    IGUIImage *myguiimage = m_GUIEnv->addImage(m_Bitmaps[2], position2d<s32>(0,0), true);



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

        if(m_Receiver.isKeyPressed(KEY_KEY_Q))
        {
            m_CameraVel.Y = frameDeltaTime * MOVE_SPEED;
        }
        else if(m_Receiver.isKeyPressed(KEY_KEY_E))
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
                    else if(m_Receiver.isKeyPressed(KEY_F1))
                    {
                        m_CameraPos = m_Camera->getPosition();
                        Tile *ttile = mLevels[m_CurrentLevel].getTile(int(m_CameraPos.Z)/UNIT_SCALE, int(m_CameraPos.X)/UNIT_SCALE);
                        if(ttile != NULL)
                        {
                            ttile->printDebug();
                        }
                        else std::cout << "Current tile = NULL!\n";
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

        //m_CameraPos = m_Camera->getPosition();
        //m_Camera->setPosition( vector3df(m_CameraPos.X, m_CameraPos.Y-0.1*frameDeltaTime, m_CameraPos.Z));



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
    if(pos == NULL || vel == NULL) return false;

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


    //if moving northward
    if(vel->X < 0)
    {
        //is position close enough to wall?
        if(tsubpos.X <= 1)
        {
            //is there a tile in direction?
            if(adjtiles[NORTH])
            {
                //is that tile solid?
                if(adjtiles[NORTH]->getType() == TILETYPE_SOLID)
                {

                }
            }
        }
    }
    //moving southward?
    else if(vel->X > 0)
    {
        std::cout << "moving southward\n";
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
                    pos->X += -vel->X;
                    vel->X = 0;
                    std::cout << "south wall collision!\n";
                }
            }
        }
    }

    //temp debug
    pos->Y = ttile->getHeight()+3;

    return true;
}


bool Game::loadLevel()
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
    if(!ifile.is_open())
    {
        std::cout << "Error opening " << tfile << std::endl;
        return false;
    }
    else std::cout << "Successfuly opened " << tfile << std::endl;

    //read block count from header
    unsigned char blockcountbuf[2];
    readBin(&ifile, blockcountbuf, 2);
    blockcount = uint16_t(lsbSum(blockcountbuf, 2));
    std::cout << std::dec << "Block count:" << blockcount << std::endl;

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


    }



    //print level 1 debug
    //mLevels[0].printDebug();

    ifile.close();

    std::cout << std::dec;
    return true;
}

bool Game::loadPalette()
{
    std::string palfile = "UWDATA\\pals.dat";
    std::ifstream pfile;
    pfile.open(palfile.c_str(), std::ios_base::binary);

    //check if file loaded
    if(!pfile.is_open())
    {
        std::cout << "Error loading " << palfile << std::endl;
        return false;
    }
    else std::cout << "Successfuly loaded " << palfile << std::endl;


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
            if(p == 0) m_Palettes[i][p] = SColor(0, 0, 0, 0 );
            else m_Palettes[i][p] = SColor(255, (int(rgb[0])+1)*4-1, (int(rgb[1])+1)*4-1, (int(rgb[2])+1)*4-1 );

        }
    }

    pfile.close();

    return true;
}

bool Game::loadTexture(std::string tfilename, std::vector<ITexture*> *tlist)
{
    if(tlist == NULL) return false;

    //open texture file
    std::ifstream ifile;
    ifile.open(tfilename.c_str(), std::ios_base::binary);

    //check if texture file loaded properly
    if(!ifile.is_open())
    {
        std::cout << "Error loading " << tfilename << std::endl;
        return false;
    }


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
                    return false;
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

    return true;
}

bool Game::loadGraphic(std::string tfilename, std::vector<ITexture*> *tlist)
{
    if(tlist == NULL) return false;

    //open graphic (.gr) file
    std::ifstream ifile;
    ifile.open(tfilename.c_str(), std::ios_base::binary);

    //check if graphic file loaded properly
    if(!ifile.is_open())
    {
        std::cout << "Error loading " << tfilename << std::endl;
        return false;
    }

    //temp vars
    unsigned char fformatbuf[1];
    unsigned char bitmapcntbuf[2];
    std::vector<std::streampos> offsets;
    //int fformat = 0;
    int bitmapcnt = 0;

    //read header data
    readBin(&ifile, fformatbuf, 1);
    //fformat = int(fformatbuf[0]);
    readBin(&ifile, bitmapcntbuf, 2);
    bitmapcnt = lsbSum(bitmapcntbuf, 2);

    //for each bitmap count, read in offsets
    for(int i = 0; i < bitmapcnt; i++)
    {
        unsigned char offsetbuf[4];
        readBin(&ifile, offsetbuf, 4);

        offsets.push_back( std::streampos( lsbSum(offsetbuf, 4)));
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
        int palSel = 0; //  wall/floor textures always use palette 0

        //create new image using bitmap dimensions
        newimg = m_Driver->createImage(ECF_A1R5G5B5, dimension2d<u32>(bwidth, bheight));

        //jump to offset
        ifile.seekg(offsets[i]);

        //read in header and set data
        //bitmap data is read differently depending on what bitmap type it is
        // types are:
        //          0x04 = 8bit uncompressed
        //          0x08 = 4bit run length
        //          0x0A = 4bit uncompressed
        readBin(&ifile, btypebuf, 1);
        btype = int(btypebuf[0]);

        readBin(&ifile, bwidthbuf, 1);
        bwidth = int(bwidthbuf[0]);

        readBin(&ifile, bheightbuf, 1);
        bheight = int(bheightbuf[0]);

        //NOTE 4-bit images also have an auxillary palette selection byte
        //if 4-bit uncompressed, read in aux pal byte
        if(btype == 0x0a)
        {
            readBin(&ifile, bauxpalbuf, 1);
            bauxpal = int(bauxpalbuf[0]);
        }

        //get size
        readBin(&ifile, bsizebuf, 2);
        bsize = lsbSum(bsizebuf, 2);


        //read bitmap data
        //if uncompressed format
        if(btype == 0x04 || btype == 0x0a)
        {
            for(int n = 0; n < bheight; n++)
            {
                for(int p = 0; p < bwidth; p++)
                {
                    //read in one byte at a time
                    unsigned char bbyte[1];

                    readBin(&ifile, bbyte, 1);

                    //if 8-bit
                    if(btype == 0x04)
                    {
                        //set the pixel at x,y using current selected palette with read in palette index #
                        newimg->setPixel(p, n, m_Palettes[palSel][int(bbyte[0])]);
                    }
                    else if(btype == 0x0a)
                    {
                        //first hi byte, the lo byte
                        int hibyte = getBitVal( int(bbyte), 4, 4);
                        int lobyte = getBitVal( int(bbyte), 0, 4);

                        //set the pixel at x,y using current selected palette with read in palette index #
                        newimg->setPixel(p, n, m_Palettes[palSel][hibyte]);
                        newimg->setPixel(p, n, m_Palettes[palSel][lobyte]);

                        //increase p counter (since we did two in one loop)
                        p++;
                    }


                }
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

    return true;
}

bool Game::loadBitmap(std::string tfilename, std::vector<ITexture*> *tlist, int tpalindex)
{
    const int bitmap_width = 320;
    const int bitmap_height = 200;

    if(tlist == NULL) return false;

    //open bitmap (.byt) file
    std::ifstream ifile;
    ifile.open(tfilename.c_str(), std::ios_base::binary);

    //check if bitmap file loaded properly
    if(!ifile.is_open())
    {
        std::cout << "Error loading " << tfilename << std::endl;
        return false;
    }

    //check if palette is valid
    if(tpalindex < 0 || tpalindex >= int(m_Palettes.size()) )
    {
        std::cout << "Error loading bitmap : Invalid palette index # - " << tpalindex << std::endl;
        return false;
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
    m_Driver->makeColorKeyTexture(newtxt,  m_Palettes[tpalindex][0]);

    //push texture into texture list
    tlist->push_back(newtxt);

    //drop image, no longer needed
    newimg->drop();
    stretchedimage->drop();

    ifile.close();

    return true;
}

bool Game::configMeshSceneNode(IMeshSceneNode *tnode)
{
    if(tnode == NULL) return false;

    tnode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, true);
    tnode->setMaterialFlag(video::EMF_LIGHTING, false);
    //tnode->setMaterialFlag(video::EMF_TEXTURE_WRAP, true);
    //tnode->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
    //tnode->setMaterialFlag(video::EMF_BLEND_OPERATION, true);
    //tnode->setMaterialType(video::EMT_PARALLAX_MAP_SOLID);
    tnode->setMaterialFlag(video::EMF_BILINEAR_FILTER, false );
    tnode->setMaterialFlag(video::EMF_TRILINEAR_FILTER, false );
    tnode->setMaterialFlag(video::EMF_ANISOTROPIC_FILTER, false );

    //automatic culling
    tnode->setAutomaticCulling(EAC_FRUSTUM_SPHERE ) ;

    //texture repeating
    tnode->getMaterial(0).getTextureMatrix(0).setScale(1);
    tnode->getMaterial(0).TextureLayer->TextureWrapU = video::ETC_REPEAT;
    tnode->getMaterial(0).TextureLayer->TextureWrapV = video::ETC_REPEAT;

    //update
    tnode->updateAbsolutePosition();

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

