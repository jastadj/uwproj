#ifndef CLASS_GAME
#define CLASS_GAME

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "level.hpp"

class Game
{
private:
    Game();
    static Game *mInstance;

    //init
    void loadlevel();

    //levels
    std::vector<Level> mLevels;


public:
    static Game *getInstance()
    {
        if(mInstance == NULL) mInstance = new Game;
        return mInstance;
    }
    ~Game();

    void start();
};
#endif // CLASS_GAME
