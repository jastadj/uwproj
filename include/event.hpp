#ifndef CLASS_EVENT
#define CLASS_EVENT


#include "irrcommon.hpp"

class MyEventReceiver : public IEventReceiver
{

private:

    std::vector<const SEvent*> eventque;

    bool Keys[KEY_KEY_CODES_COUNT];

public:
    MyEventReceiver() {  }

    // This is the one method that we have to implement
    virtual bool OnEvent(const SEvent& event)
    {
        eventque.push_back(&event);

        //capture state of key presses
        if(event.EventType == EET_KEY_INPUT_EVENT)
        {
            Keys[event.KeyInput.Key] = event.KeyInput.PressedDown;
        }

        return false;
    }

    bool processEvents(const SEvent *&event)
    {
        //no events to process
        if(eventque.empty()) return false;

        //setting event pointer to current event in que
        event = eventque.back();

        //removing processed event from que
        eventque.erase(eventque.begin() + eventque.size()-1);

        return true;
    }

    bool isKeyPressed(EKEY_CODE keycode)
    {
        return Keys[keycode];
    }

    int queSize() { return int(eventque.size());}
};

#endif // CLASS_EVENT
