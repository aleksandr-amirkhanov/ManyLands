#pragma once
// Local
#include "Wireframe_object.h"
#include "Color.h"
#include "Scene_vertex_t.h"

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

typedef Wireframe_object<Scene_vertex_t, Scene_wireframe_edge>
    Scene_wireframe_object;
