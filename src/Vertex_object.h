#pragma once
// std
#include <vector>
// boost
#include <boost/numeric/ublas/vector.hpp>

template<class TVertex>
class Vertex_object
{
public:
    std::vector<TVertex>&       get_vertices();
    const std::vector<TVertex>& vertices() const;
    virtual void                add_vertex(const TVertex& v);

    void translate_vertices(boost::numeric::ublas::vector<double> translate);
    void scale_vertices(double scale_factor);
    void scale_vertices(boost::numeric::ublas::vector<double> scale_factor);

    void get_boundaries(TVertex& origin,
                        TVertex& size) const;

protected:
    std::vector<TVertex> vertices_;
};

//******************************************************************************
// get_vertices
//******************************************************************************

template<class TVertex>
std::vector<TVertex>& Vertex_object<TVertex>::get_vertices()
{
    return vertices_;
}

//******************************************************************************
// vertices
//******************************************************************************

template<class TVertex>
const std::vector<TVertex>& Vertex_object<TVertex>::vertices() const
{
    return vertices_;
}

//******************************************************************************
// add_vertex
//******************************************************************************

template<class TVertex>
void Vertex_object<TVertex>::add_vertex(
    const TVertex& vert)
{
    vertices_.push_back(vert);
}

//******************************************************************************
// translate_vertices
//******************************************************************************

template<class TVertex>
void Vertex_object<TVertex>::translate_vertices(
    boost::numeric::ublas::vector<double> translate)
{
    for(auto& v : vertices_)
        v += translate;
}

//******************************************************************************
// scale_vertices
//******************************************************************************

template<class TVertex>
void Vertex_object<TVertex>::scale_vertices(double scale_factor)
{
    for(auto& v : vertices_)
        v *= scale_factor;
}

//******************************************************************************
// scale_vertices
//******************************************************************************

template<class TVertex>
void Vertex_object<TVertex>::scale_vertices(
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

//******************************************************************************
// get_boundaries
//******************************************************************************

template<class TVertex>
void Vertex_object<TVertex>::get_boundaries(
    TVertex& origin,
    TVertex& size) const
{
    // Finding minimum and maximum values of the curve
    TVertex min(5);
    TVertex max(5);

    if (vertices().size() > 0)
    {
        const auto& first = vertices().at(0);
        min = first;
        max = first;
    }

    for (const auto& v : vertices())
    {
        for (unsigned char i = 0; i < 4; ++i)
        {
            if (v(i) < min(i))
                min(i) = v(i);
            if (v(i) > max(i))
                max(i) = v(i);
        }
    }

    origin = min;
    size = max - min;
}
