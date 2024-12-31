#ifndef DataBinding_h__
#define DataBinding_h__

#include <string>
#include <map>
#include <functional>
#include <cstdarg>

#include "TinyTools.h"


namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
enum BINDING_TYPE
{
    BINDING_TYPE_BOOL,
    BINDING_TYPE_INT,
    BINDING_TYPE_UNSIGNED_INT,
    BINDING_TYPE_FLOAT,
    BINDING_TYPE_DOUBLE,
    BINDING_TYPE_STRING,
};

#define DATA_BINDING_TYPES  \
    ADD_BINDING_TYPE(bool,BINDING_TYPE_BOOL) \
    ADD_BINDING_TYPE(int,BINDING_TYPE_INT) \
    ADD_BINDING_TYPE(unsigned int,BINDING_TYPE_UNSIGNED_INT) \
    ADD_BINDING_TYPE(float,BINDING_TYPE_FLOAT) \
    ADD_BINDING_TYPE(double,BINDING_TYPE_DOUBLE) \
    ADD_BINDING_TYPE(std::string,BINDING_TYPE_STRING)



class BoundVar
{
public:
    const std::string ToString()
    {
        #define ADD_BINDING_TYPE(TYPE__,TYPE__NAME) if( type == TYPE__NAME ){return std::to_string(value.TYPE__NAME);}
            DATA_BINDING_TYPES
        #undef ADD_BINDING_TYPE
        return "UNKNOWN BINDING TYPE";
    }

    #define ADD_BINDING_TYPE(TYPE__,TYPE__NAME) const TYPE__ & Set(const TYPE__ &n){value.TYPE__NAME = n;return n;}
        DATA_BINDING_TYPES
    #undef ADD_BINDING_TYPE

private:
    BINDING_TYPE type;
    struct BindingValue
    {
        #define ADD_BINDING_TYPE(TYPE__,TYPE__NAME) TYPE__ TYPE__NAME;
            DATA_BINDING_TYPES
        #undef ADD_BINDING_TYPE
    }value;
    
};
*/

class DataBinding
{
public:
    DataBinding(){};
    ~DataBinding(){};

    /**
     * Looks for any binding vars in the string, and if found substitues them with the stored value.
     */
    std::string Substitute(const std::string& pString)
    {
        const tinytools::StringVec words = tinytools::string::SplitString(pString," ");
        tinytools::StringVec bindings;
        for( const std::string& w : words )
        {
            size_t s = w.find("{{");
            size_t e = w.rfind("}}");
            if( s == 0 && s < e && e < w.length() )
            {
                bindings.push_back(w.substr(s+2,e-s-2));
            }
        }

        std::string newString(pString);
        for( const std::string& b : bindings )
        {
            std::string v = Find(b);
            newString = tinytools::string::ReplaceString(newString,"{{" + b + "}}",v);
        }
        return newString;
    }

    const std::string& Find(const std::string& pBinding)
    {
        auto found = mBindings.find(pBinding);
        if( found != mBindings.end() )
        {
            return found->second;
        }
        return pBinding;
    }

    void Set(const std::string& pBinding,const std::string_view& pValue)
    {
        mBindings[pBinding] = pValue;
    }

    void Set(const std::string& pBinding,const char* pFmt,...)
    {
        char buf[1024];
        va_list args;
        va_start(args, pFmt);
        vsnprintf(buf, sizeof(buf), pFmt, args);
        va_end(args);
        Set(pBinding,std::string_view(buf));
    }
/*
    template <class THE_TYPE>void Set(const std::string& pBinding,THE_TYPE pNew)
    {
        mBindings[pBinding].Set(pNew);
    }
*/
private:
    std::map<std::string,std::string>mBindings;
};

/*
class BoundVar
{
public:

protected:
    bool ValueChange(bool pOld,bool pNew)
    {
        if( mBoolChangeEvent != nullptr )
        {
            return mBoolChangeEvent(pOld,pNew);
        }
        return true;
    }

private:
    std::function<bool(bool pOld,bool pNew)> mBoolChangeEvent = nullptr;

};

template <class BINDING_TYPE>class Var : public BoundVar
{
public:
    Var(){}
    Var(const BINDING_TYPE& pValue):value(pValue){}

    operator BINDING_TYPE()const{return value;}
    BINDING_TYPE operator = (const BINDING_TYPE& pNew)
    {
        if( ValueChange(value,pNew) )
        {
            value = pNew;
        }
        return pNew;
    }


private:
    BINDING_TYPE value;
};
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif //#ifndef DataBinding_h__