#include "console.hpp"

Console *Console::m_Instance = NULL;

Console::Console()
{
    //get game ref
    gptr = Game::getInstance();
}

Console::~Console()
{

}

void Console::addMessage(std::string msgstring, int fonttype, SColor fcolor)
{
    gptr->addMessage(msgstring, fonttype, fcolor);
}

void Console::parse(std::string cmdstring)
{
    if(cmdstring == "test")
    {
        addMessage("this is a test");
    }
}
