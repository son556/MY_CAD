#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

Model::Model(const std::string& filePath)
{
    constexpr float floatMax = std::numeric_limits<float>().max();
    constexpr float floatMin = std::numeric_limits<float>().min();
    glm::vec3 max = {floatMin, floatMin, floatMin};
    glm::vec3 min = {floatMax, floatMax, floatMax};

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenNormals);

    if (!scene || !scene->mMeshes[0])
    {
        std::cout << "Failed to load model: " << importer.GetErrorString() << std::endl;
        return;
    }

    aiMesh* mesh = scene->mMeshes[0];

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        glm::vec3 pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        max = glm::max(max, pos);
        min = glm::min(min, pos);
        _vertices.push_back(mesh->mVertices[i].x);
        _vertices.push_back(mesh->mVertices[i].y);
        _vertices.push_back(mesh->mVertices[i].z);
        _vertices.push_back(mesh->mNormals[i].x);
        _vertices.push_back(mesh->mNormals[i].y);
        _vertices.push_back(mesh->mNormals[i].z);
    }

    _center = (max + min) * 0.5f;
    _dist = std::max(std::max(max.x - min.x, max.y - min.y), max.z - min.z);

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            _indices.push_back(face.mIndices[j]);
    }
}

Model::Model(Model&& model) noexcept
{
    _vertices = std::move(model._vertices);
    _indices = std::move(model._indices);
}