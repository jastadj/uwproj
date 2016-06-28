#ifndef CLASS_SCROLL
#define CLASS_SCROLL

#include "game.hpp"
#include "irrcommon.hpp"

#include "timer.hpp"

#define SCROLL_POS 15*SCREEN_SCALE,169*SCREEN_SCALE
#define SCROLL_DIM 291*SCREEN_SCALE,30*SCREEN_SCALE
#define SCROLL_PAL_INDEX 42
#define SCROLL_EDGE_LEFT 11*SCREEN_SCALE,169*SCREEN_SCALE
#define SCROLL_EDGE_RIGHT 306*SCREEN_SCALE,169*SCREEN_SCALE
#define SCROLL_DEFAULT_FONT_PAL 47
#define SCROLL_FONT_PAL_GREEN 253
#define SCROLL_CURSOR_BLINK 500

enum SCROLLMSGFONTS
{
    FONT_NORMAL
};

class Game;
class UWFont;

struct ScrollMessage
{
    std::string msg;
    SColor color;
    UWFont *font;
};

class Scroll
{
private:

    //references
    Game *gptr;

    int m_ScrollEdgeState;

    rect<s32> m_ScrollRect;
    static SColor m_DefaultColor;
    int m_ScrollStartIndex;

    //fonts
    UWFont *m_FontNormal;

    //message buffer
    std::vector<ScrollMessage> m_MsgBuffer;

    //input mode
    std::string *m_InputModeString;
    ITexture *m_CursorGraphic;
    Timer m_CursorTimer;


public:
    Scroll(Game *ngame);
    ~Scroll();

    rect<s32> getScrollRect() { return m_ScrollRect;}
    void setScrollRect(rect<s32> nrect) {m_ScrollRect = nrect;}

    int getScrollEdgeState() { return m_ScrollEdgeState;}

    void draw();
    void addMessage(std::string msgstring, int fonttype = FONT_NORMAL, SColor fcolor = m_DefaultColor);

    //input mode
    bool startInputMode( std::string *tstring, std::string promptstr = std::string(">"));
    void endInputMode();
    void addInputCharacter(int cval);

};
#endif // CLASS_SCROLL
