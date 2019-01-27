#pragma once
// Local
#include "Curve.h"
#include "Curve_selection.h"
#include "Tesseract.h"
// boost
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
// glm
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Scene_state
{
    Scene_state();

    glm::mat4 projection_3D;
    glm::quat rotation_3D;
    glm::vec3 camera_3D;

    boost::numeric::ublas::matrix<double> projection_4D;
    boost::numeric::ublas::matrix<double> rotation_4D;
    boost::numeric::ublas::vector<double> camera_4D;

    std::shared_ptr<Curve> curve;
    std::shared_ptr<Curve> simple_curve;
    std::shared_ptr<Curve_selection> curve_selection;

    std::shared_ptr<Tesseract> tesseract;

    float tesseract_size[4];

    Color clear_color,
          x_axis_color,
          y_axis_color,
          z_axis_color,
          w_axis_color;
};
