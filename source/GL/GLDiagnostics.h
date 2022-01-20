#ifndef GLDiagnostics_H__
#define GLDiagnostics_H__

#include "Diagnostics.h"

#include <string>
#include <iostream>

#ifdef DEBUG_BUILD
	#define CHECK_OGL_ERRORS()	ReadOGLErrors(__FILE__,__LINE__)
#else
	#define CHECK_OGL_ERRORS()
#endif

#ifdef VERBOSE_SHADER_BUILD
	#define VERBOSE_SHADER_MESSAGE(THE_MESSAGE__)	{std::clog << "Shader: " << THE_MESSAGE__ << "\n";}
#else
	#define VERBOSE_SHADER_MESSAGE(THE_MESSAGE__)
#endif


namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief For debug and when verbose mode is on will display errors coming from GLES.
 * In debug and verbose is false, will say the error once.
 * In release code is not included as checking errors all the time can stall the pipeline.
 * @param pSource_file_name 
 * @param pLine_number 
 */
void ReadOGLErrors(const char *pSource_file_name,int pLine_number);

#ifdef VERBOSE_SHADER_BUILD
extern std::string gCurrentShaderName;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{


#endif //#ifndef GLDiagnostics_H__