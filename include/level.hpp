#ifndef CLASS_LEVEL
#define CLASS_LEVEL

#define TILE_COLS 64
#define TILE_ROWS 64
#define CEIL_HEIGHT 15

#include <cstdlib>
#include <vector>

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

enum _TILETYPE{TILETYPE_SOLID, TILETYPE_OPEN, TILETYPE_D_SE, TILETYPE_D_SW, TILETYPE_D_NE, TILETYPE_D_NW,
                TILETYPE_SL_N, TILETYPE_SL_S, TILETYPE_SL_E, TILETYPE_SL_W, TILETYPE_TOTAL};

enum _DCOORDS{NW, NE, SE, SW};

enum _DIRS{NORTH,EAST,SOUTH,WEST};

//forward declaration
class Tile;


class Level
{
private:
    std::vector< std::vector<Tile> > mTiles;

    int m_CeilingTextureIndex;

public:
    Level();
    ~Level();

    //not used beyond loading but might as well save it
    std::vector<int> mTextureMapping;

    Tile *getTile(int x, int y);

    bool buildLevelGeometry(); //high level, geomery gen for entire map
    bool buildTileGeometry(int x, int y); // lower level, geometry for individual tile

    //NOTE NEED TO CHANGE PARAMETERS TO F32, CANT DIVIDE SCALING WITH INT (UNLESS CASTED FIRST)
    SMesh *generateFloorMesh(int ul, int ur, int br, int bl); // generate floor model
    SMesh *generateFloorMesh(int p1, int p2, int p3); // generate diagonal floor model

    SMesh *generateWallMesh(int tl, int tr, int br, int bl); // generate wall model
    SMesh *generateDiagonalWallMesh(int tl, int tr, int br, int bl); // generate diagonal wall model

    std::vector<Tile*> getAdjacentTilesAt(int x, int y);

    //a level uses one ceiling texture
    int getCeilingTextureIndex() { return m_CeilingTextureIndex;}
    void setCeilingTextureIndex(int nindex) { m_CeilingTextureIndex = nindex;}

    std::vector<IMeshSceneNode*> getMeshes();

    void printDebug();
};



class Tile
{
private:
    //properties
    int mType; // _TILETYPE

    int mHeight;
    bool mMagicIllegal;
    bool mHasDoor;

    bool mUnk1; // has some function in uw2, light level related
    bool mUnk2;

    //geometry
    std::vector<IMeshSceneNode*> mMeshes;

    //texture indices
    int mFloorTXTIndex;
    int mWallTXTIndex;

    //objects
    int mFirstObjectIndex;

public:
    Tile();
    ~Tile();

    //set tile data
    void setType(int ntype) { mType = ntype;}
    void setHeight(int nheight) { mHeight = nheight;}
    void setFloorTXT(int nfloor) { mFloorTXTIndex = nfloor;}
    void setWallTXT(int nwall) { mWallTXTIndex = nwall;}
    void setFirstObjectIndex(int nindex) {mFirstObjectIndex = nindex;}
    void setHasDoor(bool ndoor) { mHasDoor = ndoor;}
    void setMagicIllegal(bool nmagic) { mMagicIllegal = nmagic;}
    void setUnk1(bool nval) { mUnk1 = nval;}
    void setUnk2(bool nval) {mUnk2 = nval;}

    //get tile data
    int getType() { return mType;}
    int getHeight() { if(mType == 0) return CEIL_HEIGHT+1; else return mHeight;}
    int getFloorTXT() { return mFloorTXTIndex;}
    int getWallTXT() { return mWallTXTIndex;}
    int getFirstObjectIndex() { return mFirstObjectIndex;}
    bool hasDoor() { return mHasDoor;}
    bool isMagicIllegal() { return mMagicIllegal;}
    bool getUnk1() { return mUnk1;}
    bool getUnk2() { return mUnk2;}

    //geometry
    int clearGeometry();
    bool addMesh(IMeshSceneNode *tnode);
    std::vector<IMeshSceneNode*> getMeshes();

    //debug
    void printDebug();
};

#endif // CLASS_LEVEL
