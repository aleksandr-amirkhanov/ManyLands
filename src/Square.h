#pragma once

#include "Wireframe_object.h"
#include <boost/numeric/ublas/vector.hpp>
// std
#include <vector>

class Square : public Wireframe_object
{
public:
     Square(
        boost::numeric::ublas::vector<double> v1,
        boost::numeric::ublas::vector<double> v2,
        boost::numeric::ublas::vector<double> v3,
        boost::numeric::ublas::vector<double> v4);

    Square(
        boost::numeric::ublas::vector<double> v1,
        boost::numeric::ublas::vector<double> v2,
        boost::numeric::ublas::vector<double> v3,
        boost::numeric::ublas::vector<double> v4,
        const Color* horiz_col,
        const Color* vert_col);
};
