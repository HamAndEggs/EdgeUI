#ifndef Diagnostics_H__
#define Diagnostics_H__

#include <iostream>

#ifdef VERBOSE_BUILD
	#define VERBOSE_MESSAGE(THE_MESSAGE__)	{std::clog << __LINE__ << ":" << THE_MESSAGE__ << "\n";}
#else
	#define VERBOSE_MESSAGE(THE_MESSAGE__)
#endif

#define THROW_MEANINGFUL_EXCEPTION(THE_MESSAGE__)	{throw std::runtime_error("At: " + std::to_string(__LINE__) + " In " + std::string(__FILE__) + " : " + std::string(THE_MESSAGE__));}

#define LOG_FUNCTION_PROGRESS   {std::clog << __PRETTY_FUNCTION__ << ":" << __LINE__ << "\n";}

inline std::string GetClassName(const std::string& prettyFunction)
{
    size_t colons = prettyFunction.find_last_of("::");
    if (colons == std::string::npos)
        return "::";
    size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
    size_t end = colons - begin;

    return prettyFunction.substr(begin,end);
}

#define SET_DEFAULT_ID()		{SetID(GetClassName(__PRETTY_FUNCTION__) + ":" + std::to_string((uint64_t(this))));}
// 
#endif //#ifndef Diagnostics_H__
