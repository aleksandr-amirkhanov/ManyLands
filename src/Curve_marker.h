#pragma once

#include "Mesh.h"
#include <boost/numeric/ublas/vector.hpp>
#include <memory>

struct Curve_marker
{
public:
    Curve_marker(const Mesh& m);

    std::unique_ptr<Mesh> mesh;
    double distance;
};
