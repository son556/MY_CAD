#include "Shader.h"
#include "ReadSource.h"
#include <iostream>

Shader::Shader(const std::string& shaderFilePath, GLenum syaderType)
{
    std::string src;
    ReadSource::Read(src, shaderFilePath);

    const char* srcData = src.c_str();

    _shaderHandle = glCreateShader(syaderType);
    glShaderSource(_shaderHandle, 1, &srcData, nullptr);
    glCompileShader(_shaderHandle);

    int success;
    char infoLog[512];
    glGetShaderiv(_shaderHandle, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(_shaderHandle, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FALIED\n" << infoLog << std::endl;
    }
}

Shader::Shader(Shader&& shader) noexcept
{
    _shaderHandle = shader._shaderHandle;
    shader._shaderHandle = 0;
}
