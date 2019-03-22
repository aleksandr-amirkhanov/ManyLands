#include "Scene_state.h"
// boost
#include <boost/numeric/ublas/assignment.hpp>

//******************************************************************************
// Scene_state
//******************************************************************************

Scene_state::Scene_state()
    : camera_3D(glm::vec3(0.f, 0.f, -3.f))
    , rotation_3D(glm::mat4(1.f))
    , projection_3D(glm::mat4(1.f))
    , camera_4D(5)
    , projection_4D(5, 5)
    , tesseract_size{1.f, 1.f, 1.f, 1.f}
    , unfolding_anim(0.f)
    , show_tesseract(true)
    , show_curve(true)
    , use_simple_dali_cross(false)
    , xy_rot(0.f)
    , yz_rot(0.f)
    , zx_rot(0.f)
    , xw_rot(0.f)
    , yw_rot(0.f)
    , zw_rot(0.f)
    , fov_y(0.f)
{
    tesseract_size[0] = tesseract_size[1] = tesseract_size[2] =
        tesseract_size[3] = 1;
}

//******************************************************************************
// get_color
//******************************************************************************

const Color &Scene_state::get_color(int color_id)
{
    return colors_[color_id];
}

//******************************************************************************
// update_color
//******************************************************************************

void Scene_state::update_color(int color_id, const Color& color)
{
    colors_[color_id] = color;
}
