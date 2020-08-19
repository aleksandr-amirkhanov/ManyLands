#pragma once

#include "Scene_wireframe_object.h"
#include "Color.h"

#include <boost/numeric/ublas/vector.hpp>
// std
#include <vector>

class Square : public Scene_wireframe_object
{
public:
     Square(Scene_vertex_t v1,
            Scene_vertex_t v2,
            Scene_vertex_t v3,
            Scene_vertex_t v4);

    Square(Scene_vertex_t v1,
           Scene_vertex_t v2,
           Scene_vertex_t v3,
           Scene_vertex_t v4,
           const Color &horiz_col,
           const Color &vert_col);

private:
    const static Color default_color_;
};
