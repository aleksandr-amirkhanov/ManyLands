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
    , tesseract_size{200.f, 200.f, 200.f, 200.f}
    , unfolding_anim(0.f)
    , show_tesseract(true)
    , show_curve(true)
    , show_legend(true)
    , use_simple_dali_cross(false)
    , xy_rot(0.f)
    , yz_rot(0.f)
    , zx_rot(0.f)
    , xw_rot(0.f)
    , yw_rot(0.f)
    , zw_rot(0.f)
    , fov_y(0.f)
    , is_timeplayer_active(true)
    , timeplayer_pos(0.f)
    , scale_tesseract(true)
    , use_unique_curve_colors(false)
{
    curve_colors_.emplace_back(Color(228,  26,  28));
    curve_colors_.emplace_back(Color( 55, 126, 184));
    curve_colors_.emplace_back(Color( 77, 175,  74));
    curve_colors_.emplace_back(Color(152,  78, 163));
    curve_colors_.emplace_back(Color(255, 127,   0));
    curve_colors_.emplace_back(Color(255, 255,  51));
    curve_colors_.emplace_back(Color(166,  86,  40));
    curve_colors_.emplace_back(Color(247, 129, 191));
}

//******************************************************************************
// get_color
//******************************************************************************

const Color &Scene_state::get_color(int color_id)
{
    return colors_[color_id];
}


//******************************************************************************
// get_curve_color
//******************************************************************************

const Color &Scene_state::get_curve_color(size_t curve_ind)
{
    return curve_colors_[curve_ind % curve_colors_.size()];
}

//******************************************************************************
// update_color
//******************************************************************************

void Scene_state::update_color(int color_id, const Color& color)
{
    colors_[color_id] = color;
}

//******************************************************************************
// selected_curve
//******************************************************************************

std::shared_ptr<Curve> Scene_state::selected_curve()
{
    if(curves.empty())
        return nullptr;
    else
        return curves.front();
}
