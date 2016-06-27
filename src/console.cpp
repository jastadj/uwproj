#include "console.hpp"
#include <sstream>

Console *Console::m_Instance = NULL;

Console::Console()
{
    //get game ref
    gptr = Game::getInstance();
}

Console::~Console()
{

}

void Console::addMessage(std::string msgstring, int fonttype, SColor fcolor)
{
    gptr->addMessage(msgstring, fonttype, fcolor);
}

void Console::parse(std::string cmdstring)
{
    std::vector<std::string> words;

    //parse out each word from strings and store in vector
    size_t tpos = cmdstring.find_first_of(' ');
    while(tpos != std::string::npos)
    {
        words.push_back( cmdstring.substr(0, tpos));
        cmdstring = cmdstring.substr(tpos+1);
        tpos = cmdstring.find_first_of(' ');
    }
    words.push_back(cmdstring);

    if( int(words.size()) > 0)
    {
        if(words[0] == "light")
        {
            if( int(words.size()) == 5)
            {
                if(words[1] == "a")
                {
                    //x,y,z = constant, linear, quadratic
                    vector3df attenuation;
                    attenuation.X = atof(words[2].c_str());
                    attenuation.Y = atof(words[3].c_str());
                    attenuation.Z = atof(words[4].c_str());

                    gptr->m_CameraLight->getLightData().Attenuation = attenuation;
                    addMessage("Setting light attenuation:");
                    std::stringstream attss;
                    attss << words[2] << "," << words[3] << "," << words[4];
                    addMessage(attss.str());
                }

            }
            else if( int(words.size()) == 3)
            {
                if(words[1] == "r")
                {
                    float lradius = atof(words[2].c_str());
                    gptr->m_CameraLight->setRadius(lradius);
                    std::stringstream lradss;
                    lradss << "Setting light radius = " << lradius;
                    addMessage(lradss.str());
                }
            }
        }
    }
}
