#include "ResourceMap.h"
#include "Diagnostics.h"
#include "Graphics.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
ResouceMap::ResouceMap(const tinyjson::JsonValue &root,eui::Graphics* pGraphics)
{
    // We have to do this first in it's own loop as tinyjson does not guarentee child elements order in elements.
    // Because it uses a std::map for speed.
    for(const auto &child : root)
    {
    // First check for the properties we know about, and then scan for child objects.
        if( child.first == "resource" )
        {
            for(const auto &res : child.second)
            {
                if( res.second == tinyjson::JsonValueType::OBJECT )
                {
                    const std::string name = res.first;
                    const auto &obj = res.second;
                    VERBOSE_MESSAGE("Loading resource:" + name);
                    if( obj.HasValue("font") )
                    {
                        VERBOSE_MESSAGE("Font resource " << obj["font"].GetString() << " size " << obj["size"].GetUInt32());
                        this->set(name,pGraphics->FontLoad(obj["font"],obj["size"]));
                    }
                    else if( obj.HasValue("texture") )
                    {
                        VERBOSE_MESSAGE("Texture resource " << obj["texture"].GetString());
                        this->set(name,pGraphics->TextureLoad(obj["texture"]));
                    }
                }
            }
        }
    }
}

uint32_t ResouceMap::get(const std::string& pName)const
{
    const auto& res = this->find(pName);
    if( res == this->end() )
    {
        THROW_MEANINGFUL_EXCEPTION("Resource name not found: " + pName);
    }
    return res->second;
}

void ResouceMap::set(const std::string& pName,uint32_t pRes)
{
    if( this->insert(ResouceMap::value_type(pName,pRes)).second == false )
    {
        THROW_MEANINGFUL_EXCEPTION("Resource name already used: " + pName);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

