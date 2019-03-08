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

void Base_renderer::update_redering_region(glm::ivec2 pos,
                                           glm::ivec2 size)
{
    scene_pos_ = pos;
    scene_size_ = size;
}
