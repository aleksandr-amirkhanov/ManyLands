#include "Curve_marker.h"

//******************************************************************************
// Curve_marker
//******************************************************************************

Curve_marker::Curve_marker(const Mesh& m)
    : mesh(std::make_unique<Mesh>(m))
    , distance(0)
{
}
