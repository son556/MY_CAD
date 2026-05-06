#pragma once
#include <string>
#include "GL/glew.h"

class Shader
{
public:
    Shader(const std::string& shaderFilePath, GLenum syaderType);
    Shader(Shader&& shader) noexcept;
    ~Shader() = default;
    unsigned int GetShader() const { return _shaderHandle; }

private:
    Shader() = delete;
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    unsigned int _shaderHandle;
};

