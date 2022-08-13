#include "GLDiagnostics.h"
#include "GLIncludes.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string gCurrentShaderName;

/**
 * @brief For debug and when verbose mode is on will display errors coming from GLES.
 * In debug and verbose is false, will say the error once.
 * In release code is not included as checking errors all the time can stall the pipeline.
 * @param pSource_file_name 
 * @param pLine_number 
 */
void ReadOGLErrors(const char *pSource_file_name,int pLine_number)
{
	int gl_error_code = glGetError();
	if( gl_error_code == GL_NO_ERROR )
	{
		return;
	}

#ifdef VERBOSE_SHADER_BUILD
	if( gCurrentShaderName.size() )
	{
		VERBOSE_SHADER_MESSAGE("Current shader: " << gCurrentShaderName );
	}
	else
	{
		VERBOSE_SHADER_MESSAGE("No shader selected: " << gCurrentShaderName );
	}
#endif
	std:: cerr << "\n**********************\nline " << pLine_number << " file " << pSource_file_name << "\n";
	while(gl_error_code != GL_NO_ERROR)
	{
		std:: cerr << "GL error[%d]: :" << gl_error_code;
		switch(gl_error_code)
		{
		default:
			std:: cerr << "Unknown OGL error code\n";
			break;

		case GL_INVALID_ENUM:
			std:: cerr << "An unacceptable value is specified for an enumerated argument. The offending command is ignored, having no side effect other than to set the error flag.\n";
			break;

		case GL_INVALID_VALUE:
			std:: cerr << "A numeric argument is out of range. The offending command is ignored, having no side effect other than to set the error flag.\n";
			break;

		case GL_INVALID_OPERATION:
			std:: cerr << "The specified operation is not allowed in the current state. The offending command is ignored, having no side effect other than to set the error flag.\n";
			break;

		case GL_OUT_OF_MEMORY:
			std:: cerr << "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.\n";
			break;
		}
		//Get next error.
		int last = gl_error_code;
		gl_error_code = glGetError();
		if( last == gl_error_code )
			break;
	}

	std:: cerr << "**********************\n";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{
