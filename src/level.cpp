#include "level.hpp"

#include <iostream>
#include <sstream>

#include "game.hpp"
#include "tools.hpp"
#include "object.hpp"

int loadLevel(std::vector<Level> *levels)
{
    if(levels == NULL) return -1;

    //get game reference
    Game *gptr = NULL;
    gptr = Game::getInstance();

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
        levels->push_back(Level());

        //save texture map data for shits and giggles
        levels->back().mTextureMapping = texturemap[i];

        //set ceiling texture index from texture map (level uses one for whole map)
        levels->back().setCeilingTextureIndex( texturemap[i][txtmapwalls+txtmapfloors-1]);

        //jump file position pointer to offset to begin reading in level data
        ifile.seekg(blockoffsets[i]);

        //read in 64 x 64 map tiles
        //note: uw tiles are flipped on y axis
        for(int n = TILE_ROWS-1; n >=0; n--)
        {
            for(int p = 0; p < TILE_COLS; p++)
            {
                //get tile
                Tile *tile = levels->back().getTile(p, n);

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
            ObjectInstance *newobj = new ObjectInstance( gptr->getObject(objid) );
            //add new object to master object list
            levels->back().addObject(newobj);

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
        std::vector<ObjectInstance*> *objlist = levels->back().getObjectsMaster();

        //add objects to tiles
        for(int n = 0; n < TILE_ROWS; n++)
        {
            for(int p = 0; p < TILE_COLS; p++)
            {

                //for debug purposes, only process one tile

                //get tile
                Tile *ttile = levels->back().getTile(p, n);

                //get first object index from tile
                int objindex = ttile->getFirstObjectIndex();

                //note : object 0 means empty, no objects on tile
                //add each linked object to tile object list
                while(objindex != 0 && i == gptr->getCurrentLevel()) // for now, only process current level
                {

                        //retrieve object from master list
                        ObjectInstance *tobj = (*objlist)[objindex];

                        //add object to tile objects list
                        ttile->addObject(tobj);

                        //update tile's object
                        gptr->updateObject(tobj, ttile);

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


Level::Level()
{
    mTiles.resize(TILE_ROWS);
    for(int i = 0; i < TILE_COLS; i++)
        for(int n = 0; n < TILE_COLS; n++) mTiles[i].push_back(Tile(n, i));
}

Level::~Level()
{

}

Tile *Level::getTile(int x, int y)
{
    //is tile valid?
    if(x < 0 || x >= TILE_COLS) return NULL;
    if(y < 0 || y >= TILE_ROWS) return NULL;

    return &mTiles[y][x];
}

std::vector<Tile*> Level::getAdjacentTilesAt(int x, int y)
{
    std::vector<Tile*> adjacents;

    adjacents.push_back( getTile(x, y-1));
    adjacents.push_back( getTile(x+1, y));
    adjacents.push_back( getTile(x, y+1));
    adjacents.push_back( getTile(x-1, y));

    return adjacents;
}

bool Level::addObject(ObjectInstance *nobj)
{
    if(nobj == NULL) return false;

    m_ObjectsMaster.push_back(nobj);

    return true;
}

// high level level generation, call each tile to build its geometry
bool Level::buildLevelGeometry()
{
    for(int i = 0; i < TILE_ROWS; i++)
    {
        for(int n = 0; n < TILE_COLS; n++)
        {
            if(!buildTileGeometry(n, i))
            {
                std::cout << "Error building tile geometry for " << n << "," << i << std::endl;
                return false;
            }
        }
    }

    return true;
}

// this will build all the required meshes needed for given tile
// includes translating and rotating necessary geometry for tile
bool Level::buildTileGeometry(int x, int y)
{
    //get target tile at x,y coordinate
    Tile *ttile = getTile(x,y);
    int ttype = 0;

    //temp top and bottom height calculations, clockwise from top left corner
    std::vector<int> theight_ns(4,0);
    std::vector<int> bheight_ns(4,0);
    std::vector<int> theight_ew(4,0);
    std::vector<int> bheight_ew(4,0);

    //container for nodes
    std::vector<IMeshSceneNode*> scenenodelist;

    //create strings to identify nodes for walls, ceil, floor
    std::stringstream tilenamess;
    tilenamess << "TILE_" << y*TILE_ROWS + x;
    std::string wallname = tilenamess.str() + "_W";
    std::string ceilname = tilenamess.str() + "_C";
    std::string floorname = tilenamess.str() + "_F";



    //adjacent tiles
    Tile *tilenorth = NULL;
    Tile *tilesouth = NULL;
    Tile *tilewest = NULL;
    Tile *tileeast = NULL;

    //get external resources
    Game *gptr = NULL;
    gptr = Game::getInstance();
    ISceneManager *m_SMgr = gptr->getSceneManager();
    const std::vector<ITexture*> *w64txt = gptr->getWall64Textures();
    const std::vector<ITexture*> *f32txt = gptr->getFloor32Textures();

    //valid tile?
    if(ttile == NULL) return false;

    //get type
    ttype = ttile->getType();

    //ignore geometry for solid tiles
    if(ttype == TILETYPE_SOLID) return true;

    //clear tile geometry
    ttile->clearGeometry();

    //get adjacent tiles (need to calculate adjacent wall heights)
    tilenorth = getTile(x, y-1);
    tilesouth = getTile(x, y+1);
    tilewest = getTile(x-1, y);
    tileeast = getTile(x+1, y);

    //default tile heights to height value and ceiling
    for(int i = 0; i < 4; i++)
    {
        //north / south mapping
        bheight_ns[i] = ttile->getHeight();
        theight_ns[i] = CEIL_HEIGHT+1;

        //east / west mapping
        bheight_ew[i] = ttile->getHeight();
        theight_ew[i] = CEIL_HEIGHT+1;
    }

    /////////////////////////////////
    //  HEIGHT CALCULATIONS

    //floor
    switch( ttype)
    {
    //adjust bottom coordinate of floor slope, floor only drops by 1/4 of a standard 4 unit wall height (1 unit)
    case TILETYPE_SL_S:
        bheight_ns[SW] = ttile->getHeight()+(UNIT_SCALE/4);
        bheight_ns[SE] = ttile->getHeight()+(UNIT_SCALE/4);
        bheight_ew[SW] = ttile->getHeight()+(UNIT_SCALE/4);
        bheight_ew[SE] = ttile->getHeight()+(UNIT_SCALE/4);
        break;
    case TILETYPE_SL_N:
        bheight_ns[NW] = ttile->getHeight()+(UNIT_SCALE/4);
        bheight_ns[NE] = ttile->getHeight()+(UNIT_SCALE/4);
        bheight_ew[NW] = ttile->getHeight()+(UNIT_SCALE/4);
        bheight_ew[NE] = ttile->getHeight()+(UNIT_SCALE/4);
        break;
    case TILETYPE_SL_W:
        bheight_ns[NW] = ttile->getHeight()+(UNIT_SCALE/4);
        bheight_ns[SW] = ttile->getHeight()+(UNIT_SCALE/4);
        bheight_ew[NW] = ttile->getHeight()+(UNIT_SCALE/4);
        bheight_ew[SW] = ttile->getHeight()+(UNIT_SCALE/4);
        break;
    case TILETYPE_SL_E:
        bheight_ns[NE] = ttile->getHeight()+(UNIT_SCALE/4);
        bheight_ns[SE] = ttile->getHeight()+(UNIT_SCALE/4);
        bheight_ew[NE] = ttile->getHeight()+(UNIT_SCALE/4);
        bheight_ew[SE] = ttile->getHeight()+(UNIT_SCALE/4);
        break;
    default:
        break;
    }

    //if ceiling is lower than floor, make them match
    for(int i = 0; i < 4; i++)
    {
        if(theight_ns[i] < bheight_ns[i]) theight_ns[i] = bheight_ns[i];

        if(theight_ew[i] < bheight_ew[i]) theight_ew[i] = bheight_ew[i];
    }

    //ceiling
    //north
    if(tilenorth != NULL)
    {
        int adjtype = tilenorth->getType();
        int adjheight = tilenorth->getHeight();

        //if adjacent tile is not solid
        if(adjtype != TILETYPE_SOLID)
        {

            //if adjacent is blocked diagonal
            if(adjtype == TILETYPE_D_NW || adjtype == TILETYPE_D_NE)
            {
                theight_ns[NW] = CEIL_HEIGHT+1;
                theight_ns[NE] = CEIL_HEIGHT+1;
            }
            //if adjacent tile is lower in height than current tile
            else if(adjheight <= ttile->getHeight())
            {
                //squish ceiling heights to match floor heights
                theight_ns[NW] = bheight_ns[NW];
                theight_ns[NE] = bheight_ns[NE];
            }
            //else bring the celing down to match adjacent height
            else
            {
                theight_ns[NW] = adjheight;
                theight_ns[NE] = adjheight;

                //if adjacent is sloping east
                if(adjtype == TILETYPE_SL_E)
                {
                    theight_ns[NE] += UNIT_SCALE/4;
                }
                //else if adjacent is sloping west
                else if(adjtype == TILETYPE_SL_W)
                {
                    theight_ns[NW] += UNIT_SCALE/4;
                }
                //else heights match but current tile is sloping
                else if(adjheight > ttile->getHeight() &&
                    (ttype == TILETYPE_SL_E || ttype == TILETYPE_SL_W))
                {
                    theight_ns[NW] = adjheight;
                    theight_ns[NE] = adjheight;
                }
            }
        }
    }
    //south
    if(tilesouth != NULL)
    {
        int adjtype = tilesouth->getType();
        int adjheight = tilesouth->getHeight();

        //if adjacent tile is not solid
        if(adjtype != TILETYPE_SOLID)
        {
            //if adjacent is blocked diagonal
            if(adjtype == TILETYPE_D_SW || adjtype == TILETYPE_D_SE)
            {
                theight_ns[NW] = CEIL_HEIGHT+1;
                theight_ns[NE] = CEIL_HEIGHT+1;
            }
            //if adjacent tile is lower in height than current tile
            else if(adjheight <= ttile->getHeight())
            {
                //squish ceiling heights to match floor heights
                theight_ns[SW] = bheight_ns[SW];
                theight_ns[SE] = bheight_ns[SE];
            }
            //else bring the celing down to match adjacent height
            else
            {
                theight_ns[SW] = adjheight;
                theight_ns[SE] = adjheight;

                //if adjacent is sloping east
                if(adjtype == TILETYPE_SL_E)
                {
                    theight_ns[SE] += UNIT_SCALE/4;
                }
                //else if adjacent is sloping west
                else if(adjtype == TILETYPE_SL_W)
                {
                    theight_ns[SW] += UNIT_SCALE/4;
                }
                //else heights match but current tile is sloping
                else if(adjheight > ttile->getHeight() &&
                    (ttype == TILETYPE_SL_E || ttype == TILETYPE_SL_W))
                {
                    theight_ns[SW] = adjheight;
                    theight_ns[SE] = adjheight;
                }

            }
        }
    }
    //west
    if(tilewest != NULL)
    {
        int adjtype = tilewest->getType();
        int adjheight = tilewest->getHeight();

        //if adjacent tile is not solid
        if(adjtype != TILETYPE_SOLID)
        {
            //if adjacent is blocked diagonal
            if(adjtype == TILETYPE_D_NW || adjtype == TILETYPE_D_SW)
            {
                theight_ew[NW] = CEIL_HEIGHT+1;
                theight_ew[SW] = CEIL_HEIGHT+1;
            }
            //if adjacent tile is lower in height than current tile
            else if(adjheight <= ttile->getHeight())
            {
                //squish ceiling heights to match floor heights
                theight_ew[SW] = bheight_ew[SW];
                theight_ew[NW] = bheight_ew[NW];
            }
            //else bring the celing down to match adjacent height
            else
            {
                theight_ew[SW] = adjheight;
                theight_ew[NW] = adjheight;

                //if adjacent is sloping east
                if(adjtype == TILETYPE_SL_N)
                {
                    theight_ew[NW] += UNIT_SCALE/4;
                }
                //else if adjacent is sloping west
                else if(adjtype == TILETYPE_SL_S)
                {
                    theight_ew[SW] += UNIT_SCALE/4;
                }
                //else heights match but current tile is sloping
                else if(adjheight > ttile->getHeight() &&
                    (ttype == TILETYPE_SL_N || ttype == TILETYPE_SL_S))
                {
                    theight_ew[SW] = adjheight;
                    theight_ew[NW] = adjheight;
                }

            }
        }
    }
    //east
    if(tileeast != NULL)
    {
        int adjtype = tileeast->getType();
        int adjheight = tileeast->getHeight();

        //if adjacent tile is not solid
        if(adjtype != TILETYPE_SOLID)
        {
            //if adjacent is blocked diagonal
            if(adjtype == TILETYPE_D_NE || adjtype == TILETYPE_D_SE)
            {
                theight_ew[NE] = CEIL_HEIGHT+1;
                theight_ew[SE] = CEIL_HEIGHT+1;
            }
            //if adjacent tile is lower in height than current tile
            else if(adjheight <= ttile->getHeight())
            {
                //squish ceiling heights to match floor heights
                theight_ew[SE] = bheight_ew[SE];
                theight_ew[NE] = bheight_ew[NE];
            }
            //else bring the celing down to match adjacent height
            else
            {
                theight_ew[SE] = adjheight;
                theight_ew[NE] = adjheight;

                //if adjacent is sloping east
                if(adjtype == TILETYPE_SL_N)
                {
                    theight_ew[NE] += UNIT_SCALE/4;
                }
                //else if adjacent is sloping west
                else if(adjtype == TILETYPE_SL_S)
                {
                    theight_ew[SE] += UNIT_SCALE/4;
                }
                //else heights match but current tile is sloping
                else if(adjheight > ttile->getHeight() &&
                    (ttype == TILETYPE_SL_N || ttype == TILETYPE_SL_S))
                {
                    theight_ew[SE] = adjheight;
                    theight_ew[NE] = adjheight;
                }

            }
        }
    }

    /////////////////////////////////
    //  MESH GENERATION

    //generate floor mesh
    SMesh *floormesh = NULL;

    // if diagonal type, generate alternate floor (triangle)
    if(ttype >=2 && ttype <= 5) floormesh = generateFloorMesh(bheight_ns[0], bheight_ns[1], bheight_ns[2]);
    // else generate a full floor
    else floormesh = generateFloorMesh(bheight_ns[0], bheight_ns[1], bheight_ns[2], bheight_ns[3]);


    //create floor node
    if(floormesh != NULL)
    {
        //create mesh in scene
        IMeshSceneNode *tnode = NULL;
        if(USE_OCTREE) tnode = m_SMgr->addOctreeSceneNode(floormesh);
        else tnode = m_SMgr->addMeshSceneNode(floormesh);

        //orient mesh depending on type
        switch(ttype)
        {
        case TILETYPE_D_NE:
            tnode->setRotation(vector3df(0, 90, 0));
            tnode->setPosition( vector3df( y*UNIT_SCALE,0, (x*UNIT_SCALE)+UNIT_SCALE) );
            break;
        case TILETYPE_D_NW:
            tnode->setRotation(vector3df(0, 0, 0));
            tnode->setPosition( vector3df( y*UNIT_SCALE,0, (x*UNIT_SCALE) ) );
            break;
        case TILETYPE_D_SE:
            tnode->setRotation(vector3df(0, 180, 0));
            tnode->setPosition( vector3df( y*UNIT_SCALE+UNIT_SCALE,0, (x*UNIT_SCALE)+UNIT_SCALE ) );
            break;
        case TILETYPE_D_SW:
            tnode->setRotation(vector3df(0, -90, 0));
            tnode->setPosition( vector3df( y*UNIT_SCALE+UNIT_SCALE,0, (x*UNIT_SCALE) ) );
            break;
        default:
            tnode->setPosition( vector3df( y*UNIT_SCALE,0, (x*UNIT_SCALE)) );
            break;
        }

        //set floor texture and abs position
        tnode->setMaterialTexture(0, (*f32txt)[ttile->getFloorTXT()]);

        tnode->setName(floorname.c_str());

        //add mesh and node to list
        scenenodelist.push_back(tnode);

        //drop mesh
        floormesh->drop();

    }

    //ceiling mesh generation
    SMesh *ceilmesh = NULL;
        //generate mesh
        ceilmesh = generateFloorMesh(0,0,0,0);

        //create mesh in scene
        IMeshSceneNode *cnode = NULL;
        if(USE_OCTREE) cnode = m_SMgr->addOctreeSceneNode(ceilmesh);
        else cnode = m_SMgr->addMeshSceneNode(ceilmesh);

        cnode->setName(ceilname.c_str());

        //rotate ceiling to face down and position ceiling to top of level height
        cnode->setRotation(vector3df(0,0,180));
        cnode->setPosition(vector3df(y*UNIT_SCALE+UNIT_SCALE, CEIL_HEIGHT+1, x*UNIT_SCALE));
        cnode->setMaterialTexture(0, (*f32txt)[m_CeilingTextureIndex]); // note, ceiling is always 10th floor texture?

        //add node to tile node list
        scenenodelist.push_back(cnode);
    //drop ceiling mesh
    ceilmesh->drop();

    //wall mesh generation
    //diagonal walls
    if(ttype >= 2 && ttype <= 5)
    {
        //create a diagonal wall mesh
        SMesh *dwallmesh = NULL;

        if(ttype == TILETYPE_D_SE || ttype == TILETYPE_D_SW)
            dwallmesh = generateDiagonalWallMesh(theight_ns[NW], theight_ns[NE], bheight_ns[NE], bheight_ns[NW]);
        else dwallmesh = generateDiagonalWallMesh(theight_ns[SW], theight_ns[SE], bheight_ns[SE], bheight_ns[SW]);


        if(dwallmesh != NULL)
        {
            //create mesh in scene
            IMeshSceneNode *tnode = NULL;
            if(USE_OCTREE) tnode = m_SMgr->addOctreeSceneNode(dwallmesh);
            else tnode = m_SMgr->addMeshSceneNode(dwallmesh);

            //orient scene node
            switch(ttype)
            {
            case TILETYPE_D_SE:
                tnode->setPosition( vector3df( y*UNIT_SCALE + UNIT_SCALE,0, x*UNIT_SCALE ) );
                tnode->setRotation( vector3df(0,-90,0) );
                break;
            case TILETYPE_D_NE:
                tnode->setPosition( vector3df( y*UNIT_SCALE+UNIT_SCALE,0, x*UNIT_SCALE+UNIT_SCALE) );
                tnode->setRotation( vector3df(0,180,0) );
                break;
            case TILETYPE_D_NW:
                tnode->setPosition( vector3df( y*UNIT_SCALE,0, x*UNIT_SCALE+UNIT_SCALE) );
                tnode->setRotation( vector3df(0,90,0) );
                break;
            case TILETYPE_D_SW:
            default:
                tnode->setPosition( vector3df( y*UNIT_SCALE,0, x*UNIT_SCALE ) );
                tnode->setRotation( vector3df(0,0,0) );
                break;
            }

            //set wall texture (common for all walls of tile)
            tnode->setMaterialTexture(0, (*w64txt)[ttile->getWallTXT()]);

            tnode->setName(wallname.c_str());

            //add mesh and node to list
            scenenodelist.push_back(tnode);

            //drop mesh
            dwallmesh->drop();
        }

    }

    //north wall
    if(ttype != TILETYPE_D_SE && ttype != TILETYPE_D_SW)
    {
            //generate wall mesh
            SMesh *wallmesh = generateWallMesh( theight_ns[NW], theight_ns[NE], bheight_ns[NE], bheight_ns[NW]);

            //if a valid wall mesh was generated
            if(wallmesh != NULL)
            {
                //create mesh in scene
                IMeshSceneNode *tnode = NULL;
                if(USE_OCTREE) tnode = m_SMgr->addOctreeSceneNode(wallmesh);
                else tnode = m_SMgr->addMeshSceneNode(wallmesh);

                tnode->setName(wallname.c_str());

                //orient scene node
                tnode->setPosition( vector3df( y*UNIT_SCALE,0, x*UNIT_SCALE ) );
                tnode->setRotation( vector3df(0,0,0) );

                //set wall texture (common for all walls of tile)
                tnode->setMaterialTexture(0, (*w64txt)[ttile->getWallTXT()]);

                //add mesh and node to list
                scenenodelist.push_back(tnode);

                //drop mesh
                wallmesh->drop();
            }
    }
    //south wall
    if(ttype != TILETYPE_D_NE && ttype != TILETYPE_D_NW)
    {
            //generate wall mesh
            SMesh *wallmesh = generateWallMesh( theight_ns[SE], theight_ns[SW], bheight_ns[SW], bheight_ns[SE]);

            //if a valid wall mesh was generated
            if(wallmesh != NULL)
            {
                //create mesh in scene
                IMeshSceneNode *tnode = NULL;
                if(USE_OCTREE) tnode = m_SMgr->addOctreeSceneNode(wallmesh);
                else tnode = m_SMgr->addMeshSceneNode(wallmesh);

                tnode->setName(wallname.c_str());

                //orient scene node
                tnode->setPosition( vector3df( y*UNIT_SCALE+UNIT_SCALE,0, x*UNIT_SCALE+UNIT_SCALE ) );
                tnode->setRotation( vector3df(0,180,0) );

                //set wall texture (common for all walls of tile)
                tnode->setMaterialTexture(0, (*w64txt)[ttile->getWallTXT()]);

                //add mesh and node to list
                scenenodelist.push_back(tnode);

                //drop mesh
                wallmesh->drop();
            }
    }
    //west wall
    if(ttype != TILETYPE_D_SE && ttype != TILETYPE_D_NE)
    {
            //generate wall mesh
            SMesh *wallmesh = generateWallMesh( theight_ew[SW], theight_ew[NW], bheight_ew[NW], bheight_ew[SW]);

            //if a valid wall mesh was generated
            if(wallmesh != NULL)
            {
                //create mesh in scene
                IMeshSceneNode *tnode = NULL;
                if(USE_OCTREE) tnode = m_SMgr->addOctreeSceneNode(wallmesh);
                else tnode = m_SMgr->addMeshSceneNode(wallmesh);

                tnode->setName(wallname.c_str());

                //orient scene node
                tnode->setPosition( vector3df( y*UNIT_SCALE+UNIT_SCALE,0, x*UNIT_SCALE ) );
                tnode->setRotation( vector3df(0,-90,0) );

                //set wall texture (common for all walls of tile)
                tnode->setMaterialTexture(0, (*w64txt)[ttile->getWallTXT()]);

                //add mesh and node to list
                scenenodelist.push_back(tnode);

                //drop mesh
                wallmesh->drop();
            }
    }
    //east wall
    if(ttype != TILETYPE_D_SW && ttype != TILETYPE_D_NW)
    {
            //generate wall mesh
            SMesh *wallmesh = generateWallMesh( theight_ew[NE], theight_ew[SE], bheight_ew[SE], bheight_ew[NE]);

            //if a valid wall mesh was generated
            if(wallmesh != NULL)
            {
                //create mesh in scene
                IMeshSceneNode *tnode = NULL;
                if(USE_OCTREE) tnode = m_SMgr->addOctreeSceneNode(wallmesh);
                else tnode = m_SMgr->addMeshSceneNode(wallmesh);

                tnode->setName(wallname.c_str());

                //orient scene node
                tnode->setPosition( vector3df( y*UNIT_SCALE,0, x*UNIT_SCALE+UNIT_SCALE ) );
                tnode->setRotation( vector3df(0,90,0) );

                //set wall texture (common for all walls of tile)
                tnode->setMaterialTexture(0, (*w64txt)[ttile->getWallTXT()]);

                //add mesh and node to list
                scenenodelist.push_back(tnode);

                //drop mesh
                wallmesh->drop();
            }
    }


    //perform common operations to all meshes\scene nodes
    for(int i = 0; i < int(scenenodelist.size()); i++)
    {
        //select target mesh and scene node
        IMeshSceneNode *tnode = scenenodelist[i];

        //update scene node with common flags
        gptr->configMeshSceneNode(tnode);

        //add scene node reference to tile
        ttile->addMesh(tnode);

    }


    return true;
}

// create a simple square floor using heights of upper left, upper right, bottom right, and bottom left
SMesh *Level::generateFloorMesh(int ul, int ur, int br, int bl)
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

    //determine highest and lowest point
    std::vector<int> points;
    int highestpoint = 0;
    int lowestpoint = 0;
    points.push_back(ur);
    points.push_back(br);
    points.push_back(ul);
    points.push_back(bl);

    for(int i = 0; i < int(points.size()); i++)
    {
        if(points[i] < lowestpoint) lowestpoint = points[i];
        if(points[i] > highestpoint) highestpoint = points[i];
    }

    mesh->setBoundingBox( aabbox3df(0,lowestpoint*scale,0,1*UNIT_SCALE,highestpoint*scale,1*UNIT_SCALE));

    //buf->recalculateBoundingBox();

    return mesh;

}

// alternate floor mesh generator that generate half of a floor tile (triangle)
// used for diagonal walls.  Note, this floor does require rotation and translation post generation
// since im too retarded to do matrix math
SMesh *Level::generateFloorMesh(int p1, int p2, int p3)
{

    SMesh *mesh = NULL;
    SMeshBuffer *buf = NULL;

    int vcount = 3;
    int scale = UNIT_SCALE/4;

    //FLOOR MESH
    mesh = new SMesh();
    buf = new SMeshBuffer();

    mesh->addMeshBuffer(buf);
    buf->drop();

    buf->Vertices.reallocate(vcount);
    buf->Vertices.set_used(vcount);

    //triangle 1
    buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,p1*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 0); //TL
    buf->Vertices[1] = S3DVertex(0*UNIT_SCALE,p2*scale,1*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 1, 0); //TR
    buf->Vertices[2] = S3DVertex(1*UNIT_SCALE,p3*scale,0*UNIT_SCALE, 0,1,0,    video::SColor(255,255,255,255), 0, 1);// BL

    //finalize vertices
    buf->Indices.reallocate(vcount);
    buf->Indices.set_used(vcount);
    for(int i = 0; i < vcount; i++) buf->Indices[i] = i;

    //determine highest and lowest point
    std::vector<int> points;
    int highestpoint = 0;
    int lowestpoint = 0;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);

    for(int i = 0; i < int(points.size()); i++)
    {
        if(points[i] < lowestpoint) lowestpoint = points[i];
        if(points[i] > highestpoint) highestpoint = points[i];
    }

    mesh->setBoundingBox( aabbox3df(0,lowestpoint*scale,0,1*UNIT_SCALE,highestpoint*scale,1*UNIT_SCALE));

    return mesh;

}

SMesh *Level::generateWallMesh(int tl, int tr, int br, int bl)
{
    //if top and bottoms match, return null
    if(tl == bl && tr == br) return NULL;

    SMesh *mesh = NULL;
    SMeshBuffer *buf = NULL;

    int scale = UNIT_SCALE/4;
    int vcount = 6;

    //WALL MESH
    mesh = new SMesh();
    buf = new SMeshBuffer();

    mesh->addMeshBuffer(buf);
    buf->drop();

    buf->Vertices.reallocate(vcount);
    buf->Vertices.set_used(vcount);

    //calc texture y scaling for stretching to properly map texture
    float txtscaley = 1;
    float txtscaleytop = tl;
    float txtscaleybot = bl;

    if(tr > tl) txtscaleytop = tr;
    if(br < bl) txtscaleybot = br;
    txtscaley = (txtscaleytop - txtscaleybot) / UNIT_SCALE;

    //triangle 1
    buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,tl*scale,0*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 0, 0); //TL
    buf->Vertices[1] = S3DVertex(0*UNIT_SCALE,tr*scale,1*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 1, 0); //TR
    buf->Vertices[2] = S3DVertex(0*UNIT_SCALE,bl*scale,0*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 0, txtscaley);// BL
    //triangle 2
    buf->Vertices[3] = S3DVertex(0*UNIT_SCALE,tr*scale,1*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 1, 0); //TR
    buf->Vertices[4] = S3DVertex(0*UNIT_SCALE,br*scale,1*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 1, txtscaley ); //BR
    buf->Vertices[5] = S3DVertex(0*UNIT_SCALE,bl*scale,0*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 0, txtscaley ); // BL

    int lowestpoint = bl;
    int highestpoint = tl;
    if(br < lowestpoint) lowestpoint = br;
    if(tr > highestpoint) highestpoint = tr;

    //finalize vertices
    buf->Indices.reallocate(vcount);
    buf->Indices.set_used(vcount);
    for(int i = 0; i < vcount; i++) buf->Indices[i] = i;
    mesh->setBoundingBox( aabbox3df(0,lowestpoint*scale,0,0,highestpoint*scale,1*UNIT_SCALE));
    //buf->recalculateBoundingBox();

    return mesh;
}

SMesh *Level::generateDiagonalWallMesh(int tl, int tr, int br, int bl)
{
    //if top and bottoms match, return null
    if(tl == bl && tr == br) return NULL;

    SMesh *mesh = NULL;
    SMeshBuffer *buf = NULL;

    int scale = UNIT_SCALE/4;
    int vcount = 6;

    //WALL MESH
    mesh = new SMesh();
    buf = new SMeshBuffer();

    mesh->addMeshBuffer(buf);
    buf->drop();

    buf->Vertices.reallocate(vcount);
    buf->Vertices.set_used(vcount);

    //calc texture y scaling for stretching to properly map texture
    float txtscaley = 1;
    float txtscaleytop = tl;
    float txtscaleybot = bl;

    if(tr > tl) txtscaleytop = tr;
    if(br < bl) txtscaleybot = br;
    txtscaley = (txtscaleytop - txtscaleybot) / UNIT_SCALE;

    //triangle 1
    buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,tl*scale,0*UNIT_SCALE, 1,0,-1,    video::SColor(255,255,255,255), 0, 0); //TL
    buf->Vertices[1] = S3DVertex(1*UNIT_SCALE,tr*scale,1*UNIT_SCALE, 1,0,-1,    video::SColor(255,255,255,255), 1, 0); //TR
    buf->Vertices[2] = S3DVertex(0*UNIT_SCALE,bl*scale,0*UNIT_SCALE, 1,0,-1,    video::SColor(255,255,255,255), 0, txtscaley);// BL
    //triangle 2
    buf->Vertices[3] = S3DVertex(1*UNIT_SCALE,tr*scale,1*UNIT_SCALE, 1,0,-1,    video::SColor(255,255,255,255), 1, 0); //TR
    buf->Vertices[4] = S3DVertex(1*UNIT_SCALE,br*scale,1*UNIT_SCALE, 1,0,-1,    video::SColor(255,255,255,255), 1, txtscaley); //BR
    buf->Vertices[5] = S3DVertex(0*UNIT_SCALE,bl*scale,0*UNIT_SCALE, 1,0,-1,    video::SColor(255,255,255,255), 0, txtscaley); // BL

    //finalize vertices
    buf->Indices.reallocate(vcount);
    buf->Indices.set_used(vcount);
    for(int i = 0; i < vcount; i++) buf->Indices[i] = i;
    mesh->setBoundingBox( aabbox3df(0,bl*scale,0,1*UNIT_SCALE,tr*scale,1*UNIT_SCALE));
    //buf->recalculateBoundingBox();

    return mesh;
}

std::vector<IMeshSceneNode*> Level::getMeshes()
{
    std::vector<IMeshSceneNode*> meshes;

    for(int i = 0; i < int(mTiles.size()); i++)
    {
        for(int n = 0; n < int(mTiles[i].size()); n++)
        {
            std::vector<IMeshSceneNode*> tmeshes = mTiles[i][n].getMeshes();

            for(int p = 0; p < int(tmeshes.size()); p++)
            {
                meshes.push_back( tmeshes[p]);
            }
        }
    }

    return meshes;
}

void Level::printDebug()
{

    //print a test part of the first level
    int posx = 29;
    int posy = 2;

    //flip map's y axis for drawing with 0,0 being top left
    for(int i = 0; i < TILE_ROWS; i++)
    {
        std::cout << "\n";
        for(int n = 0; n < TILE_COLS; n++)
        {
            Tile *tile = getTile(n, i);
            char tchar = 0;

            switch(tile->getType())
            {
            case 0:
                tchar = '#';
                break;
            case 1:
            case 6:
            case 7:
            case 8:
            case 9:
                tchar = '.';
                break;
            case 2:
            case 5:
                tchar = '/';
                break;
            case 3:
            case 4:
                tchar = '\\';
                break;
            default:
                tchar = '?';
                break;
            }

            if(tile->hasDoor()) tchar = 'D';
            else if(posx == n && posy == i) tchar = 'X';

            std::cout << tchar;
        }
    }

    std::cout << std::endl;
}
/////////////////////////////////////////////////////////////////////
//  TILE
Tile::Tile(int xpos, int ypos)
{

    mPosition.X = xpos;
    mPosition.Y = ypos;

    mType = TILETYPE_SOLID;
    mHeight = 0;
    mFloorTXTIndex = 0;
    mWallTXTIndex = 0;
    mFirstObjectIndex = 0;

    mMagicIllegal = false;
    mHasDoor = false;
    mUnk1 = false;
    mUnk2 = false;


}

Tile::~Tile()
{
    clearGeometry();
}

bool Tile::addObject(ObjectInstance *tobj)
{
    if(tobj == NULL) return false;

    mObjects.push_back(tobj);
    return true;
}

int Tile::clearGeometry()
{
    int meshcount = int(mMeshes.size());
    for(int i = 0; i < meshcount; i++)
    {
        mMeshes[i]->drop();
    }
    mMeshes.clear();

    return meshcount;
}

bool Tile::addMesh(IMeshSceneNode *tmesh)
{
    if(tmesh == NULL) return false;

    mMeshes.push_back(tmesh);
    return true;
}

std::vector<IMeshSceneNode*> Tile::getMeshes()
{
    return mMeshes;
}

void Tile::printDebug()
{

    std::cout << "\nTILE INFO:\n";
    std::cout << "POSITION : " << mPosition.X << "," << mPosition.Y << std::endl;
    std::cout << "TYPE : ";
    switch(getType())
    {
    case TILETYPE_D_NE:
        std::cout << "Diagonal - open to the NE\n";
        break;
    case TILETYPE_D_SE:
        std::cout << "Diagonal - open to the SE\n";
        break;
    case TILETYPE_D_NW:
        std::cout << "Diagonal - open to the NW\n";
        break;
    case TILETYPE_D_SW:
        std::cout << "Diagonal - open to the SW\n";
        break;
    case TILETYPE_SL_E:
        std::cout << "Sloping up to the East\n";
        break;
    case TILETYPE_SL_W:
        std::cout << "Sloping up to the West\n";
        break;
    case TILETYPE_SL_N:
        std::cout << "Sloping up to the North\n";
        break;
    case TILETYPE_SL_S:
        std::cout << "Sloping up to the South\n";
        break;
    case TILETYPE_OPEN:
        std::cout << "Open\n";
        break;
    case TILETYPE_SOLID:
        std::cout << "Solid\n";
        break;
    default:
        std::cout << "ERROR - UNK\n";
        break;
    }

    std::cout << "HEIGHT : " << getHeight() << std::endl;
    std::cout << "FLOORTXT : " << getFloorTXT() << std::endl;
    std::cout << "WALLTXT  : " << getWallTXT() << std::endl;
    std::cout << "HAS DOOR : " << hasDoor() << std::endl;
    std::cout << "MAGIC ILLEGAL : " << isMagicIllegal() << std::endl;
    std::cout << "UNK1 = " << getUnk1() << std::endl;
    std::cout << "UNK2 = " << getUnk2() << std::endl;
    std::cout << "\nMESHES : " << mMeshes.size() << std::endl;
    std::cout << "FIRST OBJ INDEX : " << std::hex << getFirstObjectIndex() << std::dec << std::endl;
    std::cout << "OBJECTS : " << mObjects.size() << std::endl;
    for(int i = 0; i < int(mObjects.size()); i++)
    {
        std::cout << "     " << i << ":" << mObjects[i]->getDescription() << std::endl;
        //debug
        mObjects[i]->printDebug();
    }

}
