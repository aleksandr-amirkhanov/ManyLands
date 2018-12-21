#include "Controls.h"

#include "Consts.h"

double Controls::last_x_pos_ = 0.;
double Controls::last_y_pos_ = 0.;
bool Controls::is_dragged_ = false;
glm::quat Controls::rot_ = glm::quat(glm::mat4(1.f));

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

void Controls::update(GLFWwindow* window)
{
    double x_pos, y_pos;
	glfwGetCursorPos(window, &x_pos, &y_pos);

    if(is_dragged_)
    {
        glm::vec3 axis(y_pos - last_y_pos_, x_pos - last_x_pos_, 0.f);
        float length = glm::length(axis);
        if(length != 0)
        {
            rot_ = glm::angleAxis(glm::radians(0.25f * length), glm::normalize(axis)) * rot_;
        }
    }

    last_x_pos_ = x_pos;
    last_y_pos_ = y_pos; 
}

glm::quat Controls::get_rotation_quat()
{
    return rot_;
}