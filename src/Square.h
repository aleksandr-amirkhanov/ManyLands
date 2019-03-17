#pragma once

#include "Scene_wireframe_object.h"
#include "Color.h"

#include <boost/numeric/ublas/vector.hpp>
// std
#include <vector>

class Square : public Scene_wireframe_object
{
public:
     Square(
        boost::numeric::ublas::vector<double> v1,
        boost::numeric::ublas::vector<double> v2,
        boost::numeric::ublas::vector<double> v3,
        boost::numeric::ublas::vector<double> v4);

    Square(boost::numeric::ublas::vector<double> v1,
        boost::numeric::ublas::vector<double> v2,
        boost::numeric::ublas::vector<double> v3,
        boost::numeric::ublas::vector<double> v4,
        const Color &horiz_col,
        const Color &vert_col);

private:
    const static Color default_color_;
};
