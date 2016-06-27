#ifndef CLASS_PLAYER
#define CLASS_PLAYER

#include "irrcommon.hpp"

class Player
{
private:

    vector3df m_Position;
    vector3df m_Rotation;
    vector3df m_Velocity;

public:
    Player();
    ~Player();

    vector3df getPosition() { return m_Position;}
    vector3df getRotation() { return m_Rotation;}
    vector3df getVelocity() { return m_Velocity;}

    void setPosition(vector3df newpos) { m_Position = newpos;}
    void setRotation(vector3df nrot) { m_Rotation = nrot;}
    void setVelocity(vector3df nvel) { m_Velocity = nvel;}
};
#endif // CLASS_PLAYER
