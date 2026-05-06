#pragma once
class Program
{
public:
    Program();
    Program(Program&& program) noexcept;
    ~Program() = default;
    void AttachShader(unsigned int shaderHandle);
    void Link();
    unsigned int GetProgram() const { return _programHandle; }
private:
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
    unsigned int _programHandle;
};

