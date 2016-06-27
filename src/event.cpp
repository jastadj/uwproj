#include "event.hpp"

MyEventReceiver::MyEventReceiver(Game *ngame)
{
    gptr = ngame;

    //null out all keys
    for(int i = 0; i < KEY_KEY_CODES_COUNT; i++) Keys[i] = false;
}

bool MyEventReceiver::OnEvent(const SEvent &event)
{
    //eventque.push_back(&event);

    //capture state of key presses
    if(event.EventType == EET_KEY_INPUT_EVENT)
    {
        Keys[event.KeyInput.Key] = event.KeyInput.PressedDown;
    }

    if(gptr == NULL) std::cout << "Error in event receiver : game reference not set!\n";
    gptr->processEvent(&event);


    //return false, tell irrlicht this event is not done yet
    return true;
}
