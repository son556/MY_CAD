#include "Program.h"
#include <GL/glew.h>
#include <iostream>

Program::Program()
{
    _programHandle = glCreateProgram();
}

Program::Program(Program&& program) noexcept
{
    _programHandle = program._programHandle;
    program._programHandle = 0;
}

void Program::AttachShader(unsigned int shaderHandle)
{
    glAttachShader(_programHandle, shaderHandle);
}

void Program::Link()
{
    glLinkProgram(_programHandle);

    int success;
    char infoLog[512];

    glGetProgramiv(_programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(_programHandle, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FALIED\n" << infoLog << std::endl;
    }
}




