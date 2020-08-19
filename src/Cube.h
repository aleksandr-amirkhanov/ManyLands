#pragma once
// Local
#include "Scene_wireframe_object.h"
#include "Square.h"
#include "Color.h"
// boost
#include <boost/numeric/ublas/vector.hpp>
// std
#include <string>
#include <vector>

class Cube : public Scene_wireframe_object
{
public:
    Cube(Scene_vertex_t v1,
         Scene_vertex_t v2,
         Scene_vertex_t v3,
         Scene_vertex_t v4,
         Scene_vertex_t v5,
         Scene_vertex_t v6,
         Scene_vertex_t v7,
         Scene_vertex_t v8);

    Cube(Scene_vertex_t v1,
         Scene_vertex_t v2,
         Scene_vertex_t v3,
         Scene_vertex_t v4,
         Scene_vertex_t v5,
         Scene_vertex_t v6,
         Scene_vertex_t v7,
         Scene_vertex_t v8,
         const Color &horiz_col,
         const Color &vert_col,
         const Color &depth_col);

    static std::vector<Square> split(std::vector<Cube>& cubes);

private:
    const static Color default_color_;
};
