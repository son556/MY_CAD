#pragma once

#include <vector>
#include <string>
#include <GLM/glm.hpp>

class Model
{
public:
    Model(const std::string& filePath);
    Model(Model&& model) noexcept;
    ~Model() = default;

    const std::vector<float>& GetVertices() const { return _vertices; }
    const std::vector<unsigned int>& GetIndices() const { return _indices; }
    const float GetDist() const { return _dist; }
    const glm::vec3& GetCenter() const { return _center; }

private:
    std::vector<float> _vertices;
    std::vector<unsigned int> _indices;
    glm::vec3 _center;
    float _dist;
};