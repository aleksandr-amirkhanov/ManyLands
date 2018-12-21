#include "Cube.h"
// boost
#include <boost/numeric/ublas/assignment.hpp>

using namespace boost::numeric::ublas;

Cube::Cube(
    boost::numeric::ublas::vector<double> v1,
    boost::numeric::ublas::vector<double> v2,
    boost::numeric::ublas::vector<double> v3,
    boost::numeric::ublas::vector<double> v4,
    boost::numeric::ublas::vector<double> v5,
    boost::numeric::ublas::vector<double> v6,
    boost::numeric::ublas::vector<double> v7,
    boost::numeric::ublas::vector<double> v8)
    : Cube(v1, v2, v3, v4, v5, v6, v7, v8, Color(), Color(), Color())
{
}

Cube::Cube(
    boost::numeric::ublas::vector<double> v1,
    boost::numeric::ublas::vector<double> v2,
    boost::numeric::ublas::vector<double> v3,
    boost::numeric::ublas::vector<double> v4,
    boost::numeric::ublas::vector<double> v5,
    boost::numeric::ublas::vector<double> v6,
    boost::numeric::ublas::vector<double> v7,
    boost::numeric::ublas::vector<double> v8,
    Color horiz_col,
    Color vert_col,
    Color depth_col)
{
    vertices_.push_back(v1);
    vertices_.push_back(v2);
    vertices_.push_back(v3);
    vertices_.push_back(v4);
    vertices_.push_back(v5);
    vertices_.push_back(v6);
    vertices_.push_back(v7);
    vertices_.push_back(v8);

    add_edge(0, 1, horiz_col);
    add_edge(1, 2, vert_col);
    add_edge(2, 3, horiz_col);
    add_edge(3, 0, vert_col);

    add_edge(4, 5, horiz_col);
    add_edge(5, 6, vert_col);
    add_edge(6, 7, horiz_col);
    add_edge(7, 4, vert_col);

    add_edge(0, 4, depth_col);
    add_edge(1, 5, depth_col);
    add_edge(2, 6, depth_col);
    add_edge(3, 7, depth_col);
}

std::vector<Square> Cube::split(std::vector<Cube>& cubes)
{
    std::vector<Square> squares;

    auto get_color = [&](size_t cube_ind, size_t v1, size_t v2) {
        for(const auto& e : cubes[cube_ind].get_edges())
        {
            if((e->vert1 == v1 && e->vert2 == v2) ||
               (e->vert1 == v2 && e->vert2 == v1))
            {
                return e->color;
            }
        }

        return Color();
    };

    auto create_square = [&](int cube_ind, int v1, int v2, int v3, int v4) {
        return Square(
            cubes[cube_ind].get_vertices()[v1],
            cubes[cube_ind].get_vertices()[v2],
            cubes[cube_ind].get_vertices()[v3],
            cubes[cube_ind].get_vertices()[v4],
            get_color(cube_ind, v1, v2),
            get_color(cube_ind, v1, v4));
    };

    squares.push_back(create_square(5, 4, 5, 6, 7));
    squares.push_back(create_square(1, 4, 5, 6, 7));
    squares.push_back(create_square(7, 4, 5, 6, 7));
    squares.push_back(create_square(1, 0, 1, 5, 4));
    squares.push_back(create_square(7, 0, 1, 5, 4));
    squares.push_back(create_square(7, 1, 2, 6, 5));

    return squares;
}
