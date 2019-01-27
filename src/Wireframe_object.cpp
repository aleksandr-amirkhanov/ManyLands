#include "Wireframe_object.h"

const Color Wireframe_object::default_color = Color(0, 0, 0, 255);

Wireframe_object::Wireframe_object()
{
}

Wireframe_object::Wireframe_object(const Wireframe_object& w)
{
    operator=(w);
}

Wireframe_object& Wireframe_object::operator=(const Wireframe_object& other)
{
    // Copy the vertex array
    this->vertices_ = other.vertices_;

    // Copy the edge array
    for(auto& e : other.edges_)
    {
        std::unique_ptr<Wireframe_edge> new_edge =
            std::make_unique<Wireframe_edge>();
        new_edge->vert1 = e->vert1;
        new_edge->vert2 = e->vert2;
        new_edge->color = e->color;

        this->edges_.push_back(std::move(new_edge));
    }

    return *this;
}

std::vector<boost::numeric::ublas::vector<double>>&
Wireframe_object::get_vertices()
{
    return vertices_;
}

const std::vector<boost::numeric::ublas::vector<double>>&
Wireframe_object::vertices() const
{
    return vertices_;
}

std::vector<std::unique_ptr<Wireframe_edge>>& Wireframe_object::get_edges()
{
    return edges_;
}

const std::vector<std::unique_ptr<Wireframe_edge>>&
Wireframe_object::edges() const
{
    return edges_;
}

void Wireframe_object::add_vertex(
    const boost::numeric::ublas::vector<double>& vert)
{
    vertices_.push_back(vert);
}

void Wireframe_object::add_edge(size_t vert1, size_t vert2)
{
    add_edge(vert1, vert2, &default_color);
}

void Wireframe_object::add_edge(size_t vert1, size_t vert2, const Color* color)
{
    std::unique_ptr<Wireframe_edge> e = std::make_unique<Wireframe_edge>();
    e->vert1 = vert1;
    e->vert2 = vert2;
    e->color = color;

    edges_.push_back(std::move(e));
}

void Wireframe_object::translate_vertices(
    boost::numeric::ublas::vector<double> translate)
{
    for(auto& v : vertices_)
        v += translate;
}

void Wireframe_object::scale_vertices(double scale_factor)
{
    for(auto& v : vertices_)
        v *= scale_factor;
}

void Wireframe_object::scale_vertices(
    boost::numeric::ublas::vector<double> scale_factor)
{
    for(auto& v : vertices_)
    {
        assert(v.size() == scale_factor.size());

        if(v.size() == scale_factor.size())
        {
            for(int i = 0; i < v.size(); i++)
                v[i] *= scale_factor[i];
        }
    }
}
