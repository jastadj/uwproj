#include "level.hpp"

#include <iostream>

#include "game.hpp"

Level::Level()
{
    mTiles.resize(TILE_ROWS);
    for(int i = 0; i < TILE_COLS; i++) mTiles[i].resize(TILE_COLS);
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

    //create floor scene node
    if(floormesh != NULL)
    {
        //create mesh in scene
        IMeshSceneNode *tnode = m_SMgr->addMeshSceneNode(floormesh);

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
        tnode->updateAbsolutePosition();

        //update mesh with common flags
        gptr->configMeshSceneNode(tnode);

        //link mesh to tile
        ttile->setFloorMesh(tnode);
        //drop smesh
        floormesh->drop();
    }

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
            //create scene node
            IMeshSceneNode *tnode = m_SMgr->addMeshSceneNode(dwallmesh);

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

            //note : this is common and needs to be done for all meshes, should be able to do a loop for all tile nodes?
            tnode->updateAbsolutePosition();

            //update mesh with common flags
            gptr->configMeshSceneNode(tnode);

            //link mesh to tile
            ttile->addWallMesh(tnode);
            //drop smesh
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
                //create scene node
                IMeshSceneNode *tnode = m_SMgr->addMeshSceneNode(wallmesh);

                //orient scene node
                tnode->setPosition( vector3df( y*UNIT_SCALE,0, x*UNIT_SCALE ) );
                tnode->setRotation( vector3df(0,0,0) );

                //set wall texture (common for all walls of tile)
                tnode->setMaterialTexture(0, (*w64txt)[ttile->getWallTXT()]);

                //note : this is common and needs to be done for all meshes, should be able to do a loop for all tile nodes?
                tnode->updateAbsolutePosition();

                //update mesh with common flags
                gptr->configMeshSceneNode(tnode);

                //link mesh to tile
                ttile->addWallMesh(tnode);
                //drop smesh
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
                //create scene node
                IMeshSceneNode *tnode = m_SMgr->addMeshSceneNode(wallmesh);

                //orient scene node
                tnode->setPosition( vector3df( y*UNIT_SCALE+UNIT_SCALE,0, x*UNIT_SCALE+UNIT_SCALE ) );
                tnode->setRotation( vector3df(0,180,0) );

                //set wall texture (common for all walls of tile)
                tnode->setMaterialTexture(0, (*w64txt)[ttile->getWallTXT()]);

                //note : this is common and needs to be done for all meshes, should be able to do a loop for all tile nodes?
                tnode->updateAbsolutePosition();

                //update mesh with common flags
                gptr->configMeshSceneNode(tnode);

                //link mesh to tile
                ttile->addWallMesh(tnode);
                //drop smesh
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
                //create scene node
                IMeshSceneNode *tnode = m_SMgr->addMeshSceneNode(wallmesh);

                //orient scene node
                tnode->setPosition( vector3df( y*UNIT_SCALE+UNIT_SCALE,0, x*UNIT_SCALE ) );
                tnode->setRotation( vector3df(0,-90,0) );

                //set wall texture (common for all walls of tile)
                tnode->setMaterialTexture(0, (*w64txt)[ttile->getWallTXT()]);

                //note : this is common and needs to be done for all meshes, should be able to do a loop for all tile nodes?
                tnode->updateAbsolutePosition();

                //update mesh with common flags
                gptr->configMeshSceneNode(tnode);

                //link mesh to tile
                ttile->addWallMesh(tnode);
                //drop smesh
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
                //create scene node
                IMeshSceneNode *tnode = m_SMgr->addMeshSceneNode(wallmesh);

                //orient scene node
                tnode->setPosition( vector3df( y*UNIT_SCALE,0, x*UNIT_SCALE+UNIT_SCALE ) );
                tnode->setRotation( vector3df(0,90,0) );

                //set wall texture (common for all walls of tile)
                tnode->setMaterialTexture(0, (*w64txt)[ttile->getWallTXT()]);

                //note : this is common and needs to be done for all meshes, should be able to do a loop for all tile nodes?
                tnode->updateAbsolutePosition();

                //update mesh with common flags
                gptr->configMeshSceneNode(tnode);

                //link mesh to tile
                ttile->addWallMesh(tnode);
                //drop smesh
                wallmesh->drop();
            }
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
    mesh->setBoundingBox( aabbox3df(0,0,0,1*scale,0,1*scale));
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
    mesh->setBoundingBox( aabbox3df(0,0,0,1*scale,0,1*scale));
    //buf->recalculateBoundingBox();

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

    //finalize vertices
    buf->Indices.reallocate(vcount);
    buf->Indices.set_used(vcount);
    for(int i = 0; i < vcount; i++) buf->Indices[i] = i;
    mesh->setBoundingBox( aabbox3df(0,tl*scale,0,0,br*scale,1*scale));
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
    buf->Vertices[0] = S3DVertex(0*UNIT_SCALE,tl*scale,0*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 0, 0); //TL
    buf->Vertices[1] = S3DVertex(1*UNIT_SCALE,tr*scale,1*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 1, 0); //TR
    buf->Vertices[2] = S3DVertex(0*UNIT_SCALE,bl*scale,0*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 0, txtscaley);// BL
    //triangle 2
    buf->Vertices[3] = S3DVertex(1*UNIT_SCALE,tr*scale,1*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 1, 0); //TR
    buf->Vertices[4] = S3DVertex(1*UNIT_SCALE,br*scale,1*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 1, txtscaley); //BR
    buf->Vertices[5] = S3DVertex(0*UNIT_SCALE,bl*scale,0*UNIT_SCALE, 1,0,0,    video::SColor(255,255,255,255), 0, txtscaley); // BL

    //finalize vertices
    buf->Indices.reallocate(vcount);
    buf->Indices.set_used(vcount);
    for(int i = 0; i < vcount; i++) buf->Indices[i] = i;
    mesh->setBoundingBox( aabbox3df(0,tl*scale,0,0,br*scale,1*scale));
    //buf->recalculateBoundingBox();

    return mesh;
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
Tile::Tile()
{

    mType = TILETYPE_SOLID;
    mHeight = 0;
    mFloorTXTIndex = 0;
    mWallTXTIndex = 0;
    mFirstObjectIndex = 0;

    mMagicIllegal = false;
    mHasDoor = false;
    mUnk1 = false;
    mUnk2 = false;

    //geometry
    mMeshFloor = NULL;

}

Tile::~Tile()
{
    clearGeometry();
}


int Tile::clearGeometry()
{
    //counts valid geometry cleared
    int clearcounter = 0;

    //drop meshes
    if(mMeshFloor != NULL)
    {
        mMeshFloor->drop();
        clearcounter++;
    }

    for(int i = 0; i < int(mMeshWalls.size()); i++)
    {
        mMeshWalls[i]->drop();
        clearcounter++;
    }
    mMeshWalls.clear();

    return clearcounter;
}

bool Tile::setFloorMesh(IMeshSceneNode *tfloor)
{
    if(tfloor == NULL) return false;

    if(mMeshFloor != NULL)
    {
        std::cout << "Error, unable to set floor mesh, geometry not cleared first!\n";
        return false;
    }

    mMeshFloor = tfloor;
    return true;
}

bool Tile::addWallMesh(IMeshSceneNode *twall)
{
    if(twall == NULL) return false;

    mMeshWalls.push_back(twall);
    return true;
}

void Tile::printDebug()
{

    std::cout << "\nTILE INFO:\n";
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
    std::cout << "FIRST OBJ INDEX : " << getFirstObjectIndex() << std::endl;
    std::cout << "\nWALL MESHES : " << mMeshWalls.size() << std::endl;
    std::cout << "FLOOR MESH : ";
    if(mMeshFloor == NULL) std::cout << "NULL\n";
    else std::cout << "Y\n";



}
