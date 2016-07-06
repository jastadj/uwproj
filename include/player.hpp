#ifndef CLASS_PLAYER
#define CLASS_PLAYER

#include "irrcommon.hpp"
#include <string>
#include <vector>

#include "object.hpp"

//player inventory slots
// note : left and right relative to player
enum _INVSLOTS{ INV_SLOT0,
                INV_SLOT1,
                INV_SLOT2,
                INV_SLOT3,
                INV_SLOT4,
                INV_SLOT5,
                INV_SLOT6,
                INV_SLOT7,
                INV_BACKRIGHT,
                INV_BACKLEFT,
                INV_HANDRIGHT,
                INV_HANDLEFT,
                INV_RINGRIGHT,
                INV_RINGLEFT,
                INV_ARMORHEAD,
                INV_ARMORCHEST,
                INV_ARMORHANDS,
                INV_ARMORLEGS,
                INV_ARMORFEET,
                INV_TOTALSLOTS
                };

class Player
{
private:

    vector3df m_Position;
    vector3df m_Rotation;
    vector3df m_Velocity;

    std::string m_Name;

    //inventory slots
    std::vector<ObjectInstance*> m_InvSlots;


public:
    Player();
    ~Player();

    std::string getName() { return m_Name;}

    vector3df getPosition() { return m_Position;}
    vector3df getRotation() { return m_Rotation;}
    vector3df getVelocity() { return m_Velocity;}

    void setPosition(vector3df newpos) { m_Position = newpos;}
    void setRotation(vector3df nrot) { m_Rotation = nrot;}
    void setVelocity(vector3df nvel) { m_Velocity = nvel;}

    ObjectInstance *getInventorySlot(int slotnum);
    bool setInventorySlot(int slotnum, ObjectInstance *tobj);
    ObjectInstance *popInventorySlot(int slotnum);
};
#endif // CLASS_PLAYER
