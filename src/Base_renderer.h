#pragma once

// Local
#include "Scene_state.h"
// glm
#include <glm/glm.hpp>
// std
#include <memory>

class Base_renderer
{
public:
    Base_renderer();

    virtual void set_state(std::shared_ptr<Scene_state> state);
    virtual void update_redering_region(glm::ivec2 pos,
                                        glm::ivec2 size);
    virtual void render() = 0;

protected:
    std::shared_ptr<Scene_state> state_;
    glm::ivec2 scene_pos_, scene_size_;
};
