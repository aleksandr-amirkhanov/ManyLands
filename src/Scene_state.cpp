#include "Scene_state.h"
// boost
#include <boost/numeric/ublas/assignment.hpp>

Scene_state::Scene_state()
    : camera_3D(glm::vec3(0.f, 0.f, -3.f))
    , rotation_3D(glm::mat4(1.f))
    , projection_3D(glm::mat4(1.f))
    , camera_4D(5)
    , rotation_4D(5, 5)
    , projection_4D(5, 5)
    , tesseract_size{1.f, 1.f, 1.f, 1.f}
{
    colors_[Background]       = std::make_unique<Color>();
    colors_[X_axis]           = std::make_unique<Color>();
    colors_[Y_axis]           = std::make_unique<Color>();
    colors_[Z_axis]           = std::make_unique<Color>();
    colors_[W_axis]           = std::make_unique<Color>();
    colors_[Curve_low_speed]  = std::make_unique<Color>();
    colors_[Curve_high_speed] = std::make_unique<Color>();
}

const Color* Scene_state::get_color(int color_id)
{
    return colors_[color_id].get();
}

void Scene_state::update_color(int color_id, const Color& color)
{
    colors_[color_id]->copy(color);
}
