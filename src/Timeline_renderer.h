#pragma once
// Local
#include "Base_renderer.h"
#include "Geometry_engine.h"
#include "Scene_state.h"
#include "Line_2D.h"
#include "Screen_shader.h"
// std
#include <memory.h>

class Timeline_renderer : public Base_renderer
{
private:
    struct Line_array
    {
        glm::vec2 vert;
        glm::vec4 color;
    };

    typedef Geometry_engine<Line_array> Line_geometry;

    Timeline_renderer() = delete;

public:
    Timeline_renderer(std::shared_ptr<Scene_state> state);

    void set_shader(std::shared_ptr<Screen_shader> screen);
    void render() override;

private:
    std::shared_ptr<Screen_shader> screen_shader;

};
