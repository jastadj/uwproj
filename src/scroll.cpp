#include "scroll.hpp"

SColor Scroll::m_DefaultColor(SColor(255,255,255,255));
Scroll::Scroll(Game *ngame)
{
    //link references
    gptr = ngame;
    m_FontNormal = gptr->getNormalFont();

    m_ScrollEdgeState = 0;
    m_ScrollStartIndex = 0;

    m_InputModeString = NULL;
    m_CursorGraphic = NULL;

    m_ScrollRect = rect<s32>(position2d<s32>(SCROLL_POS), dimension2d<u32>(SCROLL_DIM) );

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

bool Scroll::startInputMode(std::string *tstring, std::string promptstr)
{
    if(m_InputModeString != NULL) return false;

    //get reference to target string, where input is going to be stored
    m_InputModeString = tstring;

    //draw prompt
    addMessage(promptstr, FONT_NORMAL);

    //create cursor graphic from current prompt font and color
    UWFont *tfont = m_MsgBuffer.back().font;
    IImage *newimg = gptr->getDriver()->createImage(ECF_A1R5G5B5, dimension2d<u32>(tfont->m_WidestCharacter, tfont->m_Height ));
    newimg->fill(m_MsgBuffer.back().color);
    m_CursorGraphic = gptr->getDriver()->addTexture( "temp", newimg );
    newimg->drop();

    //reset cursor blink timer
    m_CursorTimer.reset();

    //tell game that input context is scroll input entry
    gptr->setInputContext(IMODE_SCROLL_ENTRY);
}

void Scroll::endInputMode()
{
    //for some reason there was no dynamic???
    if(m_InputModeString == NULL)
    {
        std::cout << "Error, input mode string reference is null??\n";
    }

    //destroy cursor graphic
    m_CursorGraphic->drop();
    m_CursorGraphic = NULL;

    //tell game that scroll input is done and return to previous context
    gptr->setInputContext( gptr->getPreviousInputContext());

    //add string to message buf (assumes last message was for prompt)
    m_MsgBuffer.back().msg += *m_InputModeString;

    //send string to console parser
    gptr->sendToConsole(*m_InputModeString);

    //copy input string and delete dynamic
    m_InputModeString = NULL;
}

void Scroll::addInputCharacter(int cval)
{
    if(m_InputModeString == NULL)
    {
        std::cout << "Input mode string reference is null, returning...\n";
        return;
    }

    if(cval < 0 || cval >= 127)
    {
        std::cout << "Input mode key val:" << cval << " is not valid [0-127]\n";
        return;
    }

    //handle key presses

    switch(cval)
    {
    case KEY_ESCAPE:
        //exit key entry mode
        endInputMode();
        break;
    case KEY_RETURN:
        //exit key entry mode
        endInputMode();
        break;
    case KEY_BACK:
        //remove character
        if(m_InputModeString->length() > 0) m_InputModeString->resize( m_InputModeString->length()-1);
        break;
    default:
        //ignore everything else below the space character (32)
        //add character to string
        if(cval >= 32) m_InputModeString->push_back(char(cval));
        break;
    }

}

void Scroll::draw()
{
    //update scroll edges using modulus of start index
    m_ScrollEdgeState = abs(m_ScrollStartIndex) % 5;

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
                           m_MsgBuffer[i + m_ScrollStartIndex].msg,
                           mpos,
                           m_MsgBuffer[i + m_ScrollStartIndex].color );

            //if it's the last message, and in input mode, draw input string
            if(i + m_ScrollStartIndex == int(m_MsgBuffer.size())-1 && gptr->getInputContext() == IMODE_SCROLL_ENTRY)
            {
                //if input mode string is not null
                if(m_InputModeString != NULL)
                {
                    //draw message again but attach input mode string
                    drawFontString(m_MsgBuffer[i + m_ScrollStartIndex].font,
                                   m_MsgBuffer[i + m_ScrollStartIndex].msg + *m_InputModeString,
                                   mpos,
                                   m_MsgBuffer[i + m_ScrollStartIndex].color );

                    //draw a cursor
                    if(m_CursorGraphic != NULL && m_CursorTimer.getElapsedTime() < SCROLL_CURSOR_BLINK)
                    {
                        //get width of message
                        int msgwidth = getStringWidth(m_MsgBuffer[i + m_ScrollStartIndex].font,  std::string(m_MsgBuffer[i+m_ScrollStartIndex].msg + *m_InputModeString) );
                        gptr->getDriver()->draw2DImage( m_CursorGraphic, position2d<s32>(mpos.X + msgwidth, mpos.Y ));
                    }
                    if(m_CursorTimer.getElapsedTime() >= SCROLL_CURSOR_BLINK*2) m_CursorTimer.reset();

                }

            }//end of input mode drawing
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
