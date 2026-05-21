#pragma once

#include <vector>
#include <map>
#include <cstddef>
#include <GLM/glm.hpp>

constexpr size_t INVALID_INDEX = static_cast<size_t>(-1);

struct edge_t
{
    size_t start = INVALID_INDEX;
    size_t end = INVALID_INDEX;
};

struct triangle_t
{
    size_t vertices[3] = { INVALID_INDEX, INVALID_INDEX, INVALID_INDEX };
};


class HalfEdge
{
public:
    struct halfedge_t
    {
        size_t toVertex = INVALID_INDEX;
        size_t faceIndex = INVALID_INDEX;
        size_t edgeIndex = INVALID_INDEX;
        size_t oppositeHalfEdge = INVALID_INDEX;
        size_t nextHlafEdge = INVALID_INDEX;
    };

    bool CheckValidManifold(const std::vector<size_t>& indices);
    void preProcess(
        triangle_t* triangles,
        size_t triangleSize,
        std::vector<edge_t>& outEdges
    );

    void Build(
        const size_t numVertices,
        const size_t numTriangles,
        const triangle_t* triangles,
        const size_t numEdges,
        const edge_t* edges
    );

    void GetBoundaryVertices(size_t boundaryVertexIndex, std::vector<size_t>& outBoundaryVertexIndices) const;
    void GetVertexIndicesOneRing(size_t vertexIndex, std::vector<size_t>& outRingVertexIndices) const;
    void GetFaceVertices(size_t faceIndex, std::vector<size_t>& outFaceVertexIndices) const;
    void GetFaceIndicesOneRing(size_t vertexIndex, std::vector<size_t>& outFaceIndices) const;
    void GetAdjFaceIndices(size_t faceIndex, std::vector<size_t>& outFaceIndices) const;
    glm::vec3 GetVertexNormal(size_t vertexIndex) const;
    glm::vec3 GetFaceNormal(size_t faceIndex) const;

    inline bool IsBoundaryVertex(size_t vertexIndex) const
    {
        size_t hei = _vertexHalfEdges[vertexIndex];
        return (_halfEdges[hei].faceIndex == INVALID_INDEX || _halfEdges[_halfEdges[hei].oppositeHalfEdge].faceIndex == INVALID_INDEX);
    }

    inline bool IsBoundaryEdge(size_t edgeIndex) const
    {
        size_t hei = _edgeHalfEdges[edgeIndex];
        return (_halfEdges[hei].faceIndex == INVALID_INDEX || _halfEdges[_halfEdges[hei].oppositeHalfEdge].faceIndex == INVALID_INDEX);
    }

    void SplitEdge(size_t edgeIdx, float t);

private:
    void Clear();
    glm::vec3 GetFaceNormalNotNormalized(size_t faceIndex) const;

    std::vector<glm::vec3> _points;
    std::vector<size_t> _vertexHalfEdges;
    std::vector<size_t> _faceHalfEdges;
    std::vector<size_t> _edgeHalfEdges;
    std::vector<halfedge_t> _halfEdges;
    std::map<std::pair<size_t, size_t>, size_t> _edgeToHalfEdge;
};
