#include "HalfEdge.h"
#include <map>
#include <set>
#include <iostream>

bool HalfEdge::CheckValidManifold(const std::vector<size_t>& indices)
{
    return false;
}

void HalfEdge::preProcess(triangle_t* triangles, size_t triangleSize, std::vector<edge_t>& outEdges)
{
    outEdges.clear();
    outEdges.reserve(triangleSize * 3);

    for (size_t i = 0; i < triangleSize; ++i)
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


size_t FindFaceIdByEdge(size_t v0Id, size_t v1Id, const std::map<std::pair<size_t, size_t>, size_t>& book)
{
    std::map<std::pair<size_t, size_t>, size_t>::const_iterator it = book.find({ v0Id, v1Id });
    if (it == book.end())
        return INVALID_INDEX;
    else
        return it->second;
}

void HalfEdge::Build(
    const size_t numVertices,
    const size_t numTriangles,
    const triangle_t* triangles,
    const size_t numEdges,
    const edge_t* edges)
{
    std::map<std::pair<size_t, size_t>, size_t> edgeAndFaceBook;

    for (size_t faceIndex = 0; faceIndex < numTriangles; ++faceIndex)
    {
        const triangle_t& tri = triangles[faceIndex];
        edgeAndFaceBook[{tri.vertices[0], tri.vertices[1]}] = faceIndex;
        edgeAndFaceBook[{tri.vertices[1], tri.vertices[2]}] = faceIndex;
        edgeAndFaceBook[{tri.vertices[2], tri.vertices[0]}] = faceIndex;
    }

    Clear();

    _vertexHalfEdges.resize(numVertices, INVALID_INDEX);
    _edgeHalfEdges.resize(numEdges, INVALID_INDEX);
    _faceHalfEdges.resize(numTriangles, INVALID_INDEX);
    _halfEdges.reserve(numEdges * 2);

    for (size_t ei = 0; ei < numEdges; ++ei)
    {
        const edge_t& edge = edges[ei];

        const size_t h0Index = _halfEdges.size();
        _halfEdges.push_back(halfedge_t());
        halfedge_t& he0 = _halfEdges.back();

        const size_t h1Index = _halfEdges.size();
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

        if (_vertexHalfEdges[he0.toVertex] == INVALID_INDEX || he1.faceIndex == INVALID_INDEX)
            _vertexHalfEdges[he0.toVertex] = he0.oppositeHalfEdge;

        if (_vertexHalfEdges[he1.toVertex] == INVALID_INDEX || he0.faceIndex == INVALID_INDEX)
            _vertexHalfEdges[he1.toVertex] = he1.oppositeHalfEdge;

        if (he0.faceIndex != INVALID_INDEX && _faceHalfEdges[he0.faceIndex] == INVALID_INDEX)
            _faceHalfEdges[he0.faceIndex] = h0Index;

        if (he1.faceIndex != INVALID_INDEX && _faceHalfEdges[he1.faceIndex] == INVALID_INDEX)
            _faceHalfEdges[he1.faceIndex] = h1Index;

        _edgeHalfEdges[ei] = h0Index;
    }

    std::vector<size_t> boundaryHalfEdges;
    for (size_t hei = 0; hei < _halfEdges.size(); ++hei)
    {
        halfedge_t& he = _halfEdges[hei];
        if (he.faceIndex == INVALID_INDEX)
        {
            boundaryHalfEdges.push_back(hei);
            continue;
        }

        const triangle_t& face = triangles[he.faceIndex];
        const size_t vi = he.toVertex;

        size_t vj = INVALID_INDEX;
        if (face.vertices[0] == vi) vj = face.vertices[1];
        else if (face.vertices[1] == vi) vj = face.vertices[2];
        else if (face.vertices[2] == vi) vj = face.vertices[0];

        he.nextHlafEdge = _edgeToHalfEdge[{vi, vj}];
    }

    std::map<size_t, std::set<size_t>> vertex2OutgoingBoundaryHalfEdgeIndices;
    for (std::vector<size_t>::const_iterator hei = boundaryHalfEdges.begin(); hei != boundaryHalfEdges.end(); ++hei)
    {
        const size_t originateVertexIndex = _halfEdges[_halfEdges[*hei].oppositeHalfEdge].toVertex;
        vertex2OutgoingBoundaryHalfEdgeIndices[originateVertexIndex].insert(*hei);
        if (vertex2OutgoingBoundaryHalfEdgeIndices[originateVertexIndex].size() > 1)
        {
            std::cerr << "Butterfly vertex encountered.\n";
        }
    }

    for (std::vector<size_t>::const_iterator hei = boundaryHalfEdges.begin(); hei != boundaryHalfEdges.end(); ++hei)
    {
        halfedge_t& he = _halfEdges[*hei];

        std::set<size_t>& outgoing = vertex2OutgoingBoundaryHalfEdgeIndices[he.toVertex];
        if (outgoing.empty() == false)
        {
            std::set<size_t>::iterator outgoingIter = outgoing.begin();
            he.nextHlafEdge = *outgoingIter;
            outgoing.erase(outgoingIter);
        }
    }
}

void HalfEdge::GetBoundaryVertices(size_t boundaryVertexIndex, std::vector<size_t>& outBoundaryVertexIndices) const
{
    size_t hei = _vertexHalfEdges[boundaryVertexIndex];
    size_t start = hei;
    outBoundaryVertexIndices.push_back(boundaryVertexIndex);

    hei = _halfEdges[hei].nextHlafEdge;

    while (hei != start)
    {
        outBoundaryVertexIndices.push_back(_halfEdges[_halfEdges[hei].oppositeHalfEdge].toVertex);
        hei = _halfEdges[hei].nextHlafEdge;
    }
}

void HalfEdge::GetVertexIndicesOneRing(size_t vertexIndex, std::vector<size_t>& outRingVertexIndices) const
{
    size_t hei = _vertexHalfEdges[vertexIndex];
    size_t endVertex = _halfEdges[hei].toVertex;
    outRingVertexIndices.push_back(endVertex);
    hei = _halfEdges[_halfEdges[hei].oppositeHalfEdge].nextHlafEdge;

    while (_halfEdges[hei].toVertex != endVertex)
    {
        outRingVertexIndices.push_back(_halfEdges[hei].toVertex);
        hei = _halfEdges[_halfEdges[hei].oppositeHalfEdge].nextHlafEdge;
    }
}

void HalfEdge::GetFaceVertices(size_t faceIndex, std::vector<size_t>& outFaceVertexIndices) const
{
    if (faceIndex == INVALID_INDEX)
        return;

    size_t hei = _faceHalfEdges[faceIndex];
    size_t endVertexIndex = _halfEdges[_halfEdges[hei].oppositeHalfEdge].toVertex;
    size_t vertexIndex = _halfEdges[hei].toVertex;

    outFaceVertexIndices.push_back(endVertexIndex);
    while (vertexIndex != endVertexIndex)
    {
        outFaceVertexIndices.push_back(vertexIndex);
        hei = _halfEdges[hei].nextHlafEdge;
        vertexIndex = _halfEdges[hei].toVertex;
    }
}

void HalfEdge::GetFaceIndicesOneRing(size_t vertexIndex, std::vector<size_t>& outFaceIndices) const
{
    size_t hei = _vertexHalfEdges[vertexIndex];
    size_t end = _halfEdges[hei].toVertex;

    if (_halfEdges[hei].faceIndex != INVALID_INDEX)
        outFaceIndices.push_back(_halfEdges[hei].faceIndex);

    hei = _halfEdges[_halfEdges[hei].oppositeHalfEdge].nextHlafEdge;
    while (_halfEdges[hei].toVertex != end)
    {
        if (_halfEdges[hei].faceIndex != INVALID_INDEX)
            outFaceIndices.push_back(_halfEdges[hei].faceIndex);
        hei = _halfEdges[_halfEdges[hei].oppositeHalfEdge].nextHlafEdge;
    }
}

void HalfEdge::GetAdjFaceIndices(size_t faceIndex, std::vector<size_t>& outFaceIndices) const
{
    if (faceIndex == INVALID_INDEX) return;

    size_t hei = _faceHalfEdges[faceIndex];
    size_t end = _halfEdges[_halfEdges[hei].oppositeHalfEdge].toVertex;

    do
    {
        size_t fidx = _halfEdges[_halfEdges[hei].oppositeHalfEdge].faceIndex;
        if (fidx != INVALID_INDEX)
            outFaceIndices.push_back(fidx);
        hei = _halfEdges[hei].nextHlafEdge;
    } while (_halfEdges[_halfEdges[hei].oppositeHalfEdge].toVertex != end);
}


glm::vec3 HalfEdge::GetVertexNormal(size_t vertexIndex) const
{
    std::vector<size_t> faceIndices;
    glm::vec3 normal = glm::vec3(0, 0, 0);

    GetFaceIndicesOneRing(vertexIndex, faceIndices);

    for (size_t i = 0; i < faceIndices.size(); ++i)
    {
        normal += GetFaceNormalNotNormalized(faceIndices[i]);
    }

    return glm::normalize(normal);
}

glm::vec3 HalfEdge::GetFaceNormal(size_t faceIndex) const
{
    if (faceIndex == INVALID_INDEX) return glm::vec3(0);

    std::vector<size_t> vertices;
    GetFaceVertices(faceIndex, vertices);

    glm::vec3 e0 = _points[vertices[1]] - _points[vertices[0]];
    glm::vec3 e1 = _points[vertices[2]] - _points[vertices[0]];
    glm::vec3 normal = glm::cross(e0, e1);
    return glm::normalize(normal);
}


void HalfEdge::SplitEdge(size_t edgeIdx, float t) // a -> b -> c
{
    halfedge_t& h = _halfEdges[_edgeHalfEdges[edgeIdx]];
    size_t fromVertex = _halfEdges[h.oppositeHalfEdge].toVertex;
    size_t oppositeEdgeVertex = _halfEdges[_halfEdges[_edgeHalfEdges[edgeIdx]].nextHlafEdge].toVertex;

    size_t newVertexIdx = _points.size(); // m
    _points.push_back(t * _points[fromVertex] + (1 - t) * _points[h.toVertex]);

    size_t startVertexIdx = h.toVertex;
    _vertexHalfEdges.push_back(_halfEdges.size());
    
    bool oppositeflag = false;
    halfedge_t* halfEdgeBtoD;
    halfedge_t* halfEdgeDtoC;
    if (_halfEdges[h.oppositeHalfEdge].faceIndex != -1)
    {
        halfEdgeBtoD = &(_halfEdges[_halfEdges[h.oppositeHalfEdge].nextHlafEdge]);
        halfEdgeDtoC = &(_halfEdges[halfEdgeBtoD->nextHlafEdge]);
        oppositeflag = true;
    }

    { // edge 정리
        _edgeHalfEdges.push_back(_halfEdges.size() - 6);
        _edgeHalfEdges.push_back(_halfEdges.size() - 4);
        if (oppositeflag)
            _edgeHalfEdges.push_back(_halfEdges.size() - 2);
    }

    { // face 정리
        if (_halfEdges[_faceHalfEdges[h.faceIndex]].toVertex == oppositeEdgeVertex)
            _faceHalfEdges[h.faceIndex] = _halfEdges.size() - 6;
        _faceHalfEdges.push_back(_halfEdges.size() - 5);
        if (oppositeflag)
        {
            if (_halfEdges[_faceHalfEdges[_halfEdges[h.oppositeHalfEdge].faceIndex]].toVertex == _halfEdges[_halfEdges[h.oppositeHalfEdge].oppositeHalfEdge].toVertex)
                _faceHalfEdges[_halfEdges[h.oppositeHalfEdge].faceIndex] = _halfEdges.size() - 1;
            _faceHalfEdges.push_back(_halfEdges.size() - 2);
        }
    }

    { // half edge 만들기

        halfedge_t newHalfEdgeMtoA;
        newHalfEdgeMtoA.edgeIndex = _edgeHalfEdges.size();
        newHalfEdgeMtoA.faceIndex = h.faceIndex;
        newHalfEdgeMtoA.nextHlafEdge = _halfEdges[h.nextHlafEdge].nextHlafEdge;
        newHalfEdgeMtoA.oppositeHalfEdge = _halfEdges.size() + 1;
        newHalfEdgeMtoA.toVertex = oppositeEdgeVertex;

        halfedge_t newHalfEdgeAtoM;
        newHalfEdgeAtoM.edgeIndex = _edgeHalfEdges.size();
        newHalfEdgeAtoM.faceIndex = _faceHalfEdges.size();
        newHalfEdgeAtoM.nextHlafEdge = _halfEdges.size() + 2;
        newHalfEdgeAtoM.oppositeHalfEdge = _halfEdges.size();
        newHalfEdgeAtoM.toVertex = newVertexIdx;

        halfedge_t newHalfEdgeMtoC;
        newHalfEdgeMtoC.edgeIndex = _edgeHalfEdges.size() + 1;
        newHalfEdgeMtoC.faceIndex = _faceHalfEdges.size();
        newHalfEdgeMtoC.nextHlafEdge = h.nextHlafEdge;
        newHalfEdgeMtoC.oppositeHalfEdge = _halfEdges.size() + 3;
        newHalfEdgeMtoC.toVertex = h.toVertex;

        halfedge_t newHalfEdgeCtoM;
        newHalfEdgeCtoM.edgeIndex = _edgeHalfEdges.size() + 1;
        newHalfEdgeCtoM.faceIndex = _halfEdges[h.oppositeHalfEdge].faceIndex == -1 ? -1 : _faceHalfEdges.size() + 1;
        newHalfEdgeCtoM.nextHlafEdge = _halfEdges[h.oppositeHalfEdge].faceIndex == -1 ? h.oppositeHalfEdge : _halfEdges.size() + 4;
        newHalfEdgeCtoM.oppositeHalfEdge = _halfEdges.size() + 2;
        newHalfEdgeCtoM.toVertex = newVertexIdx;

        // 반대쪽 존재한다고 가정
        halfedge_t newHalfEdgeMtoD;
        newHalfEdgeMtoD.edgeIndex = _edgeHalfEdges.size() + 2;
        newHalfEdgeMtoD.faceIndex = _faceHalfEdges.size() + 1;
        newHalfEdgeMtoD.nextHlafEdge = _halfEdges[_halfEdges[h.oppositeHalfEdge].nextHlafEdge].nextHlafEdge;
        newHalfEdgeMtoD.oppositeHalfEdge = _halfEdges.size() + 5;
        newHalfEdgeMtoD.toVertex = _halfEdges[_halfEdges[h.oppositeHalfEdge].nextHlafEdge].toVertex;

        halfedge_t newHalfEdgeDtoM;
        newHalfEdgeDtoM.edgeIndex = _edgeHalfEdges.size() + 2;
        newHalfEdgeDtoM.faceIndex = _halfEdges[h.oppositeHalfEdge].faceIndex;
        newHalfEdgeDtoM.nextHlafEdge = h.oppositeHalfEdge;
        newHalfEdgeDtoM.oppositeHalfEdge = _halfEdges.size() + 4;
        newHalfEdgeDtoM.toVertex = newVertexIdx;

        h.nextHlafEdge = _halfEdges.size();
        h.toVertex = newVertexIdx;

        // ca

        halfedge_t& halfEdgeCtoA = _halfEdges[newHalfEdgeMtoC.nextHlafEdge];
        halfEdgeCtoA.nextHlafEdge = _halfEdges.size() + 1;

        if (oppositeflag)
        {
            halfEdgeBtoD->nextHlafEdge = _halfEdges.size() + 5;
            halfEdgeDtoC->nextHlafEdge = _halfEdges.size() + 3;
        }

        _halfEdges.push_back(newHalfEdgeMtoA);
        _halfEdges.push_back(newHalfEdgeAtoM);
        _halfEdges.push_back(newHalfEdgeMtoC);
        _halfEdges.push_back(newHalfEdgeCtoM);
        if (oppositeflag)
        {
            _halfEdges.push_back(newHalfEdgeMtoD);
            _halfEdges.push_back(newHalfEdgeDtoM);
        }
    }
}

void HalfEdge::Clear()
{
    _vertexHalfEdges.clear();
    _faceHalfEdges.clear();
    _edgeHalfEdges.clear();
    _halfEdges.clear();
}

glm::vec3 HalfEdge::GetFaceNormalNotNormalized(size_t faceIndex) const
{
    if (faceIndex == INVALID_INDEX) return glm::vec3(0);

    std::vector<size_t> vertices;
    GetFaceVertices(faceIndex, vertices);

    glm::vec3 e0 = _points[vertices[1]] - _points[vertices[0]];
    glm::vec3 e1 = _points[vertices[2]] - _points[vertices[0]];
    return glm::cross(e0, e1);
}
