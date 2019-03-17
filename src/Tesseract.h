#pragma once
// Local
#include "Cube.h"
#include "Scene_wireframe_object.h"
// boost
#include <boost/numeric/ublas/vector.hpp>
// std
#include <vector>
#include <string>

class Tesseract : public Scene_wireframe_object
{
public:
    Tesseract(
        Scene_wireframe_vertex origin,
        Scene_wireframe_vertex size,
        const Color& x_color,
        const Color& y_color,
        const Color& z_color,
        const Color& w_color);

    Scene_wireframe_vertex get_size();

    std::vector<Cube> split();
    Square get_plain(std::string mask);

private:
    Scene_wireframe_vertex size_;
};
