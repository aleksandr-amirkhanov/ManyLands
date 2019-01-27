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
}
