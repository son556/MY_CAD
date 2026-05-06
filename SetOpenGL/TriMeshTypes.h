#pragma once


typedef unsigned long index_t;

struct edge_t
{
    index_t v[2] = { -1, -1 };

    index_t& start() { return v[0]; }
    const index_t& start() const { return v[0]; }

    index_t& end() { return v[1]; }
    const index_t& end() const { return v[1]; }
};

struct triangle_t
{
    index_t v[3] = {-1, -1, -1};

    index_t& i() { return v[0]; }
    const index_t& i() const { return v[0]; }

    index_t& j() { return v[1]; }
    const index_t& j() const { return v[1]; }

    index_t& k() { return v[2]; }
    const index_t& k() const { return v[2]; }
};
