#pragma once

// boost
#include <boost/numeric/ublas/vector.hpp>
// std
#include <memory>
#include <vector>

struct Color
{
    Color()
    {
        r = g = b = a = 0;
    }
    Color(int r, int g, int b, int a = 255)
    {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }
    void copy(const Color& other)
    {
        this->r = other.r;
        this->g = other.g;
        this->b = other.b;
        this->a = other.a;
    }

    int r, g, b, a;
};

struct Wireframe_edge
{
    static Wireframe_edge
    make(const size_t vert1, const size_t vert2, const Color& color)
    {
        const Wireframe_edge out = {vert1, vert2, color};
        return out;
    }
    size_t vert1;
    size_t vert2;
    Color color;
};

class Wireframe_object
{
public:
    Wireframe_object();
    Wireframe_object(const Wireframe_object& w);
    Wireframe_object& operator=(const Wireframe_object& other);

    std::vector<boost::numeric::ublas::vector<double>>& get_vertices();
    const std::vector<boost::numeric::ublas::vector<double>>& vertices() const;

    const std::vector<Wireframe_edge>& edges() const;

    virtual void add_vertex(const boost::numeric::ublas::vector<double>& vert);
    virtual void add_edge(size_t vert1, size_t vert2);
    virtual void add_edge(size_t vert1, size_t vert2, const Color &color);

    void translate_vertices(boost::numeric::ublas::vector<double> translate);
    void scale_vertices(double scale_factor);
    void scale_vertices(boost::numeric::ublas::vector<double> scale_factor);

    const static Color default_color;

protected:
    std::vector<boost::numeric::ublas::vector<double>> vertices_;
    std::vector<Wireframe_edge> edges_;
};
