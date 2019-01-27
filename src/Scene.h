#pragma once
// local
#include "Scene_state.h"
// std
#include <memory>

class Scene
{
public:
    Scene(std::shared_ptr<Scene_state> state);
    void load_ode(std::string filename);

private:
    std::shared_ptr<Curve> load_curve(std::string filename);
    void create_tesseract(
        boost::numeric::ublas::vector<double> shift,
        const Curve& curve);

    std::shared_ptr<Scene_state> state_;
    boost::numeric::ublas::vector<double> c_origin;
    boost::numeric::ublas::vector<double> c_size;

    const double tesseract_size;
};
