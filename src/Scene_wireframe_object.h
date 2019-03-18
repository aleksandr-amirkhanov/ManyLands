#pragma once

#include "Wireframe_object.h"

#include <boost/numeric/ublas/vector.hpp>

typedef boost::numeric::ublas::vector<float> Scene_wireframe_vertex;

struct Scene_wireframe_edge
{
    Scene_wireframe_edge(const size_t v1,
                         const size_t v2,
                         const Color& c)
    {
        vert1 = v1;
        vert2 = v2;
        color = c;
    }

    size_t vert1;
    size_t vert2;
    Color color;
};

typedef Wireframe_object<Scene_wireframe_vertex, Scene_wireframe_edge>
    Scene_wireframe_object;
