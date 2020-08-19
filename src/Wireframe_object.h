#pragma once
// Local
#include "Vertex_object.h"

template<class TVertex, class TEdge>
class Wireframe_object : public Vertex_object<TVertex>
{
public:
    Wireframe_object();
    Wireframe_object(const Wireframe_object& w);
    Wireframe_object& operator=(const Wireframe_object& other);

    std::vector<TEdge>&         get_edges();
    const std::vector<TEdge>&   edges() const;
    virtual void                add_edge(const TEdge& e);

protected:
    std::vector<TEdge> edges_;
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
// add_edge
//******************************************************************************

template<class TVertex, class TEdge>
void Wireframe_object<TVertex, TEdge>::add_edge(const TEdge& e)
{
    edges_.push_back(e);
}
