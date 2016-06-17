#ifndef CLASS_OBJECT
#define CLASS_OBJECT

#include <string>
#include <irrlicht.h>

//irrlicht namespaces
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class Object
{
private:

    static int m_TotalObjects;
    int m_ID;

    ITexture *m_TXT;
    std::string m_Description;

public:
    Object();
    ~Object();

    ITexture *getTexture() { return m_TXT;}
    std::string getDescription() { return m_Description;}
    int getID() { return m_ID;}

    void setTexture(ITexture *ntxt) { m_TXT = ntxt;}
    void setDescription(std::string ndesc) { m_Description = ndesc;}
};

class ObjectInstance: public Object
{
private:
    Object *m_Ref;
    vector3df m_Position;
    float m_Angle;

public:
    ObjectInstance(Object *tobj);
    ~ObjectInstance();
};

#endif // CLASS_OBJECT
