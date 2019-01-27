#pragma once

#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
// local
#include "Scene_state.h"
// std
#include <memory.h>

namespace Controls
{
void set_scene_state(std::shared_ptr<Scene_state> state);
void mouse_button_callback(GLFWwindow* window,
    int button,
    int action,
    int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void update(GLFWwindow* window);

extern double last_x_pos_, last_y_pos_;
extern bool is_dragged_;
extern std::shared_ptr<Scene_state> state_;
}
