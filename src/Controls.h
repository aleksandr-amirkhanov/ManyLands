#pragma once

#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Controls
{
void mouse_button_callback(GLFWwindow* window,
    int button,
    int action,
    int mods);
void update(GLFWwindow* window);
glm::quat get_rotation_quat();

extern double last_x_pos_, last_y_pos_;
extern bool is_dragged_;
extern glm::quat rot_;
}
