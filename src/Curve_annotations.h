#pragma once

#include <boost/numeric/ublas/vector.hpp>

struct Curve_annotations
{
    unsigned int dimensionality;
    boost::numeric::ublas::vector<double> dir;
    boost::numeric::ublas::vector<double> point;
};