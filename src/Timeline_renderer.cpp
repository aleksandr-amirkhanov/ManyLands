#include "Timeline_renderer.h"
// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

Timeline_renderer::Timeline_renderer(std::shared_ptr<Scene_state> state)
{
    set_state(state);
}

void Timeline_renderer::set_shader(std::shared_ptr<Screen_shader> screen)
{
    screen_shader = screen;
}

void Timeline_renderer::render()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(screen_shader->program_id);

    // On-screen rendering
    glm::mat4 proj_ortho = glm::ortho(-1.f, 1.f, -1.f, 1.f);
    glUniformMatrix4fv(screen_shader->proj_mat_id,
                       1,
                       GL_FALSE,
                       glm::value_ptr(proj_ortho));

    Line_2D line;
    line.start_pos = glm::vec2(-1.f, -1.f);
    line.end_pos   = glm::vec2(1.f, 1.f);
    line.width     = 1.;
    line.color     = glm::vec4(1.f, 0.f, 0.f, 0.2f);

    glLineWidth(5.f);
    screen_shader->draw_line_geometry(
        screen_shader->create_line_geometry(line));
}
