#ifndef CLASS_EVENT
#define CLASS_EVENT


#include "irrcommon.hpp"

#include "game.hpp"

//forward declarations
class Game;

class MyEventReceiver : public IEventReceiver
{

private:

    //std::vector<const SEvent*> eventque;

    bool Keys[KEY_KEY_CODES_COUNT];

    Game *gptr;

public:
    MyEventReceiver(Game *ngame);

    // This is the one method that we have to implement
    bool OnEvent(const SEvent &event);

    bool isKeyPressed(EKEY_CODE keycode){ return Keys[keycode]; }



    /*
    bool processEvents(const SEvent *&event)
    {
        //no events to process
        if(eventque.empty()) return false;

        //setting event pointer to first event in que
        event = eventque[0];
        //event = eventque.back();

        //removing processed event from que
        eventque.erase(eventque.begin());

        return true;
    }
    */


/*
    int queSize() { return int(eventque.size());}
    void clearQue()
    {
        for(int i = 0; i < int(eventque.size()); i++) delete eventque[i];
        eventque.clear();
    }
    bool getEvent(int eindex, const SEvent *event)
    {
        if(eindex < 0 || eindex >= int(eventque.size()) ) return false;

        event = eventque[eindex];
        return true;
    }
*/

};

#endif // CLASS_EVENT
