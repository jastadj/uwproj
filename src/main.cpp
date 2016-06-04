#include <cstdlib>

#include "game.hpp"

int main(int argc, char *argv[])
{
    Game *game;
    game = Game::getInstance();

    game->start();

    return 0;
}
