#ifndef CLASS_GAME
#define CLASS_GAME

#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "level.hpp"
#include <SFML\Graphics.hpp>
#include <SFML\OpenGL.hpp>

class Game
{
private:
    Game();
    static Game *mInstance;

    //render window
    sf::RenderWindow *mScreen;
    sf::ContextSettings mScreenContext;

    //init
    void initScreen();
    void loadlevel();

    //levels
    std::vector<Level> mLevels;

    //mainloop
    void mainLoop();

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
