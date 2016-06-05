#ifndef CLASS_LEVEL
#define CLASS_LEVEL

#define TILE_COLS 64
#define TILE_ROWS 64
#define CEIL_HEIGHT 15

#include <cstdlib>
#include <vector>

class Tile
{
private:
    //properties
    int mType; //0-9
        /*
            00      Solid (wall tile)
            01      Open (square tile of empty space)
            02      Diagonal, open SE
            03      Diagonal, open SW
            04      Diagonal, open NE
            05      Diagonal, open NW
            06      Sloping up to the north
            07      Sloping up to the south
            08      Sloping up to the east
            09      Sloping up to the west
        */
    int mHeight;
    bool mMagicIllegal;
    bool mHasDoor;

    bool mUnk1; // has some function in uw2, light level related
    bool mUnk2;

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
    int getHeight() { return mHeight;}
    int getFloorTXT() { return mFloorTXTIndex;}
    int getWallTXT() { return mWallTXTIndex;}
    int getFirstObjectIndex() { return mFirstObjectIndex;}
    bool hasDoor() { return mHasDoor;}
    bool isMagicIllegal() { return mMagicIllegal;}
    bool getUnk1() { return mUnk1;}
    bool getUnk2() { return mUnk2;}

};

class Level
{
private:
    std::vector< std::vector<Tile> > mTiles;

public:
    Level();
    ~Level();

    Tile *getTile(int x, int y);

    void printDebug();
};
#endif // CLASS_LEVEL
