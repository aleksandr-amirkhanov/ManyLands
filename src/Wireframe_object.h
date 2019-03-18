#pragma once

#include <vector>

#include "Color.h"

template<class TVertex, class TEdge>
class Wireframe_object
{
public:
    Wireframe_object();
    Wireframe_object(const Wireframe_object& w);
    Wireframe_object& operator=(const Wireframe_object& other);

    std::vector<TVertex>& get_vertices();
    std::vector<TEdge>&   get_edges();

    const std::vector<TVertex>& vertices() const;
    const std::vector<TEdge>&   edges() const;

    virtual void add_vertex(const TVertex& v);
    virtual void add_edge(const TEdge& e);

    /*void translate_vertices(boost::numeric::ublas::vector<double> translate);
    void scale_vertices(double scale_factor);
    void scale_vertices(boost::numeric::ublas::vector<double> scale_factor);*/

protected:
    std::vector<TVertex> vertices_;
    std::vector<TEdge>   edges_;
};

//******************************************************************************
// Wireframe_object
//******************************************************************************

template<class TVertex, class TEdge>
Wireframe_object<TVertex, TEdge>::Wireframe_object()
{
}

//******************************************************************************
// Wireframe_object
//******************************************************************************

template<class TVertex, class TEdge>
Wireframe_object<TVertex, TEdge>::Wireframe_object(const Wireframe_object& w)
{
    operator=(w);
}

//******************************************************************************
// operator=
//******************************************************************************

template<class TVertex, class TEdge>
Wireframe_object<TVertex, TEdge>& Wireframe_object<TVertex, TEdge>::operator=(
    const Wireframe_object& other)
{
    vertices_ = other.vertices_; // Copy the vertex array
    edges_ = other.edges_;       // Copy the edge array

    return *this;
}

//******************************************************************************
// get_vertices
//******************************************************************************

template<class TVertex, class TEdge>
std::vector<TVertex>& Wireframe_object<TVertex, TEdge>::get_vertices()
{
    return vertices_;
}

//******************************************************************************
// vertices
//******************************************************************************

template<class TVertex, class TEdge>
const std::vector<TVertex>& Wireframe_object<TVertex, TEdge>::vertices() const
{
    return vertices_;
}

//******************************************************************************
// get_edges
//******************************************************************************

template<class TVertex, class TEdge>
std::vector<TEdge>& Wireframe_object<TVertex, TEdge>::get_edges()
{
    return edges_;
}

//******************************************************************************
// edges
//******************************************************************************

template<class TVertex, class TEdge>
const std::vector<TEdge>& Wireframe_object<TVertex, TEdge>::edges() const
{
    return edges_;
}

//******************************************************************************
// add_vertex
//******************************************************************************

template<class TVertex, class TEdge>
void Wireframe_object<TVertex, TEdge>::add_vertex(
    const TVertex& vert)
{
    vertices_.push_back(vert);
}

//******************************************************************************
// add_edge
//******************************************************************************

template<class TVertex, class TEdge>
void Wireframe_object<TVertex, TEdge>::add_edge(const TEdge& e)
{
    edges_.push_back(e);
}

//******************************************************************************
// translate_vertices
//******************************************************************************

/*template<class TVertex, class TEdge>
void Wireframe_object::translate_vertices(
    boost::numeric::ublas::vector<double> translate)
{
    for(auto& v : vertices_)
        v += translate;
}*/

//******************************************************************************
// scale_vertices
//******************************************************************************

/*template<class TVertex, class TEdge>
void Wireframe_object::scale_vertices(double scale_factor)
{
    for(auto& v : vertices_)
        v *= scale_factor;
}*/

//******************************************************************************
// scale_vertices
//******************************************************************************

/*template<class TVertex, class TEdge>
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
}*/
