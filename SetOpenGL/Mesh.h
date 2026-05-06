#pragma once

#include <vector>

class Mesh
{
public:
    Mesh(const float* vertices, size_t verticesByteSize, const unsigned int* indices, size_t indicesByteSize);
    ~Mesh() = default;

    unsigned int GetVAO() const { return _VAO; }

private:
    std::vector<float> _vertices;
    std::vector<unsigned int> _indices;
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;

    Mesh() = delete;
};