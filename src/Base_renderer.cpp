#include "Base_renderer.h"

Base_renderer::Base_renderer()
    : scene_pos_(0, 0)
    , scene_size_(0, 0)
{
}

void Base_renderer::set_state(std::shared_ptr<Scene_state> state)
{
    state_ = state;
}

void Base_renderer::set_redering_region(glm::ivec2 pos,
                                           glm::ivec2 size,
                                           float scale_x,
                                           float scale_y)
{
    scene_pos_ = pos;
    scene_size_ = size;
    display_scale_x_ = scale_x;
    display_scale_y_ = scale_y;
}
