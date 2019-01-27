#include "Controls.h"

#include "Consts.h"

double Controls::last_x_pos_ = 0.;
double Controls::last_y_pos_ = 0.;
bool Controls::is_dragged_ = false;
std::shared_ptr<Scene_state> Controls::state_;

void Controls::set_scene_state(std::shared_ptr<Scene_state> state)
{
    state_ = state;
}

void Controls::mouse_button_callback(GLFWwindow* window,
    int button,
    int action,
    int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        is_dragged_ = true;
    else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        is_dragged_ = false;
}

void Controls::scroll_callback(GLFWwindow* window,
    double xoffset,
    double yoffset)
{
    if(!state_)
        return;

    state_->camera_3D.z += yoffset * 0.3f;
}

void Controls::update(GLFWwindow* window)
{
    if(!state_)
        return;

    double x_pos, y_pos;
	glfwGetCursorPos(window, &x_pos, &y_pos);

    if(is_dragged_)
    {
        glm::vec3 axis(y_pos - last_y_pos_, x_pos - last_x_pos_, 0.f);
        float length = glm::length(axis);
        if(length != 0)
        {
            state_->rotation_3D = 
                glm::angleAxis(
                    glm::radians(0.25f * length),
                    glm::normalize(axis)) *
                state_->rotation_3D;
        }
    }

    last_x_pos_ = x_pos;
    last_y_pos_ = y_pos; 
}
