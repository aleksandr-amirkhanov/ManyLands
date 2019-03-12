#include "Timeline_renderer.h"

// Local
#include "Consts.h"
#include "Line.h"
// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
// std
#include <vector>

//******************************************************************************

Timeline_renderer::Timeline_renderer(std::shared_ptr<Scene_state> state)
    : pictogram_num_(0)
    , pictogram_size_(60.f)
    , pictogram_spacing_(0.f)
{
    set_state(state);
}

//******************************************************************************

void Timeline_renderer::set_shader(std::shared_ptr<Screen_shader> screen)
{
    screen_shader = screen;
}

//******************************************************************************

void Timeline_renderer::render()
{
    if(!state_->curve)
        return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(screen_shader->program_id);

    glViewport(display_scale_x_ * region_.left(),
               display_scale_y_ * region_.bottom(),
               display_scale_x_ * region_.width(),
               display_scale_y_ * region_.height());

    // On-screen rendering
    const float margin = 10.f;
    Rect region(margin,
                margin,
                region_.width() - margin,
                region_.height() - margin);
    draw_axes(region);
    draw_curve(region);

    glm::mat4 proj_ortho = glm::ortho(0.f,
                                      static_cast<float>(region_.width()),
                                      0.f,
                                      static_cast<float>(region_.height()));
    glUniformMatrix4fv(screen_shader->proj_mat_id,
                       1,
                       GL_FALSE,
                       glm::value_ptr(proj_ortho));

    // Draw tesseract
    /*glm::vec2 center =  0.5f * glm::vec2(region_.width(), region_.height());
    auto draw_wireframe_obj = [this, &center](
                                  const Wireframe_object& obj) {
        for(auto& e : obj.edges())
        {
            const auto& v1 = obj.vertices()[e.vert1];
            const auto& v2 = obj.vertices()[e.vert2];

            Line_2D line;
            line.start_pos = center + glm::vec2(v1(0), v1(1));
            line.end_pos   = center + glm::vec2(v2(0), v2(1));
            line.width     = 1.5;
            line.color     = glm::vec4(1.f, 0.f, 0.f, 1.f);

            screen_shader->draw_line_geometry(
                screen_shader->create_line_geometry(line));
        }
    };

    if(state_->tesseract)
    {
        auto t = *state_->tesseract.get();
        project_point_array(t.get_vertices(), 60., state_->tesseract_size[0]);
        draw_wireframe_obj(t);
    }*/
}

//******************************************************************************

void Timeline_renderer::draw_axes(const Rect& region)
{
    const int num_section    = 8,
              num_subsection = 5,
              hitch_length   = 6;
    const float spacing      = 50;


    // Draw the bottom axis and the left axis
    auto draw_line = [this](glm::vec2 start, glm::vec2 end)
    {
        Line_strip line;
        line.push_back(Line_point(start, 1.5f, glm::vec4(0.f, 0.f, 0.f, 1.f)));
        line.push_back(Line_point(end,   1.5f, glm::vec4(0.f, 0.f, 0.f, 1.f)));
        screen_shader->draw_line_geometry(
            screen_shader->create_line_geometry(line));
    };

    draw_line(glm::vec2(region.left(),  region.bottom()),
              glm::vec2(region.right(), region.bottom()));
    draw_line(glm::vec2(region.left(),  region.bottom()),
              glm::vec2(region.left(),  region.top()));

    const float t_min = state_->curve->get_time_stamp().front();
    const float t_max = state_->curve->get_time_stamp().back();
    const float t_durr = t_max - t_min;

    float dist = region.width() / num_section;
    float sub_dist = dist / num_subsection;

    for(int i = 0; i <= num_section; ++i)
    {
        float x_pos = region.left() + i * dist;
        draw_line(glm::vec2(x_pos, region.bottom()),
                  glm::vec2(x_pos, region.bottom() - hitch_length));

        if(i == num_section)
            continue;

        for(int j = 0; j <= num_subsection; ++j)
        {
            float x_sub_pos = x_pos + j * sub_dist;
            draw_line(glm::vec2(x_sub_pos, region.bottom()),
                      glm::vec2(x_sub_pos, region.bottom() - hitch_length / 2));
        }

        /*painter.drawText(
            QRectF(
                x_pos - spacing / 2,
                region_.bottom() + hitch_length,
                spacing,
                spacing),
            Qt::AlignHCenter | Qt::AlignTop,
            QString::number(t_min + i * t_durr / num_section, 'f', 0));*/
    }
}

//******************************************************************************

void Timeline_renderer::draw_curve(const Rect& region)
{
    const float t_min = state_->curve->get_time_stamp().front();
    const float t_max = state_->curve->get_time_stamp().back();
    const float t_duration = t_max - t_min;

    auto get_strip = [this](
        const Rect& region,
        size_t dim_ind,
        float t_duration,
        float scale,
        const glm::vec4& norm_color,
        const glm::vec4& dim_color)
    {
        const float width = 2.5f;

        Line_strip strip;

        glm::vec2 prev_pnt;
        const float t_min  = state_->curve->get_time_stamp().front();

        const float min_delta_t = t_duration / region.width();
        float prev_t = -min_delta_t;

        for(size_t i = 0; i < state_->curve->get_vertices().size(); i++)
        {
            const float t_curr = state_->curve->get_time_stamp()[i];
            if(t_curr < prev_t + min_delta_t)
                continue;

            const float val = static_cast<float>(
                state_->curve->get_vertices()[i](dim_ind));
            const float x_point =
                region.left() + region.width() * (t_curr - t_min) / t_duration;
            const float y_point =
                region.bottom() + region.height() * (0.5f + val / scale);
            glm::vec2 curr_pnt(x_point, y_point);
            
            glm::vec4 color;
            if(state_->curve_selection != nullptr)
            {
                if(state_->curve_selection->t_start <= t_curr &&
                    t_curr <= state_->curve_selection->t_end)
                {
                    color = norm_color;
                }
                else
                {
                    color = dim_color;
                }
            }
            else
            {
                color = norm_color;
            }

            strip.push_back(Line_point(curr_pnt, width, color));

            prev_pnt = curr_pnt;
        }

        return strip;
    };

    std::vector<glm::vec4> colors;
    colors.push_back(glm::vec4(0.84f, 0.10f, 0.11f, 1.00f)); // #d7191c X-axis
    colors.push_back(glm::vec4(0.99f, 0.68f, 0.38f, 1.00f)); // #fdae61 Y-axis
    colors.push_back(glm::vec4(0.67f, 0.85f, 0.91f, 1.00f)); // #abd9e9 Z-axis
    colors.push_back(glm::vec4(0.17f, 0.48f, 0.71f, 1.00f)); // #2c7bb6 W-axis

    //QPen defocused = pen;
    glm::vec4 defocused = glm::vec4(0.8f, 0.8f, 0.8f, 1.f); // #cccccc

    for(size_t i = 0; i < 4; ++i)
    {
        //pen.setColor(colors[i]);
        //pen.setWidthF(2.5);

        auto strip = get_strip(region,
                               i,
                               t_duration,
                               state_->tesseract_size[0],
                               colors[i],
                               defocused);

        screen_shader->draw_line_geometry(
            screen_shader->create_line_geometry(strip));
    }
}

//******************************************************************************

void Timeline_renderer::project_point(
    boost::numeric::ublas::vector<double>& point,
    double size,
    double tesseract_size)
{
    if(point.size() < 4)
        return;

    const double axis_size = 2 * size * std::sin(PI / 8);
    const float dist_to_origin =
        size - 4 * size * std::pow(std::sin(PI / 8), 2);

    boost::numeric::ublas::vector<double> origin(2);
    origin(0) = dist_to_origin * std::cos(7 * PI / 8);
    origin(1) = -dist_to_origin * std::sin(7 * PI / 8);

    auto copy_p = point;

    // sin and cos of 45 degrees
    const double sin_45 = std::sin(PI / 4);
    const double cos_45 = std::cos(PI / 4);

    // X-axis
    point(0) = origin(0) + axis_size * (0.5 + copy_p(0) / tesseract_size);
    // Y-axis
    point(1) = origin(1) - axis_size * (0.5 + copy_p(1) / tesseract_size);
    // Z -axis
    point(0) += axis_size * (0.5 + copy_p(2) / tesseract_size) * cos_45;
    point(1) += axis_size * (0.5 + copy_p(2) / tesseract_size) * sin_45;
    // W-axis
    point(0) -= axis_size * (0.5 + copy_p(3) / tesseract_size) * cos_45;
    point(1) += axis_size * (0.5 + copy_p(3) / tesseract_size) * sin_45;
}

//******************************************************************************

void Timeline_renderer::project_point_array(
    std::vector<boost::numeric::ublas::vector<double>>& points,
    double size,
    double tesseract_size)
{
    for(auto& p : points)
        project_point(p, size, tesseract_size);
}