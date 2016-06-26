#include "timer.hpp"

#include "game.hpp"

Timer::Timer()
{
    Game *gptr = NULL;
    gptr = Game::getInstance();


    m_Device = gptr->getDevice();
    m_TimeUponCreation = m_Device->getTimer()->getTime();

    m_TimerState = TIMER_STOPPED;

}

Timer::~Timer()
{

}
