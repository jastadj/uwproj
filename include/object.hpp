#ifndef CLASS_OBJECT
#define CLASS_OBJECT

#include <string>
#include "irrcommon.hpp"

class Object
{
private:

    static int m_TotalObjects;
    int m_ID;

    ITexture *m_TXT;

public:
    Object();
    ~Object();

    ITexture *getTexture() { return m_TXT;}
    int getID() { return m_ID;}

    void setTexture(ITexture *ntxt) { m_TXT = ntxt;}

    int getObjectCount() { return m_TotalObjects;}
};

class ObjectInstance
{
private:
    static int m_InstanceCount;
    int m_InstanceID;
    Object *m_Ref;
    IBillboardSceneNode *m_Billboard;


    //object flags
    int m_Flags;
    bool m_Enchanted; // is enchanted
    bool m_DoorDir; // door direction flag
    bool m_Invisible; // invisible flag (don't draw)
    bool m_IsQuantity; // link field is quantity / special field

    //position properties
    vector3di m_Position; // position within tile, x&y 0-7, z = 0-127 (8 * 16 tiles high)
    int m_Angle; // angle * 45 deg

    //quality / chain
    int m_Quality; //quality data
    int m_Next; // index of next object in chain

    //link / special
    int m_Owner; // owner / special
    int m_Quantity; // quantity / special link / special property

public:
    ObjectInstance(Object *tobj);
    ~ObjectInstance();

    //reference
    Object *getRef() { return m_Ref;}
    int getRefID() { return m_Ref->getID();}
    int getInstanceID() { return m_InstanceID;}
    ITexture *getTexture() { return m_Ref->getTexture();}

    //billboard
    IBillboardSceneNode *getBillboard() { return m_Billboard;}
    bool setBillboard(IBillboardSceneNode *tbb);

    //object flags
    int getFlags() { return m_Flags;}
    void setFlags(int nflags) { m_Flags = nflags;}
    bool isEnchanted() { return m_Enchanted;}
    void setEnchanted(bool nench) { m_Enchanted = nench;}
    bool getDoorDir() { return m_DoorDir;}
    void setDoorDir(bool ndoordir) { m_DoorDir = ndoordir;}
    bool isInvisible() { return m_Invisible;}
    void setInvisible(bool ninvis) { m_Invisible = ninvis;}
    bool isQuantity() { return m_IsQuantity;}
    void setIsQuantity( bool nisquant) { m_IsQuantity = nisquant;}

    //position
    vector3di getPosition() { return m_Position;}
    void setPosition(vector3di newpos) { m_Position = newpos;}
    int getAngle() { return m_Angle;}
    void setAngle(int nangle) { m_Angle = nangle;}

    //quality / chain
    int getQuality() { return m_Quality;}
    void setQuality(int nqual) { m_Quality = nqual;}
    int getNext() { return m_Next;}
    void setNext(int nnext) { m_Next = nnext;}

    //link / special
    int getOwner() {return m_Owner;}
    void setOwner(int nowner) { m_Owner = nowner;}
    int getQuantity() { return m_Quantity;}
    void setQuantity(int nquant) { m_Quantity = nquant;}

    int getObjectInstanceCount() { return m_InstanceCount;}

    //debug
    void printDebug();
};

#endif // CLASS_OBJECT
