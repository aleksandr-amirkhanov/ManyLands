#pragma once

#include "Scene_wireframe_object.h"
#include "Color.h"

#include <boost/numeric/ublas/vector.hpp>
// std
#include <vector>

class Square : public Scene_wireframe_object
{
public:
     Square(Scene_wireframe_vertex v1,
            Scene_wireframe_vertex v2,
            Scene_wireframe_vertex v3,
            Scene_wireframe_vertex v4);

    Square(Scene_wireframe_vertex v1,
           Scene_wireframe_vertex v2,
           Scene_wireframe_vertex v3,
           Scene_wireframe_vertex v4,
           const Color &horiz_col,
           const Color &vert_col);

private:
    const static Color default_color_;
};
