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

bool Level::buildTileGeometry(int x, int y)
{
    //get target tile at x,y coordinate
    Tile *ttile = getTile(x,y);
    int ttype = 0;

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
    if(ttile->getType() == TILETYPE_SOLID) return true;

    //clear tile geometry
    ttile->clearGeometry();

    //default tile heights to height value
    for(int i = 0; i < 4; i++)
    {
        if(!ttile->setHeightMap(i, ttile->getHeight()) ) return false;;
    }

    //floor
    switch( ttype)
    {
    case TILETYPE_SL_S:
        ttile->setHeightMap(SW, ttile->getHeight()+(UNIT_SCALE/4) );
        ttile->setHeightMap(SE, ttile->getHeight()+(UNIT_SCALE/4) );
        break;
    case TILETYPE_SL_N:
        ttile->setHeightMap(NW, ttile->getHeight()+(UNIT_SCALE/4) );
        ttile->setHeightMap(NE, ttile->getHeight()+(UNIT_SCALE/4) );
        break;
    case TILETYPE_SL_W:
        ttile->setHeightMap(NW, ttile->getHeight()+(UNIT_SCALE/4) );
        ttile->setHeightMap(SW, ttile->getHeight()+(UNIT_SCALE/4) );
        break;
    case TILETYPE_SL_E:
        ttile->setHeightMap(SE, ttile->getHeight()+(UNIT_SCALE/4) );
        ttile->setHeightMap(NE, ttile->getHeight()+(UNIT_SCALE/4) );
        break;
    default:
        break;
    }

    //get heights after height geometry has been recalculated
    const std::vector<int> theights = ttile->getHeightMap();

    //generate floor mesh
    SMesh *floormesh = NULL;

    // if diagonal type, generate alternate floor (triangle)
    if(ttype >=2 && ttype <= 5) floormesh = generateFloorMesh(theights[0], theights[1], theights[2]);
    // else generate a full floor
    else floormesh = generateFloorMesh(theights[0], theights[1], theights[2], theights[3]);

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


    return true;
}

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
    mHeightMap.resize(4);

}

Tile::~Tile()
{
    clearGeometry();
}

bool Tile::setHeightMap(int coordnum, int height)
{
    if( coordnum < 0 || coordnum >= 4)
    {
        std::cout << "Error setting tile height map: Not a valid height coordinate! 0-3\n";
        return false;
    }

    //if tile type is solid, automatically default to solid ceil height
    if(mType == TILETYPE_SOLID)
    {
        mHeightMap[coordnum] = CEIL_HEIGHT+1;
        return true;
    }

    if(height < 0 || height > CEIL_HEIGHT)
    {
        std::cout << "Error setting tile height map :"  << height << " not a valid height!\n";
    }

    //set height map value
    mHeightMap[coordnum] = height;

    return true;
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
