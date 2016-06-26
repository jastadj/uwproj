#include "scroll.hpp"

Scroll::Scroll(Game *ngame)
{
    //link references
    gptr = ngame;
    m_FontNormal = gptr->getNormalFont();

    m_ScrollEdgeState = 0;

    m_ScrollRect = rect<s32>(position2d<s32>(SCROLL_POS_X, SCROLL_POS_Y), dimension2d<u32>(SCROLL_WIDTH, SCROLL_HEIGHT) );

    addMessage("This is a test.");
}

Scroll::~Scroll()
{

}

void Scroll::draw()
{
    for(int i = 0; i < int(m_MsgBuffer.size()); i++)
    {
        //determine where to draw message
        position2d<s32> mpos = m_ScrollRect.UpperLeftCorner;
        mpos.Y += m_MsgBuffer[i].font->m_Height;

        //draw message
        drawFontString(m_MsgBuffer[i].font, m_MsgBuffer[i].msg, mpos, m_MsgBuffer[i].color );
    }
}

void Scroll::addMessage(std::string msgstring, int fonttype, SColor fcolor)
{
    //create scroll message
    ScrollMessage newmsg;
    newmsg.msg = msgstring;
    newmsg.color = fcolor;

    switch(fonttype)
    {
    case FONT_NORMAL:
        newmsg.font = m_FontNormal;
        break;
    default:
        newmsg.font = m_FontNormal;
        break;
    }

    m_MsgBuffer.push_back(newmsg);
}
