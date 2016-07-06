#include "player.hpp"
#include <iostream>

Player::Player()
{
    //set player defaults
    m_Name = "Player";

    //create empty inventory slots
    m_InvSlots.resize(INV_TOTALSLOTS);
    for(int i = 0; i < int(m_InvSlots.size()); i++) m_InvSlots[i] = NULL;
}

Player::~Player()
{

}

ObjectInstance *Player::getInventorySlot(int slotnum)
{
    if(slotnum < 0 || slotnum >= INV_TOTALSLOTS)
    {
        std::cout << "Error getting player inv slot!  Slot " << slotnum << " not valid!\n";
        return NULL;
    }

    return m_InvSlots[slotnum];
}

bool Player::setInventorySlot(int slotnum, ObjectInstance *tobj)
{
    if(slotnum < 0 || slotnum >= INV_TOTALSLOTS)
    {
        std::cout << "Error setting player inv slot!  Slot " << slotnum << " not valid!\n";
        return false;
    }

    if(m_InvSlots[slotnum] != NULL)
    {
        std::cout << "Inventory slot not empty!\n";
        return false;
    }

    if(tobj == NULL)
    {
        std::cout << "Error setting inv slot, NULL is no valid.  Use popInventorySlot to remove object!\n";
        return false;
    }

    m_InvSlots[slotnum] = tobj;
}

ObjectInstance *Player::popInventorySlot(int slotnum)
{
    ObjectInstance *tobj = NULL;

    if(slotnum < 0 || slotnum >= INV_TOTALSLOTS)
    {
        std::cout << "Error getting player inv slot!  Slot " << slotnum << " not valid!\n";
        return tobj;
    }

    if(m_InvSlots[slotnum] == NULL)
    {
        std::cout << "Error popping inv slot.  Slot is null!\n";
        return tobj;
    }

    //get object in slot
    tobj = m_InvSlots[slotnum];

    //set slot to null
    m_InvSlots[slotnum] = NULL;

    //return object
    return tobj;
}
