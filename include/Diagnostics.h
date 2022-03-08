#ifndef Diagnostics_H__
#define Diagnostics_H__

#include <iostream>

#ifdef VERBOSE_BUILD
	#define VERBOSE_MESSAGE(THE_MESSAGE__)	{std::clog << __LINE__ << ":" << THE_MESSAGE__ << "\n";}
#else
	#define VERBOSE_MESSAGE(THE_MESSAGE__)
#endif

#define THROW_MEANINGFUL_EXCEPTION(THE_MESSAGE__)	{throw std::runtime_error("At: " + std::to_string(__LINE__) + " In " + std::string(__FILE__) + " : " + std::string(THE_MESSAGE__));}


#endif //#ifndef Diagnostics_H__
