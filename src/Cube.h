#pragma once
// Local
#include "Wireframe_object.h"
#include "Square.h"
// boost
#include <boost/numeric/ublas/vector.hpp>
// std
#include <string>
#include <vector>

class Cube : public Wireframe_object
{
public:
    Cube(
        boost::numeric::ublas::vector<double> v1,
        boost::numeric::ublas::vector<double> v2,
        boost::numeric::ublas::vector<double> v3,
        boost::numeric::ublas::vector<double> v4,
        boost::numeric::ublas::vector<double> v5,
        boost::numeric::ublas::vector<double> v6,
        boost::numeric::ublas::vector<double> v7,
        boost::numeric::ublas::vector<double> v8);

    Cube(boost::numeric::ublas::vector<double> v1,
        boost::numeric::ublas::vector<double> v2,
        boost::numeric::ublas::vector<double> v3,
        boost::numeric::ublas::vector<double> v4,
        boost::numeric::ublas::vector<double> v5,
        boost::numeric::ublas::vector<double> v6,
        boost::numeric::ublas::vector<double> v7,
        boost::numeric::ublas::vector<double> v8,
        const Color &horiz_col,
        const Color &vert_col,
        const Color &depth_col);

    static std::vector<Square> split(std::vector<Cube>& cubes);
};
