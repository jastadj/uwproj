#include "scroll.hpp"

SColor Scroll::m_DefaultColor(SColor(255,255,255,255));
Scroll::Scroll(Game *ngame)
{
    //link references
    gptr = ngame;
    m_FontNormal = gptr->getNormalFont();

    m_ScrollEdgeState = 0;
    m_ScrollStartIndex = 0;

    m_ScrollRect = rect<s32>(position2d<s32>(SCROLL_POS_X, SCROLL_POS_Y), dimension2d<u32>(SCROLL_WIDTH, SCROLL_HEIGHT) );

    if(SCROLL_DEFAULT_FONT_PAL >= 0 && SCROLL_DEFAULT_FONT_PAL < int((*gptr->getPalletes())[0].size()) )
        m_DefaultColor = (*gptr->getPalletes())[0][SCROLL_DEFAULT_FONT_PAL];
    else
    {
        std::cout << "Error initializing scroll default color!\n";
        m_DefaultColor = SColor(255,255,255,255);
    }

}

Scroll::~Scroll()
{

}

void Scroll::draw()
{
    //update scroll edges using modulus of start index
    m_ScrollEdgeState = abs(m_ScrollStartIndex) % 5;
    std::cout << "scroll edge state = " << m_ScrollEdgeState << std::endl;
    std::cout << "scroll start index = " << m_ScrollStartIndex << std::endl;

    //if start index is too far up or down, adjust
    if(m_ScrollStartIndex < -4) m_ScrollStartIndex = -4;
    else if(m_ScrollStartIndex >= int(m_MsgBuffer.size()) && !m_MsgBuffer.empty() ) m_ScrollStartIndex = int(m_MsgBuffer.size()-1);

    for(int i = 0; i < 5; i++)
    {
        //determine where to draw message
        position2d<s32> mpos = m_ScrollRect.UpperLeftCorner;

        //if i and scroll start index are valid
        if(i + m_ScrollStartIndex >= 0 && i + m_ScrollStartIndex < int(m_MsgBuffer.size()) )
        {
            mpos.Y += m_MsgBuffer[i + m_ScrollStartIndex].font->m_Height * i;

            //draw message
            drawFontString(m_MsgBuffer[i + m_ScrollStartIndex].font,
                           m_MsgBuffer[i + m_ScrollStartIndex].msg, mpos,
                           m_MsgBuffer[i + m_ScrollStartIndex].color );
        }

    }
}

void Scroll::addMessage(std::string msgstring, int fonttype, SColor fcolor)
{
    UWFont *font = NULL;

    switch(fonttype)
    {
    case FONT_NORMAL:
        font = m_FontNormal;
        break;
    default:
        font = m_FontNormal;
        break;
    }

    //if message is too long for scroll window, break off at closest space and create new message
    int maxwidth = m_ScrollRect.getWidth();

    //container for substrings
    std::vector<std::string> substrings;

    //working string
    std::string wstring = msgstring;

    int curpos = 0;
    int curwidth = 0;

    //check each character and calculate current message position ,checking to see if it exceeds width
    bool endofstring = false;
    while(!endofstring)
    {
        int charval = int(wstring[curpos]);

        //only process recognized characters
        if(charval >= 0 && charval < font->m_Count )
        {
            //advance current width by character width
            curwidth += font->m_Clips[charval].getWidth();

            //if current position exceeds scroll window length
            if(curwidth >= maxwidth)
            {
                //capture current widths, positions, and values
                int retroval = charval;
                int retropos = curpos;

                //back up until a space is found or beginning of string is found
                while(retroval != 32 && retropos > 0)
                {
                    //subtract width
                    curwidth -= font->m_Clips[retroval].getWidth();

                    //move back one character
                    retropos--;

                    //as long as position is valid
                    if(retropos >= 0)
                    {
                        //get character value at retro position
                        retroval = int(wstring[retropos]);
                    }
                }

                //did the string run all the way to the front?
                if(retropos == 0)
                {
                    //since no space could be found, just shear off string at max width
                    substrings.push_back( wstring.substr(0, curpos-1));
                    wstring = wstring.substr(curpos);
                }
                //else found a space
                else
                {
                    substrings.push_back( wstring.substr(0, retropos));
                    wstring = wstring.substr(retropos+1);
                }

                //reset position and width
                curpos = 0;
                curwidth = 0;

            }
            //else, advance
            else
            {
                curpos++;
            }
        }
        //else advance
        else
        {
            curpos++;
        }

        //if current position has reached end of working string
        if(curpos >= int(wstring.length()) )
        {
            endofstring = true;
        }
    }

    //push working string into string container
    substrings.push_back(wstring);

    //create one or more messages from string container
    for(int i = 0; i < int(substrings.size()); i++)
    {
        //create scroll message
        ScrollMessage newmsg;
        newmsg.msg = substrings[i];
        newmsg.color = fcolor;
        newmsg.font = font;
        m_MsgBuffer.push_back(newmsg);
    }

    //reposition scroll window message index if necessary
    if(m_ScrollStartIndex < int(m_MsgBuffer.size())-5) m_ScrollStartIndex = m_MsgBuffer.size()-5;
}
