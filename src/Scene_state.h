#pragma once
// local
#include "Curve.h"
#include "Curve_selection.h"
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

    glm::mat4 projection_3D;
    glm::quat rotation_3D;
    glm::vec3 camera_3D;

    boost::numeric::ublas::matrix<double> projection_4D;
    boost::numeric::ublas::vector<double> camera_4D;

    float xy_rot, yz_rot, zx_rot, xw_rot, yw_rot, zw_rot;

    std::shared_ptr<Curve> curve, simple_curve;
    std::shared_ptr<Curve_selection> curve_selection;

    std::shared_ptr<Tesseract> tesseract;

    const Color* get_color(int color_id);
    void update_color(int color_id, const Color& color);

    float unfolding_anim_;
    bool show_tesseract,
         show_curve,
         use_simple_dali_cross;
    std::array<float, 4> tesseract_size;

private:
    std::map<std::int32_t, std::unique_ptr<Color>> colors_;
};
