#ifndef CLASS_SCROLL
#define CLASS_SCROLL

#include "game.hpp"
#include "irrcommon.hpp"


#define SCROLL_POS_X 15*SCREEN_SCALE
#define SCROLL_POS_Y 169*SCREEN_SCALE
#define SCROLL_WIDTH 291*SCREEN_SCALE
#define SCROLL_HEIGHT 30*SCREEN_SCALE
#define SCROLL_PAL_INDEX 42
#define SCROLL_EDGE_Y 169*SCREEN_SCALE
#define SCROLL_EDGE_LEFT_X 11*SCREEN_SCALE
#define SCROLL_EDGE_RIGHT_X 306*SCREEN_SCALE
#define SCROLL_DEFAULT_FONT_PAL 47
#define SCROLL_FONT_PAL_GREEN 253

enum SCROLLMSGFONTS
{
    FONT_NORMAL
};

class Game;

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

public:
    Scroll(Game *ngame);
    ~Scroll();

    rect<s32> getScrollRect() { return m_ScrollRect;}
    void setScrollRect(rect<s32> nrect) {m_ScrollRect = nrect;}

    int getScrollEdgeState() { return m_ScrollEdgeState;}

    void draw();
    void addMessage(std::string msgstring, int fonttype = FONT_NORMAL, SColor fcolor = m_DefaultColor);

};
#endif // CLASS_SCROLL
