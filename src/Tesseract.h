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
        boost::numeric::ublas::vector<double> origin,
        boost::numeric::ublas::vector<double> size,
        const Color& x_color,
        const Color& y_color,
        const Color& z_color,
        const Color& w_color);

    boost::numeric::ublas::vector<double> get_size();

    std::vector<Cube> split();
    Square get_plain(std::string mask);

private:
    boost::numeric::ublas::vector<double> size_;
};
