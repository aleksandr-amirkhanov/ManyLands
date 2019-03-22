#include "Square.h"

const Color Square::default_color_ = Color(0, 0, 0, 255);

//******************************************************************************
// Square
//******************************************************************************

Square::Square(Scene_wireframe_vertex v1,
               Scene_wireframe_vertex v2,
               Scene_wireframe_vertex v3,
               Scene_wireframe_vertex v4)
{
    vertices_.push_back(v1);
    vertices_.push_back(v2);
    vertices_.push_back(v3);
    vertices_.push_back(v4);

    add_edge(Scene_wireframe_edge(0, 1, default_color_));
    add_edge(Scene_wireframe_edge(1, 2, default_color_));
    add_edge(Scene_wireframe_edge(2, 3, default_color_));
    add_edge(Scene_wireframe_edge(3, 0, default_color_));
}

//******************************************************************************
// Square
//******************************************************************************

Square::Square(Scene_wireframe_vertex v1,
               Scene_wireframe_vertex v2,
               Scene_wireframe_vertex v3,
               Scene_wireframe_vertex v4,
               const Color& horiz_col,
               const Color& vert_col)
{
    vertices_.push_back(v1);
    vertices_.push_back(v2);
    vertices_.push_back(v3);
    vertices_.push_back(v4);

    add_edge(Scene_wireframe_edge(0, 1, horiz_col));
    add_edge(Scene_wireframe_edge(1, 2, vert_col ));
    add_edge(Scene_wireframe_edge(2, 3, horiz_col));
    add_edge(Scene_wireframe_edge(3, 0, vert_col ));
}
