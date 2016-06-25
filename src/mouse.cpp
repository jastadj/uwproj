#include "mouse.hpp"

Mouse::Mouse(Game *tgame)
{
    m_Texture = NULL;

    //link to game reference
    gptr = tgame;

    //get references
    m_Device = gptr->getDevice();
    m_Driver = gptr->getDriver();
}

void Mouse::updatePosition()
{
    m_MousePos = m_Device->getCursorControl()->getPosition();
}

void Mouse::setTexture(ITexture *ttxt)
{
    m_Texture = ttxt;
}

void Mouse::draw()
{
    if(m_Texture == NULL) return;

    core::rect<s32> screen_rect( position2d<s32>(0,0), m_Texture->getSize());
    m_Driver->draw2DImage( m_Texture, m_MousePos, screen_rect, NULL, SColor(255,255,255,255), true);
}
