#include "game.hpp"
#include "tools.hpp"

Game *Game::mInstance = NULL;

Game::Game()
{
    //init pointers
    m_Device = NULL;
    m_Camera = NULL;
}

Game::~Game()
{
    //destroy rendering device
    m_Device->drop();
}

/////////////////////////////////////////////////////////////////////
//
void Game::start()
{
    std::cout << "Game started.\n";

    loadlevel();

    //init
    initIrrlicht();
    initCamera();

    mainLoop();
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

    return true;
}

bool Game::initCamera()
{
    //already initialized!!
    if(m_Camera != NULL) return false;

    //add camera to scene
    m_Camera = m_SMgr->addCameraSceneNode(0, m_CameraPos, m_CameraTarget);
    m_CameraTarget = vector3df(0,0,-20);
    updateCamera(vector3df(0,0,0));

    //level camera (avoid camera rotation when pointing at target)
    matrix4 m;
    m.setRotationDegrees(m_Camera->getRotation());
    vector3df upv(0.0f, 0.0f, 1.0f);
    m_Camera->setUpVector(upv);

    //temp position target?
    updateCamera(vector3df(0,0,0));

    //capture default FOV
    m_CameraDefaultFOV = m_Camera->getFOV();

    return true;
}

void Game::updateCamera(vector3df cameratargetpos)
{
    m_CameraTarget = cameratargetpos;
    m_CameraPos = vector3df(80,80,200) + m_CameraTarget;
    m_Camera->setPosition(m_CameraPos);
    m_Camera->setTarget(m_CameraTarget);
}

void Game::loadlevel()
{
    //read in level archive
    std::ifstream ifile;
    //const std::string tfile("UWDATA\\lev.ark");
    const std::string tfile("SAVE01\\lev.ark");

    //blocks in level archive (read from header)
    uint16_t blockcount = 0;

    //offset locations for each block
    std::vector<uint64_t> blockoffsets;

    //attempt to open level archive
    ifile.open(tfile.c_str(), std::ios_base::binary);
    if(!ifile.is_open())
    {
        std::cout << "Error opening " << tfile << std::endl;
        return;
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

        blockoffsets.push_back( uint64_t(lsbSum(bobuf, 4)));
        //std::cout << std::dec << "block " << i << ": offset = 0x" << std::hex << blockoffsets.back() << std::endl;
    }

    //read in level data
    // note : UW1 only has 9 levels
    // note : UW1 maps are 64x64 tiles * 4 bytes
    for(int i = 0; i < 9; i++)
    {
        //create level
        mLevels.push_back(Level());

        //jump file position pointer to offset to begin reading in level data
        ifile.seekg(blockoffsets[i]);

        //read in 64 x 64 map tiles
        for(int n = 0; n < TILE_ROWS; n++)
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
                tile->setFloorTXT( getBitVal(tiledata1, 10, 4));

                //set magic illegal flag
                if( getBitVal(tiledata1, 14, 1)) tile->setMagicIllegal(true);
                else tile->setMagicIllegal(false);

                //set has door flag
                if( getBitVal(tiledata1, 15, 1)) tile->setHasDoor(true);
                else tile->setHasDoor(false);


                //last two bytes
                unsigned char ldatabuf2[2];
                readBin(&ifile, ldatabuf2, 2);
                int tiledata2 = lsbSum(ldatabuf2, 2);

                //set wall texture index
                tile->setWallTXT( getBitVal(tiledata2, 0, 6) );

                //set first object in tile
                tile->setFirstObjectIndex( getBitVal(tiledata2, 6, 10) );
            }
        }


    }

    //print level 1 debug
    mLevels[0].printDebug();

    ifile.close();
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

    //test - create an image, and a texture from image


    IImage *myimage = NULL;
    myimage = m_Driver->createImage(ECF_A1R5G5B5, dimension2d<u32>(32,32));

    //set some pixels
    myimage->fill(SColor(255,0,0,0));
    myimage->setPixel(16,16, SColor(255,255,255,255));

    ITexture *mytexture = NULL;
    mytexture = m_Driver->addTexture("mytex", myimage);

    SMesh *squaremesh = getSquareMesh(32,32);

    vector3df squarepos(0, 0, 0);
    IMeshSceneNode *mysquare = m_SMgr->addMeshSceneNode(squaremesh);

        mysquare->setPosition(vector3df(0,0, 0));
        mysquare->setMaterialTexture(0, mytexture);
        mysquare->setMaterialFlag(video::EMF_BACK_FACE_CULLING, true);
        mysquare->setMaterialFlag(video::EMF_LIGHTING, false);
        //mycube->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
        //mycube->setMaterialFlag(video::EMF_BLEND_OPERATION, true);
        //mycube->setMaterialType(video::EMT_PARALLAX_MAP_SOLID);
        mysquare->updateAbsolutePosition();





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
            m_Driver->draw3DLine(vector3df(0,0,0), vector3df(0,0,100), SColor(255,0,0,255));
            m_Driver->draw3DLine(vector3df(0,0,0), vector3df(100,0,0), SColor(0,255,0,255));
            m_Driver->draw3DLine(vector3df(0,0,0), vector3df(0,100,0), SColor(000,255,255,0));
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
        buf->Vertices[0] = S3DVertex(0,0,0, 0,0,1,    video::SColor(255,255,255,255), 0, 0);
        buf->Vertices[1] = S3DVertex(1*width,0,0, 0,0,1,  video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[2] = S3DVertex(0,1*width,0, 0,0,1,    video::SColor(255,255,255,255), 0, 1);
        buf->Vertices[3] = S3DVertex(1*width,0,0, 0,0,1,    video::SColor(255,255,255,255), 1, 0);
        buf->Vertices[4] = S3DVertex(1*width,1*width,0, 0,0,1,    video::SColor(255,255,255,255), 1, 1);
        buf->Vertices[5] = S3DVertex(0,1*width,0, 0,0,1,    video::SColor(255,255,255,255), 0, 1);


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
