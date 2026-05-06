#pragma once

#include <vector>
#include <map>
#include <GLM/glm.hpp>

struct edge_t
{
    int start = -1;
    int end = -1;
};

struct triangle_t
{
    int vertices[3] = { -1, -1, -1 };
};


class HalfEdge
{
public:
    struct halfedge_t
    {
        int toVertex = -1;
        int faceIndex = -1;
        int edgeIndex = -1;
        int oppositeHalfEdge = -1;
        int nextHlafEdge = -1;
    };

    bool CheckValidManifold(const std::vector<int>& indices);
    void preProcess(
        triangle_t* triangles,
        int triangleSize,
        std::vector<edge_t>& outEdges
    );

    void Build(
        const int numVertices,
        const int numTriangles,
        const triangle_t* triangles,
        const int numEdges,
        const edge_t* edges
    );

    void GetBoundaryVertices(int boundaryVertexIndex, std::vector<int>& outBoundaryVertexIndices) const;
    void GetVertexIndicesOneRing(int vertexIndex, std::vector<int>& outRingVertexIndices) const;
    void GetFaceVertices(int faceIndex, std::vector<int>& outFaceVertexIndices) const;
    void GetFaceIndicesOneRing(int vertexIndex, std::vector<int>& outFaceIndices) const;
    void GetAdjFaceIndices(int faceIndex, std::vector<int>& outFaceIndices) const;
    glm::vec3 GetVertexNormal(int vertexIndex) const;
    glm::vec3 GetFaceNormal(int faceIndex) const;

    inline bool IsBoundaryVertex(int vertexIndex) const
    {
        int hei = _vertexHalfEdges[vertexIndex];
        return (_halfEdges[hei].faceIndex == -1 || _halfEdges[_halfEdges[hei].oppositeHalfEdge].faceIndex == -1);
    }

    inline bool IsBoundaryEdge(int edgeIndex) const
    {
        int hei = _edgeHalfEdges[edgeIndex];
        return (_halfEdges[hei].faceIndex == -1 || _halfEdges[_halfEdges[hei].oppositeHalfEdge].faceIndex == -1);
    }

private:
    void Clear();
    glm::vec3 GetFaceNormalNotNormalized(int faceIndex) const;

    std::vector<glm::vec3> _points;
    std::vector<int> _vertexHalfEdges;
    std::vector<int> _faceHalfEdges;
    std::vector<int> _edgeHalfEdges;
    std::vector<halfedge_t> _halfEdges;
    std::map<std::pair<int, int>, int> _edgeToHalfEdge;
};
