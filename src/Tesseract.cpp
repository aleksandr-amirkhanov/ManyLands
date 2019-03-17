#include "Tesseract.h"
// boost
#include <boost/numeric/ublas/assignment.hpp>
// std
#include <tuple>

Tesseract::Tesseract(
    Scene_wireframe_vertex origin,
    Scene_wireframe_vertex size,
    const Color& x_color,
    const Color& y_color,
    const Color& z_color,
    const Color& w_color)
{
    vertices_.resize(16);
    edges_.resize(32, Scene_wireframe_edge(0, 0, Color()));

    size_ = size;

    auto index =
        [](unsigned int x, unsigned int y, unsigned int z, unsigned int w) {
            return (x * 8 + y * 4 + z * 2 + w);
        };

    size_t li = 0;
    for(char x = 0; x < 2; ++x)
    {
        for(char y = 0; y < 2; ++y)
        {
            for(char z = 0; z < 2; ++z)
            {
                for(char w = 0; w < 2; ++w)
                {
                    Scene_wireframe_vertex v(5);
                    v(0) = x == 0 ? origin(0) : origin(0) + size_(0);
                    v(1) = y == 0 ? origin(1) : origin(1) + size_(1);
                    v(2) = z == 0 ? origin(2) : origin(2) + size_(2);
                    v(3) = w == 0 ? origin(3) : origin(3) + size_(3);
                    v(4) = 1;
                    vertices_[index(x, y, z, w)] = v;

                    if(x == 0)
                    {
                        edges_[li++] = {
                            index(0, y, z, w), index(1, y, z, w), x_color};
                    }
                    if(y == 0)
                    {
                        edges_[li++] = {
                            index(x, 0, z, w), index(x, 1, z, w), y_color};
                    }
                    if(z == 0)
                    {
                        edges_[li++] = {
                            index(x, y, 0, w), index(x, y, 1, w), z_color};
                    }
                    if(w == 0)
                    {
                        edges_[li++] = {
                            index(x, y, z, 0), index(x, y, z, 1), w_color};
                    }
                }
            }
        }
    }
}

Scene_wireframe_vertex Tesseract::get_size()
{
    return size_;
}

std::vector<Cube> Tesseract::split()
{
    std::vector<Cube> cubes;

    const auto& v = vertices();

    auto get_color = [&](int vert_ind1, int vert_ind2) {
        for(const auto& e : edges())
        {
            if((e.vert1 == vert_ind1 && e.vert2 == vert_ind2) ||
               (e.vert1 == vert_ind2 && e.vert2 == vert_ind1))
            {
                return e.color;
            }
        }
        throw std::logic_error("The color always should be found!");
    };

    auto add_cube =
        [&](int v1, int v2, int v3, int v4, int v5, int v6, int v7, int v8) {
            cubes.emplace_back(Cube(
                v[v1],
                v[v2],
                v[v3],
                v[v4],
                v[v5],
                v[v6],
                v[v7],
                v[v8],
                get_color(v1, v2),
                get_color(v1, v4),
                get_color(v1, v5)));
        };

    // Cube 1
    add_cube(0b0011, 0b1011, 0b1111, 0b0111, 0b0001, 0b1001, 0b1101, 0b0101);
    // Cube 2
    add_cube(0b0010, 0b1010, 0b1110, 0b0110, 0b0000, 0b1000, 0b1100, 0b0100);
    // Cube 3
    add_cube(0b0011, 0b1011, 0b1111, 0b0111, 0b0010, 0b1010, 0b1110, 0b0110);
    // Cube 4
    add_cube(0b0000, 0b1000, 0b1100, 0b0100, 0b0001, 0b1001, 0b1101, 0b0101);
    // Cube 5
    add_cube(0b0011, 0b1011, 0b1010, 0b0010, 0b0001, 0b1001, 0b1000, 0b0000);
    // Cube 6
    add_cube(0b0110, 0b1110, 0b1111, 0b0111, 0b0100, 0b1100, 0b1101, 0b0101);
    // Cube 7
    add_cube(0b0011, 0b0010, 0b0110, 0b0111, 0b0001, 0b0000, 0b0100, 0b0101);
    // Cube 8
    add_cube(0b1010, 0b1011, 0b1111, 0b1110, 0b1000, 0b1001, 0b1101, 0b1100);

    return cubes;
}

Square Tesseract::get_plain(std::string mask)
{
    std::vector<std::tuple<char, char>> limits(4);
    for(char i = 0; i < 4; ++i)
    {
        if(mask[i] == '0')
        {
            std::get<0>(limits[i]) = 0;
            std::get<1>(limits[i]) = 0;
        }
        else if(mask[i] == '1')
        {
            std::get<0>(limits[i]) = 1;
            std::get<1>(limits[i]) = 1;
        }
        else
        {
            std::get<0>(limits[i]) = 0;
            std::get<1>(limits[i]) = 1;
        }
    }

    std::vector<Scene_wireframe_vertex> verts;
    for(char x = std::get<0>(limits[0]); x <= std::get<1>(limits[0]); ++x)
    {
        for(char y = std::get<0>(limits[1]); y <= std::get<1>(limits[1]); ++y)
        {
            for(char z = std::get<0>(limits[2]); z <= std::get<1>(limits[2]);
                ++z)
            {
                for(char w = std::get<0>(limits[3]);
                    w <= std::get<1>(limits[3]);
                    ++w)
                {
                    size_t vert_ind = x << 3 | y << 2 | z << 1 | w << 0;
                    verts.push_back(vertices_[vert_ind]);
                }
            }
        }
    }

    return Square(verts[0], verts[1], verts[3], verts[2]);
}
