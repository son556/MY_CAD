#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class OpenGLInit
{
public:
    static bool Init(GLFWwindow*& window);

private:
    OpenGLInit() = delete;
    ~OpenGLInit() = delete;
    OpenGLInit(const OpenGLInit&) = delete;
    OpenGLInit& operator=(const OpenGLInit&) = delete;
};

