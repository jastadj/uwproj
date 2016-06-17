#include "object.hpp"
#include "game.hpp"

int Object::m_TotalObjects = 0;

Object::Object()
{
    Game *gptr = NULL;
    gptr = Game::getInstance();

    //assign object id
    m_ID = m_TotalObjects;

    //increase total object counter
    m_TotalObjects++;

    //set default texture to question mark
    m_TXT = gptr->getDefaultTexture();

    //set default description
    m_Description = gptr->getDefaultString();

}

Object::~Object()
{

}
