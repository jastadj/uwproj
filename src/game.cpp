#include "game.hpp"
#include "tools.hpp"

#include <GL/glew.h>
#include <GL/glut.h>

Game *Game::mInstance = NULL;

Game::Game()
{
    mScreen = NULL;
}

Game::~Game()
{

}

/////////////////////////////////////////////////////////////////////
//
void Game::start()
{
    std::cout << "Game started.\n";

    initScreen();
    loadlevel();

    mainLoop();
}

void Game::initScreen()
{

    //configure context settings
    mScreenContext.depthBits = 24;
    mScreenContext.stencilBits = 8;
    mScreenContext.antialiasingLevel = 0;
    mScreenContext.majorVersion = 3;
    mScreenContext.minorVersion = 3;

    //create render window
    mScreen = new sf::RenderWindow(sf::VideoMode(640,480,32), "UW Proj", sf::Style::Default, mScreenContext);


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
    bool quit = false;

    //reset view
        glViewport(0,0,640,480); // set viewport
        glMatrixMode(GL_PROJECTION); // select projection matrix
        glLoadIdentity(); // reset projection matrix

        // Calculate The Aspect Ratio Of The Window
        gluPerspective(45.0f,640.f/480.f,0.1f,100.0f);

        glMatrixMode(GL_MODELVIEW);// Select The Modelview Matrix
        //glLoadIdentity();// Reset The Modelview Matrix

    //opengl init
        glShadeModel(GL_SMOOTH);//smooth shading
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);// Black Background

        //glClearDepth(1.0f);                         // Depth Buffer Setup
        glEnable(GL_DEPTH_TEST);                        // Enables Depth Testing
        //glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Test To Do

    float cameraz = 0.0;


    while(!quit)
    {
        sf::Event event;

        //mScreen->clear();

        //handle input
        while(mScreen->pollEvent(event))
        {
            if(event.type == sf::Event::Closed) quit = true;
            else if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Escape) quit = true;
            }
        }

        //clear buffers and reset view
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();                       // Reset The View

        //updates


        //draw

        glTranslatef(-1.5f,0.0f,-6.0f);                 // Move Left 1.5 Units And Into The Screen 6.0
        glBegin(GL_TRIANGLES); // Drawing Using Triangles
            glColor3f(1.0f,0.0f,0.0f);
            glVertex3f( 0.0f, 1.0f, 0.0f);              // Top
            glColor3f(0.0f,1.0f,0.0f);
            glVertex3f(-1.0f,-1.0f, 0.0f);              // Bottom Left
            glColor3f(0.0f,0.0f,1.0f);
            glVertex3f( 1.0f,-1.0f, 0.0f);              // Bottom Right
        glEnd();

        glTranslatef(3.0f,0.0f,0.0f);                   // Move Right 3 Units
        glColor3f(0.5f,0.5f,1.0f);
        glBegin(GL_QUADS);                      // Draw A Quad
                glVertex3f(-1.0f, 1.0f, 0.0f);              // Top Left
                glVertex3f( 1.0f, 1.0f, 0.0f);              // Top Right
                glVertex3f( 1.0f,-1.0f, 0.0f);              // Bottom Right
                glVertex3f(-1.0f,-1.0f, 0.0f);              // Bottom Left
            glEnd();



        //display
        mScreen->display();
    }
}
