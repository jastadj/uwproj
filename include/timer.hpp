#ifndef CLASS_TIMER
#define CLASS_TIMER

#include "irrcommon.hpp"

class Timer
{
private:

    enum
    {
        TIMER_STOPPED,
        TIMER_RUNNING
    };

    IrrlichtDevice *m_Device;
    u32 m_TimeUponCreation;

    int m_TimerState;

public:
    Timer();
    ~Timer();

    u32 getElapsedTime() { return m_Device->getTimer()->getTime() - m_TimeUponCreation;}
    void reset() { m_TimeUponCreation = m_Device->getTimer()->getTime();}

};
#endif // CLASS_TIMER
