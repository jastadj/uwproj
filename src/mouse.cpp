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
    //disable mouse cursor
    m_Device->getCursorControl()->setVisible(false);

    if(m_Texture == NULL) return;

    vector2di tsize(m_Texture->getSize().Width, m_Texture->getSize().Height);
    tsize.X = tsize.X/2;
    tsize.Y = tsize.Y/2;

    core::rect<s32> screen_rect( position2d<s32>(0,0), m_Texture->getSize());
    m_Driver->draw2DImage( m_Texture, m_MousePos - tsize, screen_rect, NULL, SColor(255,255,255,255), true);
}

////////////////////////////////////////////////////////////
//  MOUSE UPDATE THREAD
MouseUpdateThread::MouseUpdateThread(Mouse *nmouse, bool *nshutdownflag)
{
    m_Mouse = nmouse;

    shutdownflag = nshutdownflag;
}

MouseUpdateThread::~MouseUpdateThread()
{

}

void MouseUpdateThread::InternalThreadEntry()
{
    std::cout << "Starting mouse update thread...\n";
    while(!(*shutdownflag) )
    {
        m_Mouse->updatePosition();
    }

    pthread_exit(NULL);
}
