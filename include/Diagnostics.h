#ifndef Diagnostics_H__
#define Diagnostics_H__

#include <iostream>
#include <vector>
#include <string>

#ifdef VERBOSE_BUILD
	#define VERBOSE_MESSAGE(THE_MESSAGE__)	{std::clog << __LINE__ << ":" << THE_MESSAGE__ << "\n";}
#else
	#define VERBOSE_MESSAGE(THE_MESSAGE__)
#endif

#define THROW_MEANINGFUL_EXCEPTION(THE_MESSAGE__)	{throw std::runtime_error("At: " + std::to_string(__LINE__) + " In " + std::string(__FILE__) + " : " + std::string(THE_MESSAGE__));}

#define LOG_FUNCTION_PROGRESS   {std::clog << __PRETTY_FUNCTION__ << ":" << __LINE__ << "\n";}

inline std::string GetClassName(const std::string& prettyFunction)
{
    std::vector<std::string> res;
    for (size_t p = 0, q = 0; p != prettyFunction.npos; p = q)
	{
		const std::string part(prettyFunction.substr(p + (p != 0), (q = prettyFunction.find("::", p + 1)) - p - (p != 0)));
		if( part.size() > 0 )
		{
	        res.push_back(part);
		}
	}

    if( res.size() == 0 )
        return "::";

    if( res.size() == 1 )
        return res[0];

    return res[1];
}

#define SET_DEFAULT_ID()		{SetID(GetClassName(__PRETTY_FUNCTION__) + ":" + std::to_string((uint64_t(this))));}
// 
#endif //#ifndef Diagnostics_H__
