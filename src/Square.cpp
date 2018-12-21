#include "Square.h"

using namespace boost::numeric::ublas;

Square::Square(
    vector<double> v1,
    vector<double> v2,
    vector<double> v3,
    vector<double> v4)
{
    vertices_.push_back(v1);
    vertices_.push_back(v2);
    vertices_.push_back(v3);
    vertices_.push_back(v4);

    add_edge(0, 1);
    add_edge(1, 2);
    add_edge(2, 3);
    add_edge(3, 0);
}

Square::Square(
    boost::numeric::ublas::vector<double> v1,
    boost::numeric::ublas::vector<double> v2,
    boost::numeric::ublas::vector<double> v3,
    boost::numeric::ublas::vector<double> v4,
    const Color& horiz_col,
    const Color& vert_col)
{
    vertices_.push_back(v1);
    vertices_.push_back(v2);
    vertices_.push_back(v3);
    vertices_.push_back(v4);

    add_edge(0, 1, horiz_col);
    add_edge(1, 2, vert_col);
    add_edge(2, 3, horiz_col);
    add_edge(3, 0, vert_col);
}
