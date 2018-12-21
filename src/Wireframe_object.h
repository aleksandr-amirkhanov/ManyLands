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
        r = 0;
        g = 0;
        b = 0;
    }
    Color(int r, int g, int b)
    {
        this->r = r;
        this->g = g;
        this->b = b;
    }

    int r, g, b;
};

struct Wireframe_edge
{
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

    std::vector<std::unique_ptr<Wireframe_edge>>& get_edges();
    const std::vector<std::unique_ptr<Wireframe_edge>>& edges() const;

    virtual void add_vertex(const boost::numeric::ublas::vector<double>& vert);
    virtual void add_edge(size_t vert1, size_t vert2);
    virtual void add_edge(size_t vert1, size_t vert2, Color color);

    void translate_vertices(boost::numeric::ublas::vector<double> translate);
    void scale_vertices(double scale_factor);
    void scale_vertices(boost::numeric::ublas::vector<double> scale_factor);

protected:
    std::vector<boost::numeric::ublas::vector<double>> vertices_;
    std::vector<std::unique_ptr<Wireframe_edge>> edges_;
};
