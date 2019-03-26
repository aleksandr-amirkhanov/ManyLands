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

void Scene::load_ode(std::string filename)
{
    assert(state_);
    if(state_ == nullptr)
        return;

    // Load curve from the file and calculate statistics for the curve
    state_->curve = load_curve(filename);
    state_->curve->update_stats();

    // Simplify the curve and calculate statistics for the simplified curve
    state_->simple_curve = std::shared_ptr<Curve>(
        new Curve(state_->curve->get_simpified_curve(10.f))
        );
    state_->simple_curve->update_stats();

    //gui_.TimeLine->set_model(tesseract_, curve_, tesseract_size_);

    // update_marker();
    // update_plot();
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

    const int tesseract_scale_mode = 0;
    if(tesseract_scale_mode == 0)
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
    else if(tesseract_scale_mode == 1)
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
