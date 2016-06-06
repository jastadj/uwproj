#include "game.hpp"
#include "tools.hpp"

#include <sstream>

Game *Game::mInstance = NULL;

Game::Game()
{
    //init pointers
    m_Device = NULL;
    m_Camera = NULL;

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
    m_Device = createDevice( video::EDT_OPENGL, dimension2d<u32>(800, 600), 16, false, false, false, &m_Receiver);
    if(!m_Device) {std::cout << "Error creating device.\n";return false;}
    m_Device->setWindowCaption(L"UWproj");

    //get video
    m_Driver = m_Device->getVideoDriver();

    //get scene manager
    m_SMgr = m_Device->getSceneManager();
    //get gui environment
    m_GUIEnv = m_Device->getGUIEnvironment();
    //get collision manager
    m_IMgr = m_SMgr->getSceneCollisionManager();

    //draw screen
    m_Driver->endScene();

    return true;
}

bool Game::initCamera()
{
    //already initialized!!
    if(m_Camera != NULL) return false;

    //add camera to scene
    //m_CameraTarget = vector3df(0,0,0);
    //m_CameraPos = vector3df(0,4,0);
    //m_Camera = m_SMgr->addCameraSceneNode(0, m_CameraPos, m_CameraTarget);
    m_Camera = m_SMgr->addCameraSceneNodeFPS();
    m_Camera->setPosition( vector3df(0,4,0));

    core::list<ISceneNodeAnimator*>::ConstIterator anim = m_Camera->getAnimators().begin();
    ISceneNodeAnimatorCameraFPS *animfps = (ISceneNodeAnimatorCameraFPS*)(*anim);
    animfps->setMoveSpeed(0.05);

    //level camera (avoid camera rotation when pointing at target)
     //m_Camera->setUpVector(vector3df(1,0,0));

    //capture default FOV
    m_CameraDefaultFOV = m_Camera->getFOV();

    return true;
}

void Game::updateCamera(vector3df cameratargetpos)
{
    //std::cout << m_Camera->getRotation().X << "," << m_Camera->getRotation().Y << "," << m_Camera->getRotation().Z << std::endl;

    //m_CameraTarget = cameratargetpos;
    m_Camera->setPosition(m_CameraPos);
    //m_Camera->setTarget(m_CameraTarget);
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

            m_Palettes[i][p] = SColor(255, (int(rgb[0])+1)*4-1, (int(rgb[1])+1)*4-1, (int(rgb[2])+1)*4-1 );

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
    int msize = 12;
    f32 cubesize = 50;
    SMesh *cubemesh = getCubeMesh(cubesize);
    std::vector< std::vector<ISceneNode*> > mymap;
    mymap.resize(msize);
    for(int i = 0; i < msize; i++)
    {
        for(int n = 0; n < msize; n++)
        {
            vector3df cubepos(cubesize*n, cubesize*i, 0);
            IMeshSceneNode *mycube = m_SMgr->addMeshSceneNode(cubemesh);
            mymap[i].push_back(mycube);
            if(mycube)
            {
                mycube->setPosition(vector3df(n*cubesize, i*cubesize, 0));
                mycube->setMaterialTexture(0, m_Driver->getTexture(".\\Data\\Art\\wall.jpg"));
                mycube->setMaterialFlag(video::EMF_BACK_FACE_CULLING, true);
                //mycube->setMaterialFlag(video::EMF_LIGHTING, false);
                //mycube->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
                //mycube->setMaterialFlag(video::EMF_BLEND_OPERATION, true);
                //mycube->setMaterialType(video::EMT_PARALLAX_MAP_SOLID);
                mycube->updateAbsolutePosition();

            }
        }
    }
*/

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

    //std::vector<IMeshSceneNode*> testtiles = generateTileMeshes(mLevels[0].getTile(30,2), 0, 0);
    //if(testtiles.empty()) std::cout << "DID NOT GENERATE TILES!\n";

    //generate level meshes
/*
    for(int i = 0; i < TILE_ROWS; i++)
    {
        for(int n = 0; n < TILE_COLS; n++)
        {
            std::vector<IMeshSceneNode*> tilemeshes;

            Tile *ttile = mLevels[m_CurrentLevel].getTile(n,i);

            tilemeshes = generateTileMeshes(ttile, n, i);

            //dump meshes into master list
            for(int p = 0; p < int(tilemeshes.size()); p++)
            {
                mLevelMeshes.push_back(tilemeshes[p]);
            }
        }
    }
*/

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
        const f32 frameDeltaTime = (f32)(now - then) / 1000.f; // Time in seconds
        then = now;


        //check if keys are held for movement
        if(m_Receiver.isKeyPressed(KEY_KEY_A))
        {

        }
        else if(m_Receiver.isKeyPressed(KEY_KEY_D))
        {

        }
        else if(m_Receiver.isKeyPressed(KEY_KEY_W))
        {

        }
        else if(m_Receiver.isKeyPressed(KEY_KEY_S))
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
                    else if(event->KeyInput.Key == KEY_SPACE) m_CameraPos.Z += 10;
                    else if(event->KeyInput.Key == KEY_KEY_E)
                    {

                        /*
                        txtindex++;
                        if(txtindex >= int(m_Wall64TXT.size()) ) txtindex = 0;
                        mysquare->setMaterialTexture(0, m_Wall64TXT[txtindex]);
                        std::cout << "TEXTURE INDEX = " << txtindex << std::endl;
                        */


                    }
                    else if(event->KeyInput.Key == KEY_KEY_Q)
                    {

                        /*
                        txtindex--;
                        if(txtindex < 0) txtindex = int(m_Wall64TXT.size()-1);
                        mysquare->setMaterialTexture(0, m_Wall64TXT[txtindex]);
                        std::cout << "TEXTURE INDEX = " << txtindex << std::endl;
                        */

                    }
                    else if(event->KeyInput.Key == KEY_KEY_T)
                    {

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
        //updateCamera(vector3df(0,0,0));
        //m_CameraPos = m_Camera->getPosition();
        //m_Camera->setPosition( vector3df(m_CameraPos.X, m_CameraPos.Y-0.1*frameDeltaTime, m_CameraPos.Z));



        //clear scene
        m_Driver->beginScene(true, true, SColor(255,100,101,140));

        //current floor plane
        plane3df myplane(vector3df(0,0,-25), vector3df(0,0,1));
        //line from camera to mouse cursor
        line3df myline = m_IMgr->getRayFromScreenCoordinates(m_MousePos);
        vector3df myint;
        //get intersection of camera_mouse_line to the floor plane
        myplane.getIntersectionWithLimitedLine(myline.end, myline.start, myint);


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
        m_GUIEnv->drawAll();

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

    //texture repeating
    tnode->getMaterial(0).getTextureMatrix(0).setScale(1);
    tnode->getMaterial(0).TextureLayer->TextureWrapU = video::ETC_REPEAT;
    tnode->getMaterial(0).TextureLayer->TextureWrapV = video::ETC_REPEAT;

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

        Mesh->setBoundingBox( aabbox3df(0,0,0,cubesize,cubesize,-cubesize));
        //buf->recalculateBoundingBox();


        return Mesh;

}

SMesh *Game::getSquareMesh(f32 width, f32 height)
{

        SMesh* Mesh = new SMesh();

        int vcount = 6;

        SMeshBuffer *buf = new SMeshBuffer();
        Mesh->addMeshBuffer(buf);
        buf->drop();

        buf->Vertices.reallocate(vcount);
        buf->Vertices.set_used(vcount);

        //top
        buf->Vertices[0] = S3DVertex(0*width,0*width,0*width, 0,0,1,    video::SColor(255,255,255,255), 0, 0); //TL
        buf->Vertices[1] = S3DVertex(1*width,0*width,0*width, 0,0,1,  video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[2] = S3DVertex(0*width,1*width,0*width, 0,0,1,    video::SColor(255,255,255,255), 0, 1);// BL

        buf->Vertices[3] = S3DVertex(1*width,0,0, 0,0,1,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[4] = S3DVertex(1*width,1*width,0, 0,0,1,    video::SColor(255,255,255,255), 1, 1); //BR
        buf->Vertices[5] = S3DVertex(0,1*width,0, 0,0,1,    video::SColor(255,255,255,255), 0, 1); // BL


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

        Mesh->setBoundingBox( aabbox3df(0,0,0,width,width,0));
        //buf->recalculateBoundingBox();

        return Mesh;

}

SMesh *Game::generateWallMesh(int tl, int tr, int bl, int br)
{
    SMesh *mesh = NULL;
    SMeshBuffer *buf = NULL;

    const int vcount = 6;
    int scale = UNIT_SCALE/4;
    //int heightval = (CEIL_HEIGHT-UNIT_SCALE+1)*(UNIT_SCALE/4);

    //WALL MESH
    mesh = new SMesh();
    buf = new SMeshBuffer();

    mesh->addMeshBuffer(buf);
    buf->drop();

    buf->Vertices.reallocate(vcount);
    buf->Vertices.set_used(vcount);

    //triangle 1
    buf->Vertices[0] = S3DVertex(0*scale,tl*scale,0*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 0, 0); //TL
    buf->Vertices[1] = S3DVertex(0*scale,tr*scale,1*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 1, 0); //TR
    buf->Vertices[2] = S3DVertex(0*scale,bl*scale,0*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 0, 1);// BL
    //triangle 2
    buf->Vertices[3] = S3DVertex(0*scale,tr*scale,1*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 1, 0); //TR
    buf->Vertices[4] = S3DVertex(0*scale,br*scale,1*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 1, 1); //BR
    buf->Vertices[5] = S3DVertex(0*scale,bl*scale,0*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 0, 1); // BL

    //finalize vertices
    buf->Indices.reallocate(vcount);
    buf->Indices.set_used(vcount);
    for(int i = 0; i < vcount; i++) buf->Indices[i] = i;
    mesh->setBoundingBox( aabbox3df(0,tl*scale,0,0,br*scale,1*scale));
    //buf->recalculateBoundingBox();

    return mesh;
}

SMesh *Game::generateDiagonalWallMesh(int tiletype, int tl, int tr, int bl, int br)
{
    SMesh *mesh = NULL;
    SMeshBuffer *buf = NULL;

    const int vcount = 6;
    int scale = UNIT_SCALE/4;
    //int heightval = (CEIL_HEIGHT-UNIT_SCALE+1)*(UNIT_SCALE/4);

    if(tiletype < 2 || tiletype > 5)
    {
        std::cout << "ERROR!! Unable to create diagonal wall - wrong type!!\n";
        return new SMesh();
    }

    //WALL MESH
    mesh = new SMesh();
    buf = new SMeshBuffer();

    mesh->addMeshBuffer(buf);
    buf->drop();

    buf->Vertices.reallocate(vcount);
    buf->Vertices.set_used(vcount);

    if(tiletype == 2 || tiletype == 5) //diagonal, open SE or open NW (requires rotation)
    {
        //triangle 1
        buf->Vertices[0] = S3DVertex(1*UNIT_SCALE,tl*scale,0*UNIT_SCALE, 1,0,1,    video::SColor(255,255,255,255), 0, 0); //TL
        buf->Vertices[1] = S3DVertex(0*UNIT_SCALE,tr*scale,1*UNIT_SCALE, 1,0,1,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[2] = S3DVertex(1*UNIT_SCALE,bl*scale,0*UNIT_SCALE, 1,0,1,    video::SColor(255,255,255,255), 0, 1);// BL
        //triangle 2
        buf->Vertices[3] = S3DVertex(0*UNIT_SCALE,tr*scale,1*UNIT_SCALE, 1,0,1,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[4] = S3DVertex(0*UNIT_SCALE,br*scale,1*UNIT_SCALE, 1,0,1,    video::SColor(255,255,255,255), 1, 1); //BR
        buf->Vertices[5] = S3DVertex(1*UNIT_SCALE,bl*scale,0*UNIT_SCALE, 1,0,1,    video::SColor(255,255,255,255), 0, 1); // BL
    }
    else if(tiletype == 3 || tiletype == 4) // diagonal, open SW or NE (requires rotation)
    {
        //triangle 1
        buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,tl*scale,0*UNIT_SCALE, 1,0,-1,    video::SColor(255,255,255,255), 0, 0); //TL
        buf->Vertices[1] = S3DVertex(1*UNIT_SCALE,tr*scale,1*UNIT_SCALE, 1,0,-1,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[2] = S3DVertex(0*UNIT_SCALE,bl*scale,0*UNIT_SCALE, 1,0,-1,    video::SColor(255,255,255,255), 0, 1);// BL
        //triangle 2
        buf->Vertices[3] = S3DVertex(1*UNIT_SCALE,tr*scale,1*UNIT_SCALE, 1,0,-1,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[4] = S3DVertex(1*UNIT_SCALE,br*scale,1*UNIT_SCALE, 1,0,-1,    video::SColor(255,255,255,255), 1, 1); //BR
        buf->Vertices[5] = S3DVertex(0*UNIT_SCALE,bl*scale,0*UNIT_SCALE, 1,0,-1,    video::SColor(255,255,255,255), 0, 1); // BL
    }

    //finalize vertices
    buf->Indices.reallocate(vcount);
    buf->Indices.set_used(vcount);
    for(int i = 0; i < vcount; i++) buf->Indices[i] = i;
    mesh->setBoundingBox( aabbox3df(0,tl*scale,0,0,br*scale,1*scale));
    //buf->recalculateBoundingBox();

    return mesh;
}

SMesh *Game::generateFloorMesh(int tiletype)
{
    SMesh *mesh = NULL;
    SMeshBuffer *buf = NULL;

    int vcount = 0;
    int scale = UNIT_SCALE/4;
    //int heightval = (CEIL_HEIGHT-UNIT_SCALE+1)*(UNIT_SCALE/4);

    //FLOOR MESH
    mesh = new SMesh();
    buf = new SMeshBuffer();

    mesh->addMeshBuffer(buf);
    buf->drop();

    buf->Vertices.reallocate(vcount);
    buf->Vertices.set_used(vcount);

    //standard floor type
    if(tiletype == TILETYPE_OPEN)
    {
        vcount = 6;

        buf->Vertices.reallocate(vcount);
        buf->Vertices.set_used(vcount);

        //triangle 1
        buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 0); //TL
        buf->Vertices[1] = S3DVertex(0*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[2] = S3DVertex(1*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1);// BL
        //triangle 2
        buf->Vertices[3] = S3DVertex(0*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[4] = S3DVertex(1*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 1); //BR
        buf->Vertices[5] = S3DVertex(1*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1); // BL
    }
    else if(tiletype == TILETYPE_D_NE)
    {
        vcount = 3;
        buf->Vertices.reallocate(vcount);
        buf->Vertices.set_used(vcount);

        //triangle 1
        buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 0); //TL
        buf->Vertices[1] = S3DVertex(0*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[2] = S3DVertex(1*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 1);// BR
    }
    else if(tiletype == TILETYPE_D_NW)
    {
        vcount = 3;
        buf->Vertices.reallocate(vcount);
        buf->Vertices.set_used(vcount);

        //triangle 1
        buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 0); //TL
        buf->Vertices[1] = S3DVertex(0*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[2] = S3DVertex(1*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1);// BL
    }
    else if(tiletype == TILETYPE_D_SW)
    {
        vcount = 3;
        buf->Vertices.reallocate(vcount);
        buf->Vertices.set_used(vcount);

        //triangle 1
        buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 0); //TL
        buf->Vertices[1] = S3DVertex(1*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 1); //BR
        buf->Vertices[2] = S3DVertex(1*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1);// BL


    }
    else if(tiletype == TILETYPE_D_SE)
    {
        vcount = 3;
        buf->Vertices.reallocate(vcount);
        buf->Vertices.set_used(vcount);

        //triangle 1
        buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0);// TR
        buf->Vertices[1] = S3DVertex(1*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 1); //BR
        buf->Vertices[2] = S3DVertex(1*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1); //BL
    }
    else if(tiletype == TILETYPE_SL_N)
    {
        vcount = 6;

        buf->Vertices.reallocate(vcount);
        buf->Vertices.set_used(vcount);

        //triangle 1
        buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,1*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 0); //TL
        buf->Vertices[1] = S3DVertex(0*UNIT_SCALE,1*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[2] = S3DVertex(1*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1);// BL
        //triangle 2
        buf->Vertices[3] = S3DVertex(0*UNIT_SCALE,1*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[4] = S3DVertex(1*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 1); //BR
        buf->Vertices[5] = S3DVertex(1*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1); // BL
    }
    else if(tiletype == TILETYPE_SL_S)
    {
        vcount = 6;

        buf->Vertices.reallocate(vcount);
        buf->Vertices.set_used(vcount);

        //triangle 1
        buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 0); //TL
        buf->Vertices[1] = S3DVertex(0*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[2] = S3DVertex(1*UNIT_SCALE,1*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1);// BL
        //triangle 2
        buf->Vertices[3] = S3DVertex(0*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[4] = S3DVertex(1*UNIT_SCALE,1*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 1); //BR
        buf->Vertices[5] = S3DVertex(1*UNIT_SCALE,1*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1); // BL
    }
    else if(tiletype == TILETYPE_SL_E)
    {
        vcount = 6;

        buf->Vertices.reallocate(vcount);
        buf->Vertices.set_used(vcount);

        //triangle 1
        buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 0); //TL
        buf->Vertices[1] = S3DVertex(0*UNIT_SCALE,1*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[2] = S3DVertex(1*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1);// BL
        //triangle 2
        buf->Vertices[3] = S3DVertex(0*UNIT_SCALE,1*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[4] = S3DVertex(1*UNIT_SCALE,1*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 1); //BR
        buf->Vertices[5] = S3DVertex(1*UNIT_SCALE,0*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1); // BL
    }
    else if(tiletype == TILETYPE_SL_W)
    {
        vcount = 6;

        buf->Vertices.reallocate(vcount);
        buf->Vertices.set_used(vcount);

        //triangle 1
        buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,1*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 0); //TL
        buf->Vertices[1] = S3DVertex(0*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[2] = S3DVertex(1*UNIT_SCALE,1*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1);// BL
        //triangle 2
        buf->Vertices[3] = S3DVertex(0*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
        buf->Vertices[4] = S3DVertex(1*UNIT_SCALE,0*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 1); //BR
        buf->Vertices[5] = S3DVertex(1*UNIT_SCALE,1*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1); // BL
    }

    //finalize vertices
    buf->Indices.reallocate(vcount);
    buf->Indices.set_used(vcount);
    for(int i = 0; i < vcount; i++) buf->Indices[i] = i;
    mesh->setBoundingBox( aabbox3df(0,0,0,1*scale,0,1*scale));
    //buf->recalculateBoundingBox();

    return mesh;

}

std::vector<IMeshSceneNode*> Game::generateTileMeshes(Tile *ttile, int xpos, int ypos)
{
    std::vector<IMeshSceneNode*> meshlist;

    if(ttile == NULL) return meshlist;

    if(ttile->getType() == TILETYPE_SOLID) return meshlist;

    int vcount = 6;
    int scale = UNIT_SCALE;
    int heightval = (CEIL_HEIGHT-UNIT_SCALE+1)*(UNIT_SCALE/4);

    //temp pointers
    SMesh* Mesh = NULL;
    SMeshBuffer *buf = NULL;
    IMeshSceneNode *mysquare = NULL;


    //FLOOR MESH
    Mesh = generateFloorMesh(ttile->getType());

    //create scene object
    mysquare = m_SMgr->addMeshSceneNode(Mesh);
    mysquare->setPosition(vector3df(ypos*scale,(ttile->getHeight()-1)*(UNIT_SCALE/4),xpos*scale));
    //mysquare->setRotation(vector3df(180, 270, 0));
    mysquare->setMaterialTexture(0, m_Floor32TXT[ttile->getFloorTXT()]);
    mysquare->updateAbsolutePosition();
    Mesh->drop();
    meshlist.push_back(mysquare);

    //WALL MESH

    //if tile type is diagonal
    if(ttile->getType() >= 2 && ttile->getType() <= 5)
    {
        Mesh = generateDiagonalWallMesh(ttile->getType(), CEIL_HEIGHT, CEIL_HEIGHT, ttile->getHeight()-1, ttile->getHeight()-1);
        mysquare = m_SMgr->addMeshSceneNode(Mesh);

        if(ttile->getType() == TILETYPE_D_SE)
        {
            mysquare->setPosition(vector3df( (ypos*scale),0, (xpos*scale) ) );
            mysquare->setRotation(vector3df(0, 0, 0));
        }
        else if(ttile->getType() == TILETYPE_D_SW)
        {
            mysquare->setPosition(vector3df( (ypos*scale),0, (xpos*scale) ) );
            mysquare->setRotation(vector3df(0, 0, 0));
        }
        else if(ttile->getType() == TILETYPE_D_NE)
        {
            mysquare->setPosition(vector3df( (ypos*scale)+scale,0, (xpos*scale)+scale ) );
            mysquare->setRotation(vector3df(0, 180, 0));
        }
        else if(ttile->getType() == TILETYPE_D_NW)
        {
            mysquare->setPosition(vector3df( (ypos*scale)+scale,0, (xpos*scale)+scale ) );
            mysquare->setRotation(vector3df(0, 180, 0));
        }
        mysquare->setMaterialTexture(0, m_Wall64TXT[ttile->getWallTXT()]);
        mysquare->updateAbsolutePosition();

        meshlist.push_back(mysquare);
        Mesh->drop();
    }

    //create scene wall objects
    //south wall
    if(mLevels[m_CurrentLevel].getTile(xpos, ypos+1) != NULL)
    {
        //by default, wall height is top height
        int topheight = CEIL_HEIGHT;

        //get adjacent tile
        Tile *adj = mLevels[m_CurrentLevel].getTile(xpos, ypos+1);

        //if adjacent is higher
        if( adj->getHeight() > ttile->getHeight() &&
           ttile->getType() != TILETYPE_D_NE && ttile->getType() != TILETYPE_D_NW)
        {
            topheight = adj->getHeight()-1;

            //if adjacent is sloping and not higher
            if(adj->getType() != TILETYPE_SL_S && topheight != ttile->getHeight())
            {
                Mesh = generateWallMesh(topheight, topheight, ttile->getHeight()-1, ttile->getHeight()-1);

                mysquare = m_SMgr->addMeshSceneNode(Mesh);

                mysquare->setPosition(vector3df( (ypos*scale)+scale,0, (xpos*scale)+scale ) );
                mysquare->setRotation(vector3df(0, 180, 0));
                mysquare->setMaterialTexture(0, m_Wall64TXT[ttile->getWallTXT()]);
                mysquare->updateAbsolutePosition();

                meshlist.push_back(mysquare);
                Mesh->drop();
            }

        }
    }
    //north wall
    if(mLevels[m_CurrentLevel].getTile(xpos, ypos-1) != NULL)
    {
        //by default, wall height is top height
        int topheight = CEIL_HEIGHT;

        //get adjacent tile
        Tile *adj = mLevels[m_CurrentLevel].getTile(xpos, ypos-1);

        //if adjacent is higher, draw wall
        if( adj->getHeight() > ttile->getHeight() &&
           ttile->getType() != TILETYPE_D_SE && ttile->getType() != TILETYPE_D_SW )
        {
            topheight = adj->getHeight()-1;

            if(adj->getType() != TILETYPE_SL_N && topheight != ttile->getHeight())
            {
                Mesh = generateWallMesh(topheight, topheight, ttile->getHeight()-1, ttile->getHeight()-1);

                mysquare = m_SMgr->addMeshSceneNode(Mesh);

                mysquare->setPosition(vector3df( (ypos*scale),0, (xpos*scale) ) );
                mysquare->setRotation(vector3df(0, 0, 0));
                mysquare->setMaterialTexture(0, m_Wall64TXT[ttile->getWallTXT()]);
                mysquare->updateAbsolutePosition();

                meshlist.push_back(mysquare);
                Mesh->drop();
            }
        }

    }

    //east wall
    if(mLevels[m_CurrentLevel].getTile(xpos+1, ypos) != NULL)
    {
        //by default, wall height is top height
        int topheight = CEIL_HEIGHT;

        //get adjacent tile
        Tile *adj = mLevels[m_CurrentLevel].getTile(xpos+1, ypos);

        //if adjacent is higher, draw wall
        if( adj->getHeight() > ttile->getHeight()  &&
           ttile->getType() != TILETYPE_D_NW && ttile->getType() != TILETYPE_D_SW)
        {
            topheight = adj->getHeight()-1;

             //if adjacent is sloping and not higher
            if(adj->getType() != TILETYPE_SL_E && topheight != ttile->getHeight())
            {
                Mesh = generateWallMesh(topheight, topheight, ttile->getHeight()-1, ttile->getHeight()-1);

                mysquare = m_SMgr->addMeshSceneNode(Mesh);

                mysquare->setPosition(vector3df( (ypos*scale),0, (xpos*scale)+scale ) );
                mysquare->setRotation(vector3df(0, 90, 0));
                mysquare->setMaterialTexture(0, m_Wall64TXT[ttile->getWallTXT()]);
                mysquare->updateAbsolutePosition();

                meshlist.push_back(mysquare);
                Mesh->drop();
            }
        }
    }
    //west wall
    if(mLevels[m_CurrentLevel].getTile(xpos-1, ypos) != NULL)
    {
        //by default, wall height is top height
        int topheight = CEIL_HEIGHT;

        //get adjacent tile
        Tile *adj = mLevels[m_CurrentLevel].getTile(xpos-1, ypos);

        //if adjacent is higher, draw wall
        if( adj->getHeight() > ttile->getHeight()   &&
           ttile->getType() != TILETYPE_D_NE && ttile->getType() != TILETYPE_D_SE)
        {
            topheight = adj->getHeight()-1;

            //if adjacent is sloping and not higher
            if(adj->getType() != TILETYPE_SL_S && topheight != ttile->getHeight())
            {
                Mesh = generateWallMesh(topheight, topheight, ttile->getHeight()-1, ttile->getHeight()-1);

                mysquare = m_SMgr->addMeshSceneNode(Mesh);

                mysquare->setPosition(vector3df( (ypos*scale)+scale,0, (xpos*scale) ) );
                mysquare->setRotation(vector3df(0, -90, 0));
                mysquare->setMaterialTexture(0, m_Wall64TXT[ttile->getWallTXT()]);
                mysquare->updateAbsolutePosition();

                meshlist.push_back(mysquare);
                Mesh->drop();
            }
        }
    }


    //Mesh->drop();

    //configure all scene meshes
    for(int i = 0; i < int(meshlist.size()); i++) configMeshSceneNode(meshlist[i]);

    return meshlist;
}
