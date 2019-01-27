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
    boost::numeric::ublas::matrix<double> rotation_4D;
    boost::numeric::ublas::vector<double> camera_4D;

    std::shared_ptr<Curve>           curve;
    std::shared_ptr<Curve>           simple_curve;
    std::shared_ptr<Curve_selection> curve_selection;

    std::shared_ptr<Tesseract> tesseract;

    float tesseract_size[4];

    const Color* get_color(int color_id);
    void update_color(int color_id, const Color& color);

private:
    std::map<std::int32_t, std::unique_ptr<Color>> colors_;
};
