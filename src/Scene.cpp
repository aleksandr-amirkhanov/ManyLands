#include "Scene.h"
// local
#include "Mesh.h"
// std
#include <fstream>
// boost
#include <boost/algorithm/string.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/assignment.hpp>

//******************************************************************************
// Scene
//******************************************************************************

Scene::Scene(std::shared_ptr<Scene_state> state)
    : state_(state)
{
}

//******************************************************************************
// load_ode
//******************************************************************************

void Scene::load_ode(
    const std::vector<std::string>& fnames,
    float cuve_min_rad,
    float tesseract_size/* = 200.f*/)
{
    assert(state_);
    if(state_ == nullptr)
        return;

    // Remove all previous curves
    state_->curves.clear();
    // Reset size of the tesseract
    for(auto& s : state_->tesseract_size)
        s = tesseract_size;

    // The aggregative origin and size for all curves
    Scene_vertex_t total_origin(5), total_size(5);
    for(char i = 0; i < 5; ++i)
    {
        total_origin(i) = std::numeric_limits<float>::max();
        total_size(i)   = std::numeric_limits<float>::min();
    }

    std::vector<std::shared_ptr<Curve>> curves;
    
    // Load curves from files and calculate statistics
    for(auto& fn: fnames)
    {
        // Load curves
        auto curve = load_curve(fn);
        if(!curve)
            continue;

        Scene_vertex_t origin, size;
        curve->get_boundaries(origin, size);

        for(char i = 0; i < 5; ++i)
        {
            if(total_origin(i) > origin(i)) total_origin(i) = origin(i);
            if(total_size(i)   < size(i)  ) total_size(i)   = size(i);
        }

        curves.push_back(std::move(curve));
    }

    if(!state_->scale_tesseract)
    {
        float max_size = std::numeric_limits<float>::min();
        for(char i = 0; i < 4; ++i)
        {
            if(max_size < total_size(i)) max_size = total_size(i);
        }

        for(char i = 0; i < 4; ++i)
        {
            state_->tesseract_size[i] =
                tesseract_size * total_size[i] / max_size;
        }
    }

    Scene_vertex_t scale(5);
    scale[0] = state_->tesseract_size[0] / total_size[0];
    scale[1] = state_->tesseract_size[1] / total_size[1];
    scale[2] = state_->tesseract_size[2] / total_size[2];
    scale[3] = state_->tesseract_size[3] / total_size[3];
    scale[4] = 1;

    for(auto& c : curves)
    {
        c->translate_vertices(-0.5f * total_size - total_origin);
        c->scale_vertices(scale);

        auto curve = std::make_shared<Curve>(
            c->get_simpified_curve(cuve_min_rad));
        curve->update_stats(
            state_->stat_kernel_size,
            state_->stat_max_movement,
            state_->stat_max_value);
        state_->curves.push_back(std::move(curve));
    }

    // Save curve origin and size to the class members
    create_tesseract();

    if(state_->curves.size() > 0)
    {
        // Set selection (currently we take the range of the first curve, but
        // this behaviour can be reconsidered in the future)
        auto& first_curve = state_->curves.front();
        state_->curve_selection = std::make_unique<Curve_selection>();
        state_->curve_selection->t_start = first_curve->t_min();
        state_->curve_selection->t_end = first_curve->t_max();
    }
}

//******************************************************************************
// load_curve
//******************************************************************************

std::shared_ptr<Curve> Scene::load_curve(std::string fname)
{
    std::ifstream stream(fname);
    if(!stream.is_open())
        return nullptr;

    auto curve = std::make_shared<Curve>();

    std::string line;
    while(getline(stream, line))
    {
        // Remove some characters to make it easier to load files we get from
        // Wolfram Mathematica
        boost::erase_all(line, "\"{");
        boost::erase_all(line, "}\"");
        boost::replace_all(line, ", ", " ");
        boost::replace_all(line, "*^", "e");

        std::vector<std::string> vals;
        boost::split(vals, line, boost::is_any_of("\t, "));
        if(vals.size() == 5)
        {
            float t = std::stof(vals[0]);
            Scene_vertex_t p(5);
            p <<= std::stof(vals[1]), std::stof(vals[2]), std::stof(vals[3]),
                std::stof(vals[4]), 1;

            curve->add_point(p, t);
        }
    }

    return curve;
}

//******************************************************************************
// normalize_curve
//******************************************************************************

void Scene::normalize_curve(Curve& curve)
{
    Scene_vertex_t origin, size;
    curve.get_boundaries(origin, size);

    Scene_vertex_t scale(5);
    scale[0] = state_->tesseract_size[0] / size[0];
    scale[1] = state_->tesseract_size[0] / size[1];
    scale[2] = state_->tesseract_size[0] / size[2];
    scale[3] = state_->tesseract_size[0] / size[3];
    scale[4] = 1;
    curve.scale_vertices(scale);
}

//******************************************************************************
// create_tesseract
//******************************************************************************

void Scene::create_tesseract()
{
    assert(state_);
    if(state_ == nullptr)
        return;

    Scene_vertex_t origin(4), size(4);
    for(char i = 0; i < 4; ++i)
    {
        origin(i) = -0.5f * state_->tesseract_size[i];
        size(i) = state_->tesseract_size[i];
    }

    state_->tesseract = std::make_unique<Tesseract>(
        origin,
        size,
        state_->get_color(X_axis),
        state_->get_color(Y_axis),
        state_->get_color(Z_axis),
        state_->get_color(W_axis));
}
