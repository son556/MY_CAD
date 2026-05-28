#include "HalfEdge.h"
#include <map>
#include <set>
#include <iostream>

bool HalfEdge::CheckValidManifold(const std::vector<int>& indices)
{
    return false;
}

void HalfEdge::preProcess(triangle_t* triangles, int triangleSize, std::vector<edge_t>& outEdges)
{
    outEdges.clear();
    outEdges.reserve(triangleSize * 3);

    for (int i = 0; i < triangleSize; ++i)
    {
        edge_t edge;

        edge.start = triangles[i].vertices[0];
        edge.end = triangles[i].vertices[1];
        outEdges.push_back(edge);

        edge.start = triangles[i].vertices[1];
        edge.end = triangles[i].vertices[2];
        outEdges.push_back(edge);

        edge.start = triangles[i].vertices[2];
        edge.end = triangles[i].vertices[0];
        outEdges.push_back(edge);
    }
}


int FindFaceIdByEdge(int v0Id, int v1Id, const std::map<std::pair<int, int>, int>& book)
{
    std::map<std::pair<int, int>, int>::const_iterator it = book.find({ v0Id, v1Id });
    if (it == book.end())
        return -1;
    else
        return it->second;
}

void HalfEdge::Build(
    const int numVertices,
    const int numTriangles,
    const triangle_t* triangles,
    const int numEdges,
    const edge_t* edges,
    const std::vector<glm::vec3>& points
)
{
    for (const auto& point : points)
        _points.push_back(point);

    std::map<std::pair<int, int>, int> edgeAndFaceBook;

    for (int faceIndex = 0; faceIndex < numTriangles; ++faceIndex)
    {
        const triangle_t& tri = triangles[faceIndex];
        edgeAndFaceBook[{tri.vertices[0], tri.vertices[1]}] = faceIndex;
        edgeAndFaceBook[{tri.vertices[1], tri.vertices[2]}] = faceIndex;
        edgeAndFaceBook[{tri.vertices[2], tri.vertices[0]}] = faceIndex;
    }

    Clear();

    _vertexHalfEdges.resize(numVertices, -1);
    _edgeHalfEdges.resize(numEdges, -1);
    _faceHalfEdges.resize(numTriangles, -1);
    _halfEdges.reserve(numEdges * 2);

    for (int ei = 0; ei < numEdges; ++ei)
    {
        const edge_t& edge = edges[ei];

        const int h0Index = (int)_halfEdges.size();
        _halfEdges.push_back(halfedge_t());
        halfedge_t& he0 = _halfEdges.back();

        const int h1Index = (int)_halfEdges.size();
        _halfEdges.push_back(halfedge_t());
        halfedge_t& he1 = _halfEdges.back();

        he0.faceIndex = FindFaceIdByEdge(edge.start, edge.end, edgeAndFaceBook);
        he0.toVertex = edge.end;
        he0.edgeIndex = ei;

        he1.faceIndex = FindFaceIdByEdge(edge.end, edge.start, edgeAndFaceBook);
        he1.toVertex = edge.start;
        he1.edgeIndex = ei;

        he0.oppositeHalfEdge = h1Index;
        he1.oppositeHalfEdge = h0Index;

        _edgeToHalfEdge[{edge.start, edge.end}] = h0Index;
        _edgeToHalfEdge[{edge.end, edge.start}] = h1Index;

        if (_vertexHalfEdges[he0.toVertex] == -1 || he1.faceIndex == -1)
            _vertexHalfEdges[he0.toVertex] = he0.oppositeHalfEdge;

        if (_vertexHalfEdges[he1.toVertex] == -1 || he0.faceIndex == -1)
            _vertexHalfEdges[he1.toVertex] = he1.oppositeHalfEdge;

        if (he0.faceIndex != -1 && _faceHalfEdges[he0.faceIndex] == -1)
            _faceHalfEdges[he0.faceIndex] = h0Index;

        if (he1.faceIndex != -1 && _faceHalfEdges[he1.faceIndex] == -1)
            _faceHalfEdges[he1.faceIndex] = h1Index;

        _edgeHalfEdges[ei] = h0Index;
    }

    std::vector<int> boundaryHalfEdges;
    for (int hei = 0; hei < (int)_halfEdges.size(); ++hei)
    {
        halfedge_t& he = _halfEdges[hei];
        if (he.faceIndex == -1)
        {
            boundaryHalfEdges.push_back(hei);
            continue;
        }

        const triangle_t& face = triangles[he.faceIndex];
        const int vi = he.toVertex;

        int vj = -1;
        if (face.vertices[0] == vi) vj = face.vertices[1];
        else if (face.vertices[1] == vi) vj = face.vertices[2];
        else if (face.vertices[2] == vi) vj = face.vertices[0];

        he.nextHlafEdge = _edgeToHalfEdge[{vi, vj}];
    }

    std::map<int, std::set<int>> vertex2OutgoingBoundaryHalfEdgeIndices;
    for (std::vector<int>::const_iterator hei = boundaryHalfEdges.begin(); hei != boundaryHalfEdges.end(); ++hei)
    {
        const int originateVertexIndex = _halfEdges[_halfEdges[*hei].oppositeHalfEdge].toVertex;
        vertex2OutgoingBoundaryHalfEdgeIndices[originateVertexIndex].insert(*hei);
        if (vertex2OutgoingBoundaryHalfEdgeIndices[originateVertexIndex].size() > 1)
        {
            std::cerr << "Butterfly vertex encountered.\n";
        }
    }

    for (std::vector<int>::const_iterator hei = boundaryHalfEdges.begin(); hei != boundaryHalfEdges.end(); ++hei)
    {
        halfedge_t& he = _halfEdges[*hei];

        std::set<int>& outgoing = vertex2OutgoingBoundaryHalfEdgeIndices[he.toVertex];
        if (outgoing.empty() == false)
        {
            std::set<int>::iterator outgoingIter = outgoing.begin();
            he.nextHlafEdge = *outgoingIter;
            outgoing.erase(outgoingIter);
        }
    }
}

void HalfEdge::GetBoundaryVertices(int boundaryVertexIndex, std::vector<int>& outBoundaryVertexIndices) const
{
    int hei = _vertexHalfEdges[boundaryVertexIndex];
    if (IsBoundaryVertex(boundaryVertexIndex) == false)
        return;

    if (_halfEdges[hei].faceIndex != -1)
        hei = _halfEdges[hei].oppositeHalfEdge;

    int start = hei;
    outBoundaryVertexIndices.push_back(boundaryVertexIndex);

    hei = _halfEdges[hei].nextHlafEdge;

    while (hei != start)
    {
        outBoundaryVertexIndices.push_back(_halfEdges[_halfEdges[hei].oppositeHalfEdge].toVertex);
        hei = _halfEdges[hei].nextHlafEdge;
    }
}

void HalfEdge::GetVertexIndicesOneRing(int vertexIndex, std::vector<int>& outRingVertexIndices) const
{
    int hei = _vertexHalfEdges[vertexIndex];
    int endVertex = _halfEdges[hei].toVertex;
    outRingVertexIndices.push_back(endVertex);
    hei = _halfEdges[_halfEdges[hei].oppositeHalfEdge].nextHlafEdge;

    while (_halfEdges[hei].toVertex != endVertex)
    {
        outRingVertexIndices.push_back(_halfEdges[hei].toVertex);
        hei = _halfEdges[_halfEdges[hei].oppositeHalfEdge].nextHlafEdge;
    }
}

void HalfEdge::GetFaceVertices(int faceIndex, std::vector<int>& outFaceVertexIndices) const
{
    if (faceIndex == -1)
        return;

    int hei = _faceHalfEdges[faceIndex];
    int endVertexIndex = _halfEdges[_halfEdges[hei].oppositeHalfEdge].toVertex;
    int vertexIndex = _halfEdges[hei].toVertex;

    outFaceVertexIndices.push_back(endVertexIndex);
    while (vertexIndex != endVertexIndex)
    {
        outFaceVertexIndices.push_back(vertexIndex);
        hei = _halfEdges[hei].nextHlafEdge;
        vertexIndex = _halfEdges[hei].toVertex;
    }
}

void HalfEdge::GetFaceIndicesOneRing(int vertexIndex, std::vector<int>& outFaceIndices) const
{
    int hei = _vertexHalfEdges[vertexIndex];
    int end = _halfEdges[hei].toVertex;

    if (_halfEdges[hei].faceIndex != -1)
        outFaceIndices.push_back(_halfEdges[hei].faceIndex);

    hei = _halfEdges[_halfEdges[hei].oppositeHalfEdge].nextHlafEdge;
    while (_halfEdges[hei].toVertex != end)
    {
        if (_halfEdges[hei].faceIndex != -1)
            outFaceIndices.push_back(_halfEdges[hei].faceIndex);
        hei = _halfEdges[_halfEdges[hei].oppositeHalfEdge].nextHlafEdge;
    }
}

void HalfEdge::GetAdjFaceIndices(int faceIndex, std::vector<int>& outFaceIndices) const
{
    if (faceIndex == -1) return;

    int hei = _faceHalfEdges[faceIndex];
    int end = _halfEdges[_halfEdges[hei].oppositeHalfEdge].toVertex;

    do
    {
        int fidx = _halfEdges[_halfEdges[hei].oppositeHalfEdge].faceIndex;
        if (fidx != -1)
            outFaceIndices.push_back(fidx);
        hei = _halfEdges[hei].nextHlafEdge;
    } while (_halfEdges[_halfEdges[hei].oppositeHalfEdge].toVertex != end);
}


glm::vec3 HalfEdge::GetVertexNormal(int vertexIndex) const
{
    std::vector<int> faceIndices;
    glm::vec3 normal = glm::vec3(0, 0, 0);

    GetFaceIndicesOneRing(vertexIndex, faceIndices);

    for (int i = 0; i < (int)faceIndices.size(); ++i)
    {
        normal += GetFaceNormalNotNormalized(faceIndices[i]);
    }

    return glm::normalize(normal);
}

glm::vec3 HalfEdge::GetFaceNormal(int faceIndex) const
{
    if (faceIndex == -1) return glm::vec3(0);

    std::vector<int> vertices;
    GetFaceVertices(faceIndex, vertices);

    glm::vec3 e0 = _points[vertices[1]] - _points[vertices[0]];
    glm::vec3 e1 = _points[vertices[2]] - _points[vertices[0]];
    glm::vec3 normal = glm::cross(e0, e1);
    return glm::normalize(normal);
}


void HalfEdge::SplitEdge(int edgeIdx, float t)
{
    _halfEdges.reserve(_halfEdges.size() + 6);
    _edgeHalfEdges.reserve(_edgeHalfEdges.size() + 3);
    _faceHalfEdges.reserve(_faceHalfEdges.size() + 2);

    halfedge_t& halfEdgeBtoC = _halfEdges[_edgeHalfEdges[edgeIdx]];
    halfedge_t& halfEdgeCtoB = _halfEdges[halfEdgeBtoC.oppositeHalfEdge];

    int nextCtoBHalfEdgeIdx = halfEdgeCtoB.nextHlafEdge;

    int halfEdgesSize = (int)_halfEdges.size();
    int edgeHalfEdgesSize = (int)_edgeHalfEdges.size();
    int faceHalfEdgesSize = (int)_faceHalfEdges.size();
    int vertexHalfEdgesSize = (int)_vertexHalfEdges.size();

    halfedge_t* halfEdgeAtoB = nullptr;
    halfedge_t* halfEdgeCtoA = nullptr;
    halfedge_t* halfEdgeBtoD = nullptr;
    halfedge_t* halfEdgeDtoC = nullptr;
    halfedge_t newHalfEdgeBtoM;
    halfedge_t newHalfEdgeMtoB;
    halfedge_t newHalfEdgeMtoA;
    halfedge_t newHalfEdgeAtoM;
    halfedge_t& halfEdgeMtoC = halfEdgeBtoC;
    halfedge_t& halfEdgeCtoM = halfEdgeCtoB;

    _points.push_back(t * _points[halfEdgeCtoB.toVertex] + (1 - t) * _points[halfEdgeBtoC.toVertex]);
    _vertexHalfEdges.push_back(_edgeHalfEdges[edgeIdx]);

    bool aFlag = false;
    if (halfEdgeBtoC.faceIndex != -1)
    {
        aFlag = true;
        halfEdgeCtoA = &(_halfEdges[halfEdgeBtoC.nextHlafEdge]);
        halfEdgeAtoB = &(_halfEdges[halfEdgeCtoA->nextHlafEdge]);

        // face
        _faceHalfEdges[halfEdgeBtoC.faceIndex] = halfEdgeBtoC.nextHlafEdge;
        _faceHalfEdges.push_back(halfEdgeCtoA->nextHlafEdge);

        // edge
        _edgeHalfEdges[edgeIdx] = halfEdgesSize;
        _edgeHalfEdges.push_back(halfEdgesSize + 3);
        _edgeHalfEdges.push_back(halfEdgeBtoC.oppositeHalfEdge);

        // halfedge
        halfEdgeAtoB->faceIndex = faceHalfEdgesSize;
        halfEdgeAtoB->nextHlafEdge = halfEdgesSize;

        newHalfEdgeBtoM.edgeIndex = edgeIdx;
        newHalfEdgeBtoM.faceIndex = faceHalfEdgesSize;
        newHalfEdgeBtoM.nextHlafEdge = halfEdgesSize + 2;
        newHalfEdgeBtoM.oppositeHalfEdge = halfEdgesSize + 1;
        newHalfEdgeBtoM.toVertex = vertexHalfEdgesSize;

        newHalfEdgeMtoA.edgeIndex = edgeHalfEdgesSize;
        newHalfEdgeMtoA.faceIndex = faceHalfEdgesSize;
        newHalfEdgeMtoA.nextHlafEdge = halfEdgeCtoA->nextHlafEdge;
        newHalfEdgeMtoA.oppositeHalfEdge = halfEdgesSize + 3;
        newHalfEdgeMtoA.toVertex = halfEdgeCtoA->toVertex;

        newHalfEdgeAtoM.edgeIndex = edgeHalfEdgesSize;
        newHalfEdgeAtoM.faceIndex = halfEdgeBtoC.faceIndex;
        newHalfEdgeAtoM.nextHlafEdge = halfEdgeCtoB.oppositeHalfEdge;
        newHalfEdgeAtoM.oppositeHalfEdge = halfEdgesSize + 2;
        newHalfEdgeAtoM.toVertex = vertexHalfEdgesSize;

        halfEdgeCtoA->nextHlafEdge = halfEdgesSize + 3;

        halfedge_t& halfEdgeCtoM = halfEdgeCtoB;
        halfEdgeCtoM.edgeIndex = edgeHalfEdgesSize + 1;
        halfEdgeCtoM.toVertex = vertexHalfEdgesSize;

        halfedge_t& halfEdgeMtoC = halfEdgeBtoC;
        halfEdgeMtoC.edgeIndex = edgeHalfEdgesSize + 1;

        newHalfEdgeMtoB.edgeIndex = edgeIdx;
        newHalfEdgeMtoB.oppositeHalfEdge = halfEdgesSize;
        newHalfEdgeMtoB.toVertex = halfEdgeCtoB.toVertex;

        _halfEdges.push_back(newHalfEdgeBtoM);
        _halfEdges.push_back(newHalfEdgeMtoB);
        _halfEdges.push_back(newHalfEdgeMtoA);
        _halfEdges.push_back(newHalfEdgeAtoM);
    }
    else
    {
        // edge
        _edgeHalfEdges[edgeIdx] = halfEdgesSize;
        _edgeHalfEdges.push_back(halfEdgeBtoC.oppositeHalfEdge);

        // half edge
        newHalfEdgeBtoM.edgeIndex = edgeIdx;
        newHalfEdgeBtoM.faceIndex = -1;
        newHalfEdgeBtoM.oppositeHalfEdge = halfEdgesSize + 1;
        newHalfEdgeBtoM.nextHlafEdge = halfEdgeCtoB.oppositeHalfEdge;
        newHalfEdgeBtoM.toVertex = vertexHalfEdgesSize;


        newHalfEdgeMtoB.edgeIndex = edgeIdx;
        newHalfEdgeMtoB.oppositeHalfEdge = halfEdgesSize;
        newHalfEdgeMtoB.toVertex = halfEdgeCtoB.toVertex;

        halfedge_t& halfEdgeCtoM = halfEdgeCtoB;
        halfEdgeCtoM.edgeIndex = edgeHalfEdgesSize;
        halfEdgeCtoM.toVertex = vertexHalfEdgesSize;

        halfedge_t& halfEdgeMtoC = halfEdgeBtoC;
        halfEdgeMtoC.edgeIndex = edgeHalfEdgesSize;

        _halfEdges.push_back(newHalfEdgeBtoM);
        _halfEdges.push_back(newHalfEdgeMtoB);
    }

    edgeHalfEdgesSize = (int)_edgeHalfEdges.size();
    faceHalfEdgesSize = (int)_faceHalfEdges.size();
    halfEdgesSize = (int)_halfEdges.size();

    if (halfEdgeCtoB.faceIndex != -1)
    {
        int mtobIndex = aFlag ? halfEdgesSize - 3 : halfEdgesSize - 1;

        halfedge_t* halfEdgeMtoB = &(_halfEdges[mtobIndex]);

        halfedge_t& halfEdgeBtoD = _halfEdges[halfEdgeMtoB->nextHlafEdge];
        halfedge_t& halfEdgeDtoC = _halfEdges[halfEdgeBtoD.nextHlafEdge];

        // face
        _faceHalfEdges[halfEdgeCtoB.faceIndex] = _halfEdges[halfEdgeCtoB.nextHlafEdge].nextHlafEdge;
        _faceHalfEdges.push_back(halfEdgesSize);

        // edge
        _edgeHalfEdges.push_back(halfEdgesSize);

        // half edge
        halfEdgeMtoB->faceIndex = faceHalfEdgesSize;
        halfEdgeMtoB->nextHlafEdge = halfEdgeCtoB.nextHlafEdge;

        halfEdgeBtoD.faceIndex = faceHalfEdgesSize;
        halfEdgeBtoD.nextHlafEdge = halfEdgesSize;

        halfedge_t newHalfEdgeDtoM;
        newHalfEdgeDtoM.edgeIndex = edgeHalfEdgesSize;
        newHalfEdgeDtoM.faceIndex = faceHalfEdgesSize;
        newHalfEdgeDtoM.nextHlafEdge = mtobIndex;
        newHalfEdgeDtoM.oppositeHalfEdge = halfEdgesSize + 1;
        newHalfEdgeDtoM.toVertex = vertexHalfEdgesSize;

        halfedge_t newHalfEdgeMtoD;
        newHalfEdgeMtoD.edgeIndex = edgeHalfEdgesSize;
        newHalfEdgeMtoD.faceIndex = halfEdgeCtoB.faceIndex;
        newHalfEdgeMtoD.nextHlafEdge = _halfEdges[halfEdgeDtoC.oppositeHalfEdge].oppositeHalfEdge;
        newHalfEdgeMtoD.oppositeHalfEdge = halfEdgesSize;
        newHalfEdgeMtoD.toVertex = _halfEdges[_halfEdges[newHalfEdgeMtoD.nextHlafEdge].oppositeHalfEdge].toVertex;

        halfEdgeCtoM.nextHlafEdge = halfEdgesSize + 1;

        _halfEdges.push_back(newHalfEdgeDtoM);
        _halfEdges.push_back(newHalfEdgeMtoD);
    }
    else
    {
        int mtobIndex = aFlag ? halfEdgesSize - 3 : halfEdgesSize - 1;

        halfedge_t* halfEdgeMtoB = &(_halfEdges[mtobIndex]);

        halfEdgeMtoB->faceIndex = -1;
        halfEdgeMtoB->nextHlafEdge = nextCtoBHalfEdgeIdx;

        halfEdgeCtoM.faceIndex = -1;
        halfEdgeCtoM.nextHlafEdge = _halfEdges[halfEdgeMtoB->oppositeHalfEdge].oppositeHalfEdge;

        halfEdgeMtoB->faceIndex = -1;
    }
}

void HalfEdge::Clear()
{
    _vertexHalfEdges.clear();
    _faceHalfEdges.clear();
    _edgeHalfEdges.clear();
    _halfEdges.clear();
}

glm::vec3 HalfEdge::GetFaceNormalNotNormalized(int faceIndex) const
{
    if (faceIndex == -1) return glm::vec3(0);

    std::vector<int> vertices;
    GetFaceVertices(faceIndex, vertices);

    glm::vec3 e0 = _points[vertices[1]] - _points[vertices[0]];
    glm::vec3 e1 = _points[vertices[2]] - _points[vertices[0]];
    return glm::cross(e0, e1);
}
