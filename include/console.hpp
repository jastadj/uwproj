#ifndef CLASS_CONSOLE
#define CLASS_CONSOLE

#include <string>

#include "irrcommon.hpp"
#include "game.hpp"
#include "scroll.hpp"

class Game;
class Scroll;

class Console
{
private:
    Console();
    static Console *m_Instance;

    //references
    Game *gptr;
    Scroll *m_Scroll;

public:

    static Console *getInstance()
    {
        if(m_Instance == NULL) m_Instance = new Console;
        return m_Instance;
    }
    ~Console();

    std::string m_String;

    void parse(std::string cmdstring);

    void addMessage(std::string msgstring, int fonttype = 0, SColor fcolor = SColor(255,255,255,255));
};
#endif // CLASS_CONSOLE
