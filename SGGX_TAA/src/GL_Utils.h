#pragma once
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
        __debugbreak(); // Only works on msvc
        std::cout << "GL Message Callback Low Severity: " << m << std::endl;
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        __debugbreak(); // Only works on msvc
        std::cout << "GL Message Callback Medium Severity: " << m << std::endl;
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        __debugbreak(); // Only works on msvc
        std::cout << "GL Message Callback High Severity: " << m << std::endl;
        break;
    }
}

void inline registerGLCallBack() {
    glDebugMessageCallback(callBackFunc, NULL);
}

