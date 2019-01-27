#include "Scene.h"
// std
#include <fstream>
// boost
#include <boost/algorithm/string.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/assignment.hpp>

using namespace boost::numeric::ublas;

Scene::Scene(std::shared_ptr<Scene_state> state)
    : tesseract_size(200.)
    , state_(state)
{
}

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
        new Curve(state_->curve->get_simpified_curve(20.))
        );
    state_->simple_curve->update_stats();

    //gui_.TimeLine->set_model(tesseract_, curve_, tesseract_size_);

    // update_marker();
    // update_plot();
}

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
            double t = std::stod(vals[0]);
            vector<double> p(5);
            p <<= std::stod(vals[1]), std::stod(vals[2]), std::stod(vals[3]),
                std::stod(vals[4]), 1;

            curve->add_point(p, t);
        }
    }

    boost::numeric::ublas::vector<double> shift;
    curve->shift_to_origin(tesseract_size, shift);

    auto longest_axis_length = [](Curve* c) {
        boost::numeric::ublas::vector<double> origin, size;
        c->get_boundaries(origin, size);

        double max_size = size[0];
        for(int i = 1; i < 4; ++i)
            if(size[i] > max_size)
                max_size = size[i];

        return max_size;
    };

    boost::numeric::ublas::vector<double> origin, size;
    curve->get_boundaries(origin, size);

    const int tesseract_scale_mode = 1;
    if(tesseract_scale_mode == 0)
    {
        double max_size = longest_axis_length(curve.get());
        curve->scale_vertices(tesseract_size / max_size);
    }
    else if(tesseract_scale_mode == 1)
    {
        boost::numeric::ublas::vector<double> scale(5);
        scale[0] = tesseract_size / size[0];
        scale[1] = tesseract_size / size[1];
        scale[2] = tesseract_size / size[2];
        scale[3] = tesseract_size / size[3];
        scale[4] = 1;
        curve->scale_vertices(scale);
        curve->shift_to_origin(tesseract_size, shift);
    }

    // Save curve origin and size to the class members
    curve->get_boundaries(c_origin, c_size);
    create_tesseract(shift, *curve.get());

    // Set selection
    state_->curve_selection = std::make_unique<Curve_selection>();
    state_->curve_selection->t_start = curve->get_time_stamp().front();
    state_->curve_selection->t_end = curve->get_time_stamp().back();

    if(curve->get_time_stamp().size() > 2)
    {
        // Set UI
        float min = state_->curve_selection->t_start;
        float max = state_->curve_selection->t_end;

        //gui_.outTMin->setText(QString::number(min, 'f', 2));
        //gui_.outTMax->setText(QString::number(max, 'f', 2));

        //gui_.tIntervalStart->setMinimum(min);
        //gui_.tIntervalStart->setMaximum(max);

        //gui_.tIntervalEnd->setMinimum(min);
        //gui_.tIntervalEnd->setMaximum(max);
    }

    return curve;
}

void Scene::create_tesseract(
    boost::numeric::ublas::vector<double> shift,
    const Curve& curve)
{
    assert(state_);
    if(state_ == nullptr)
        return;

    vector<double> origin(4);
    vector<double> size(4);
    curve.get_boundaries(origin, size);

    state_->tesseract = std::make_unique<Tesseract>(
        origin,
        size,
        state_->get_color(X_axis),
        state_->get_color(Y_axis),
        state_->get_color(Z_axis),
        state_->get_color(W_axis) );
}
