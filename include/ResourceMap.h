#ifndef RESOURCE_MAP_H__
#define RESOURCE_MAP_H__

#include <string>
#include <map>
#include "../TinyJson/TinyJson.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
class Graphics;
struct ResouceMap;

typedef ResouceMap* ResouceMapPtr;

struct ResouceMap:private std::map<std::string,uint32_t>
{
    ResouceMap(const tinyjson::JsonValue &root,eui::Graphics* pGraphics);
    uint32_t get(const std::string& pName)const;
    void set(const std::string& pName,uint32_t pRes);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif
