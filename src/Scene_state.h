#pragma once
// local
#include "Curve.h"
#include "Curve_selection.h"
#include "Color.h"
#include "Tesseract.h"
// boost
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
// glm
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
// std
#include <map>

enum Scene_color : std::int32_t
{
    Background,
    X_axis,
    Y_axis,
    Z_axis,
    W_axis,
    Curve_low_speed,
    Curve_high_speed
};

class Scene_state
{
public:
    Scene_state();
    
    const Color& get_color(int color_id);
    const Color& get_curve_color(size_t curve_ind);
    void update_color(int color_id, const Color& color);

    std::shared_ptr<Curve> selected_curve();

    glm::mat4 projection_3D;
    glm::quat rotation_3D;
    glm::vec3 camera_3D;

    boost::numeric::ublas::matrix<float> projection_4D;
    Scene_wireframe_vertex camera_4D;

    float xy_rot, yz_rot, zx_rot, xw_rot, yw_rot, zw_rot;
    float fov_y;

    std::vector<std::shared_ptr<Curve>> curves;
    std::shared_ptr<Curve_selection> curve_selection;

    std::shared_ptr<Tesseract> tesseract;

    float unfolding_anim;

    // Timeplayer
    bool is_timeplayer_active;
    float timeplayer_pos;

    bool show_tesseract,
         show_curve,
         show_legend,
         use_simple_dali_cross,
         scale_tesseract;

    std::array<float, 4> tesseract_size;

private:
    std::map<std::int32_t, Color> colors_;
    std::vector<Color> curve_colors_;
};
