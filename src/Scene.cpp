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

    // Load curves from files and calculate statistics
    std::vector<Curve> orig_curves;
    for(auto& fn: fnames)
    {
        orig_curves.push_back(*load_curve(fn).get());
        // Calculate normal curve
        /*auto curve = std::make_shared<Curve>(
            original->get_simpified_curve_RDP(cuve_min_rad));
        curve->update_stats(
            state_->stat_kernel_size,
            state_->stat_max_movement,
            state_->stat_max_value);

        state_->curves.emplace_back(std::move(curve));*/
    }

    if(orig_curves.empty())
        return;

    normalize_curves(orig_curves);

    for(auto& c: orig_curves)
    {
        auto curve = std::make_shared<Curve>(
            c.get_simpified_curve_RDP(cuve_min_rad));
        curve->update_stats(
            state_->stat_kernel_size,
            state_->stat_max_movement,
            state_->stat_max_value);

        state_->curves.emplace_back(std::move(curve));
    }

    // Create tesseract

    auto& first_curve = orig_curves.front();
    Scene_wireframe_vertex shift;
    first_curve.shift_to_origin(shift);
    // Save curve origin and size to the class members
    create_tesseract(shift, first_curve);

    // Set selection
    state_->curve_selection = std::make_unique<Curve_selection>();
    state_->curve_selection->t_start = first_curve.t_min();
    state_->curve_selection->t_end = first_curve.t_max();
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
            size_t size = vals.size();
            float t = std::stof(vals[0]);
            Scene_wireframe_vertex p(5);
            p <<= std::stof(vals[1]), std::stof(vals[2]), std::stof(vals[3]),
                std::stof(vals[4]), 1;

            curve->add_point(p, t);
        }
    }

    return curve;
}

//******************************************************************************
// normalize_curves
//******************************************************************************

void Scene::normalize_curves(std::vector<Curve>& curves)
{
    if(curves.empty())
        return;

    // Normalize to the first curve
    auto& first_curve = curves.front();


    Scene_wireframe_vertex shift;
    {
        Scene_wireframe_vertex origin(5), size(5);
        first_curve.get_boundaries(origin, size);
        shift = -0.5f * size - origin;
    }

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
    first_curve.get_boundaries(origin, size);

    for(auto& c : curves)
    {
        if(state_->scale_tesseract)
        {
            Scene_wireframe_vertex scale(5);
            scale[0] = state_->tesseract_size[0] / size[0];
            scale[1] = state_->tesseract_size[0] / size[1];
            scale[2] = state_->tesseract_size[0] / size[2];
            scale[3] = state_->tesseract_size[0] / size[3];
            scale[4] = 1;
            c.translate_vertices(shift);
            c.scale_vertices(scale);
            //c.shift_to_origin(shift);
        }
        else
        {
            size_t max_ind = longest_axis_ind(&c);
            auto vert_scale = state_->tesseract_size[max_ind] / size[max_ind];
            c.scale_vertices(vert_scale);

            for(char i = 0; i < 4; ++i)
            {
                state_->tesseract_size[i] =
                    state_->tesseract_size[max_ind] * size[i] / size[max_ind];
            }
        }
    }
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
