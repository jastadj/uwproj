#include "level.hpp"

Level::Level()
{
    mTiles.resize(TILE_ROWS);
    for(int i = 0; i < TILE_COLS; i++) mTiles[i].resize(TILE_COLS);
}

Level::~Level()
{

}

/////////////////////////////////////////////////////////////////////
//  TILE
Tile::Tile()
{

    mType = 0;
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

}
