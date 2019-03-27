#include "Scene.h"
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
    float cuve_min_rad)
{
    assert(state_);
    if(state_ == nullptr)
        return;

    state_->curves.clear();

    // Load curves from files and calculate statistics
    for(auto& fn: fnames)
    {
        auto original = load_curve(fn);

        // Calculate normal curve
        auto curve = std::make_shared<Curve>(
            original->get_simpified_curve_RDP(cuve_min_rad));
        curve->update_stats();

        state_->curves.emplace_back(std::move(curve));
    }
}

//******************************************************************************
// load_curve
//******************************************************************************

std::shared_ptr<Curve> Scene::load_curve(std::string filename)
{
    std::ifstream stream(filename);
    if(!stream.is_open())
        return nullptr;

    auto curve = std::make_shared<Curve>();

    std::string line;
    while(getline(stream, line))
    {
        std::vector<std::string> vals;
        boost::split(vals, line, boost::is_any_of("\t, "));
        if(vals.size() == 5)
        {
            size_t size = vals.size();
            float t = std::stof(vals[0]);
            Scene_wireframe_vertex p(5);
            p <<= std::stof(vals[1]), std::stof(vals[2]), std::stof(vals[3]),
                std::stof(vals[4]), 1;

            curve->add_point(p, t);
        }
    }

    Scene_wireframe_vertex shift;
    curve->shift_to_origin(shift);

    auto longest_axis_ind = [](Curve* c)
    {
        Scene_wireframe_vertex origin, size;
        c->get_boundaries(origin, size);

        size_t ind = 0;
        float max_size = size[0];
        for(char i = 1; i < 4; ++i)
        {
            if(size[i] > max_size)
            {
                ind = i;
                max_size = size[i];
            }
        }

        return ind;
    };

    Scene_wireframe_vertex origin, size;
    curve->get_boundaries(origin, size);

    if(state_->scale_tesseract)
    {
        Scene_wireframe_vertex scale(5);
        scale[0] = state_->tesseract_size[0] / size[0];
        scale[1] = state_->tesseract_size[0] / size[1];
        scale[2] = state_->tesseract_size[0] / size[2];
        scale[3] = state_->tesseract_size[0] / size[3];
        scale[4] = 1;
        curve->scale_vertices(scale);
        curve->shift_to_origin(shift);
    }
    else
    {
        size_t max_ind = longest_axis_ind(curve.get());
        auto vert_scale = state_->tesseract_size[max_ind] / size[max_ind];
        curve->scale_vertices(vert_scale);

        for(char i = 0; i < 4; ++i)
        {
            state_->tesseract_size[i] =
                state_->tesseract_size[max_ind] * size[i] / size[max_ind];
        }

    }

    // Save curve origin and size to the class members
    curve->get_boundaries(c_origin, c_size);
    create_tesseract(shift, *curve.get());

    // Set selection
    state_->curve_selection = std::make_unique<Curve_selection>();
    state_->curve_selection->t_start = curve->t_min();
    state_->curve_selection->t_end = curve->t_max();

    return curve;
}

//******************************************************************************
// create_tesseract
//******************************************************************************

void Scene::create_tesseract(Scene_wireframe_vertex shift,
                             const Curve& curve)
{
    assert(state_);
    if(state_ == nullptr)
        return;

    Scene_wireframe_vertex origin(4), size(4);
    curve.get_boundaries(origin, size);

    state_->tesseract = std::make_unique<Tesseract>(
        origin,
        size,
        state_->get_color(X_axis),
        state_->get_color(Y_axis),
        state_->get_color(Z_axis),
        state_->get_color(W_axis));
}
