#pragma once

#define _BREAK_POINT  
#ifdef _WIN32
#define BREAK_POINT __debugbreak();
#elif defined(unix) || defined(__unix__) || defined(__unix)
# define BREAK_POINT 
#endif

#include <GL/glew.h>
#include <iostream>

static void inline GLClearError()
{
	while (glGetError()/* != GL_NO_ERROR */);
}

static void inline GLPrintError() {
	GLenum error;
	while (error = glGetError()/* != GL_NO_ERROR */) 
	{
		std::cout << "OpenGL Error: " << error << std::endl;
	}
}

void inline GLAPIENTRY callBackFunc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    std::string m(message, length);
    switch (severity) {
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        //std::cout << "GL Message Callback Notification: " << m << std::endl;
        break;
    case GL_DEBUG_SEVERITY_LOW:
        _BREAK_POINT
        std::cout << "GL Message Callback Low Severity: " << m << std::endl;
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        _BREAK_POINT
        std::cout << "GL Message Callback Medium Severity: " << m << std::endl;
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        _BREAK_POINT
        std::cout << "GL Message Callback High Severity: " << m << std::endl;
        break;
    }
}

void inline registerGLCallBack() {
    glDebugMessageCallback(callBackFunc, NULL);
}

