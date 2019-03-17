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
    Cube(Scene_wireframe_vertex v1,
         Scene_wireframe_vertex v2,
         Scene_wireframe_vertex v3,
         Scene_wireframe_vertex v4,
         Scene_wireframe_vertex v5,
         Scene_wireframe_vertex v6,
         Scene_wireframe_vertex v7,
         Scene_wireframe_vertex v8);

    Cube(Scene_wireframe_vertex v1,
         Scene_wireframe_vertex v2,
         Scene_wireframe_vertex v3,
         Scene_wireframe_vertex v4,
         Scene_wireframe_vertex v5,
         Scene_wireframe_vertex v6,
         Scene_wireframe_vertex v7,
         Scene_wireframe_vertex v8,
         const Color &horiz_col,
         const Color &vert_col,
         const Color &depth_col);

    static std::vector<Square> split(std::vector<Cube>& cubes);

private:
    const static Color default_color_;
};
