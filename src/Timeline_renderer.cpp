#include "Timeline_renderer.h"

// Local
#include "Consts.h"
#include "Global.h"
#include "Scene_wireframe_object.h"
// glm
#include <glm/gtc/type_ptr.hpp>
// std
#include <vector>
// boost
#include <boost/geometry.hpp>
// CDT
#include "CDT.h"

//******************************************************************************
// Timeline_renderer
//******************************************************************************

Timeline_renderer::Timeline_renderer(std::shared_ptr<Scene_state> state)
    : pictogram_num_(0)
    , pictogram_size_(0.f)
    , pictogram_spacing_(0.f)
    , player_pos_(0.f)
    , track_mouse_(false)
{
    set_state(state);
}

//******************************************************************************
// set_shader
//******************************************************************************

void Timeline_renderer::set_shader(std::shared_ptr<Screen_shader> screen)
{
    screen_shader_ = screen;
}

//******************************************************************************
// render
//******************************************************************************

void Timeline_renderer::render()
{
    if(!state_->curve)
        return;

    screen_geom_ = std::make_unique<Screen_shader::Screen_geometry>();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(screen_shader_->program_id);

    glViewport(static_cast<GLint>  (display_scale_x_ * region_.left()   ),
               static_cast<GLint>(  display_scale_y_ * region_.bottom() ),
               static_cast<GLsizei>(display_scale_x_ * region_.width()  ),
               static_cast<GLsizei>(display_scale_y_ * region_.height()));

    glm::mat4 proj_ortho = glm::ortho(0.f,
                                      static_cast<float>(region_.width()),
                                      0.f,
                                      static_cast<float>(region_.height()));
    glUniformMatrix4fv(screen_shader_->proj_mat_id,
                       1,
                       GL_FALSE,
                       glm::value_ptr(proj_ortho));

    // On-screen rendering

    draw_axes(plot_region_);
    draw_curve(plot_region_);
    draw_switches(plot_region_);
    draw_marker(plot_region_);
    draw_selection(plot_region_, mouse_selection_);
    draw_pictograms(plot_region_);

    if(screen_geom_->data_array.size() > 0)
    {
        screen_geom_->init_buffers();
        screen_shader_->draw_geometry(*screen_geom_.get());
    }
}

//******************************************************************************
// process_input
//******************************************************************************

void Timeline_renderer::process_input(const Renderer_io& io)
{
    glm::vec2 mouse_pos = io.mouse_pos -
                          glm::vec2(region_.left(), region_.bottom());

    if(io.mouse_down && plot_region_.contains(mouse_pos))
    {
        track_mouse_ = true;
        mouse_selection_.is_active = true;
        mouse_selection_.start_pnt = mouse_pos;
        mouse_selection_.end_pnt = mouse_pos;
    }

    if(io.mouse_up)
    {
        track_mouse_ = false;
        mouse_selection_.is_active = false;

        make_selection(mouse_selection_);
    }

    if(track_mouse_ && glm::length(io.mouse_move) > 0)
    {
        mouse_selection_.end_pnt = mouse_pos;
    }
}

//******************************************************************************
// set_redering_region
//******************************************************************************

void Timeline_renderer::set_redering_region(Region region,
                                            float scale_x,
                                            float scale_y)
{
    Base_renderer::set_redering_region(region, scale_x, scale_y);
    update_regions();
}

//******************************************************************************
// set_pictogram_size
//******************************************************************************

void Timeline_renderer::set_pictogram_size(float size)
{
    pictogram_size_ = size;
    update_regions();
}

//******************************************************************************
// draw_axes
//******************************************************************************

void Timeline_renderer::draw_axes(const Region& region)
{
    const int num_section    = 8,
              num_subsection = 5,
              hitch_length   = 6;
    const float spacing      = 50;


    // Draw the bottom axis and the left axis
    auto draw_line = [this](glm::vec2 start, glm::vec2 end)
    {
        Screen_shader::Line_strip line;
        line.emplace_back(Screen_shader::Line_point(
            start, 1.5f, glm::vec4(0.f, 0.f, 0.f, 1.f)));
        line.emplace_back(Screen_shader::Line_point(
            end, 1.5f, glm::vec4(0.f, 0.f, 0.f, 1.f)));
        screen_shader_->append_to_geometry(*screen_geom_.get(), line);
    };

    draw_line(glm::vec2(region.left(),  region.bottom()),
              glm::vec2(region.right(), region.bottom()));
    draw_line(glm::vec2(region.left(),  region.bottom()),
              glm::vec2(region.left(),  region.top()));

    const float t_min = state_->curve->t_min();
    const float t_max = state_->curve->t_max();
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
// draw_curve
//******************************************************************************

void Timeline_renderer::draw_curve(const Region& region)
{
    const float t_min = state_->simple_curve->t_min();
    const float t_max = state_->simple_curve->t_max();
    const float t_duration = t_max - t_min;

    auto get_strip = [this](
        const Region& region,
        size_t dim_ind,
        float t_duration,
        float scale,
        const glm::vec4& norm_color,
        const glm::vec4& dim_color)
    {
        const float width = 2.5f;

        Screen_shader::Line_strip strip;

        glm::vec2 prev_pnt;
        const float t_min  = state_->simple_curve->t_min();

        const float min_delta_t = width * t_duration / region.width();
        float prev_t = -min_delta_t;

        for(size_t i = 0; i < state_->simple_curve->get_vertices().size(); i++)
        {
            const float t_curr = state_->simple_curve->time_stamp()[i];
            if(t_curr < prev_t + min_delta_t)
                continue;

            const float val = static_cast<float>(
                state_->simple_curve->get_vertices()[i](dim_ind));
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

            strip.emplace_back(
                Screen_shader::Line_point(curr_pnt, width, color));

            prev_pnt = curr_pnt;
        }

        return strip;
    };

    std::vector<glm::vec4> colors;
    // #d7191c X-axis
    colors.emplace_back(glm::vec4(0.84f, 0.10f, 0.11f, 1.00f));
    // #fdae61 Y-axis
    colors.emplace_back(glm::vec4(0.99f, 0.68f, 0.38f, 1.00f));
    // #abd9e9 Z-axis
    colors.emplace_back(glm::vec4(0.67f, 0.85f, 0.91f, 1.00f));
    // #2c7bb6 W-axis
    colors.emplace_back(glm::vec4(0.17f, 0.48f, 0.71f, 1.00f));

    //QPen defocused = pen;
    glm::vec4 defocused = glm::vec4(0.9f, 0.9f, 0.9f, 1.f);

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

        screen_shader_->append_to_geometry(*screen_geom_, strip);
    }
}

//******************************************************************************
// draw_switches
//******************************************************************************

void Timeline_renderer::draw_switches(const Region& region)
{
    const float width = 1.f;
    const glm::vec4 color(0.f, 0.f, 0.f, 0.27f);

    std::vector<float> points;
    calculate_switch_points(points, region);
    for(auto p : points)
    {
        Screen_shader::Line_strip line;
        line.emplace_back(Screen_shader::Line_point(
            glm::vec2(p, region.top()), width, color));
        line.emplace_back(Screen_shader::Line_point(
            glm::vec2(p, region.bottom()), width, color));
        screen_shader_->append_to_geometry(*screen_geom_, line);
    }
}

//******************************************************************************
// draw_marker
//******************************************************************************

void Timeline_renderer::draw_marker(const Region& region)
{
    const float width = 1.f;
    const glm::vec4 color(1.f, 0.f, 0.f, 1.f);
    
    float norm_pos = player_pos_ / (state_->curve->t_max() -
                                    state_->curve->t_min());

    float x_pos = region.left() + norm_pos * region.width();

    Screen_shader::Line_strip line;
    line.emplace_back(Screen_shader::Line_point(
        glm::vec2(x_pos, region.top()), width, color));
    line.emplace_back(Screen_shader::Line_point(
        glm::vec2(x_pos, region.bottom()), width, color));
    screen_shader_->append_to_geometry(*screen_geom_, line);
}

//******************************************************************************
// draw_selection
//******************************************************************************

void Timeline_renderer::draw_selection(const Region& region,
                                       const Mouse_selection& s)
{
    if(mouse_selection_.is_active)
    {   
        float left  = std::clamp(s.start_pnt.x, region.left(), region.right()),
              right = std::clamp(s.end_pnt.x,   region.left(), region.right());

        Screen_shader::Rectangle rect(left,
                                      region.bottom(),
                                      right,
                                      region.top(),
                                      glm::vec4(0.f, 0.68f, 0.94f, 0.20f));

        screen_shader_->append_to_geometry(*screen_geom_, rect);
    }
    else
    {
        auto width_ratio = (region.right() - region.left()) /
            state_->curve->t_duration();
        auto left = region.left() +
            state_->curve_selection->t_start * width_ratio;
        auto right = region.left() +
            state_->curve_selection->t_end   * width_ratio;

        const glm::vec4 background = glm::vec4(0.f, 0.f, 0.f, 0.07f);

        Screen_shader::Rectangle lrect(
            region.left(),
            region.bottom(),
            left,
            region.top(),
            background);

        Screen_shader::Rectangle rrect(
            right,
            region.bottom(),
            region.right(),
            region.top(),
            background);

        screen_shader_->append_to_geometry(*screen_geom_, lrect);
        screen_shader_->append_to_geometry(*screen_geom_, rrect);
    }
}

//******************************************************************************
// draw_pictograms
//******************************************************************************

void Timeline_renderer::draw_pictograms(const Region& region)
{
    if(state_->curve->get_stats().switches_inds.size() == 0)
        return;
    
    // Draw pictograms
    pictogram_num_ = state_->curve->get_stats().switches_inds.size() + 1;

    std::vector<float> switch_points;
    calculate_switch_points(switch_points, plot_region_);

    std::vector<float> switch_center;
    if(switch_points.size() > 0)
    {
        switch_center.push_back(
            0.5f * (plot_region_.left() + switch_points[0]));

        for(size_t i = 1; i < switch_points.size(); ++i)
        {
            switch_center.push_back(
                0.5f * (switch_points[i - 1] + switch_points[i]));
        }

        switch_center.push_back(
            0.5f * (switch_points.back() + plot_region_.right()));
    }

    float x_pos = 0.5f * pictogram_size_ + pictogram_spacing_;
    for(int i = 0; i < pictogram_num_; ++i)
    {
        Curve_selection selection;
        std::string dim;
        Curve_stats::Range range = state_->curve->get_stats().range[i];
        if(i == 0)
        {
            selection.t_start = state_->curve->t_min();
            selection.t_end =
                state_->curve->time_stamp()[state_->curve->get_stats()
                                                .switches_inds[i]];

            dim = state_->curve->get_stats().dimensionality.front();
        }
        else if(i == pictogram_num_ - 1)
        {
            size_t ind = state_->curve->get_stats().switches_inds[i - 1];

            selection.t_start = state_->curve->time_stamp()[ind];
            selection.t_end = state_->curve->t_max();

            dim = state_->curve->get_stats().dimensionality[ind];
        }
        else
        {
            size_t ind = state_->curve->get_stats().switches_inds[i - 1];

            selection.t_start = state_->curve->time_stamp()[ind];
            selection.t_end =
                state_->curve->time_stamp()[state_->curve->get_stats()
                                                .switches_inds[i]];

            dim = state_->curve->get_stats().dimensionality[ind];
        }

        draw_pictogram(
            glm::vec2(x_pos,
                        pictogram_region_.bottom() +
                        0.5f * pictogram_region_.height()),
            pictogram_size_,
            selection,
            dim,
            range);

        /*pen.setWidthF(1.);
        pen.setColor(QColor(0, 0, 0, 80));
        painter.setPen(pen);
        painter.setBrush(QBrush());

        glm::vec2 pictogram_p(
            x_pos, pictogram_region_.center().y() - pictogram_size_);
        glm::vec2 switch_p(switch_center[i], plot_region_.bottom());

        QPainterPath path;
        path.moveTo(switch_p);
        path.cubicTo(
            QPointF(switch_p.x(), 0.5 * (switch_p.y() + pictogram_p.y())),
            QPointF(
                pictogram_p.x(), 0.5 * (switch_p.y() + pictogram_p.y())),
            pictogram_p);
        painter.drawPath(path);*/

        x_pos += pictogram_size_ + pictogram_spacing_;
    }
}

//******************************************************************************
// draw_pictogram
//******************************************************************************

void Timeline_renderer::draw_pictogram(const glm::vec2& center,
                                       float size,
                                       const Curve_selection& seleciton,
                                       std::string dim,
                                       Curve_stats::Range range)
{
    if(!state_->tesseract)
        return;

    auto fill_wireframe_obj = [this, &center](Scene_wireframe_object& obj,
                                              float speed)
    {
        typedef CDT::Triangulation<float> Triangulation_type;
        typedef CDT::V2d<float> Vert_type;

        std::vector<Vert_type> verts;
        for(size_t i = 0; i < obj.vertices().size(); ++i)
        {
            verts.emplace_back(
                Vert_type::make(center.x + obj.vertices()[i](0),
                                center.y + obj.vertices()[i](1)));
        }


        Triangulation_type cdt;
        cdt.insertVertices(verts);
        cdt.eraseSuperTriangle();

        for(auto& t : cdt.triangles)
        {
            Screen_shader::Triangle screen_t;

            screen_t.v1 = glm::vec2(cdt.vertices[t.vertices[0]].pos.x,
                                    cdt.vertices[t.vertices[0]].pos.y);
            screen_t.v2 = glm::vec2(cdt.vertices[t.vertices[1]].pos.x,
                                    cdt.vertices[t.vertices[1]].pos.y);
            screen_t.v3 = glm::vec2(cdt.vertices[t.vertices[2]].pos.x,
                                    cdt.vertices[t.vertices[2]].pos.y);

            screen_t.color = glm::vec4(1.f, 0.f, 0.f, 0.5f);

            screen_shader_->append_to_geometry(*screen_geom_, screen_t);
        }
    };

    auto draw_wireframe_obj = [this, &center](
                                  const Scene_wireframe_object& obj) {
        for(auto& e : obj.edges())
        {
            const auto& v1 = obj.vertices()[e.vert1];
            const auto& v2 = obj.vertices()[e.vert2];

            const float width(1.f);
            const glm::vec4 color(0.f, 0.f, 0.f, 1.f);

            Screen_shader::Line_strip line;
            line.emplace_back(Screen_shader::Line_point(
                center + glm::vec2(v1(0), v1(1)), width, color));
            line.emplace_back(Screen_shader::Line_point(
                center + glm::vec2(v2(0), v2(1)), width, color));

            screen_shader_->append_to_geometry(*screen_geom_, line);
        }
    };

    auto draw_curve =
        [this, &center, &seleciton](const Curve& curve) {
            for(auto& e : curve.edges())
            {
                const auto& v1 = curve.vertices()[e.vert1];
                const auto& v2 = curve.vertices()[e.vert2];

                float t1 = curve.time_stamp()[e.vert1];
                float t2 = curve.time_stamp()[e.vert2];

                if(seleciton.in_range(t1) && seleciton.in_range(t2))
                {
                    const float width(1.f);
                    const glm::vec4 color(0.f, 0.f, 0.f, 1.f);

                    Screen_shader::Line_strip line;
                    line.emplace_back(Screen_shader::Line_point(
                        center + glm::vec2(v1(0), v1(1)), width, color));
                    line.emplace_back(Screen_shader::Line_point(
                        center + glm::vec2(v2(0), v2(1)), width, color));

                    screen_shader_->append_to_geometry(*screen_geom_, line);
                }
            }
        };

    auto get_curve_speed = [this, &seleciton](Curve& curve) {
        auto speed = std::numeric_limits<float>::min();
        for(auto& e : curve.edges())
        {
            const auto& v1 = curve.vertices()[e.vert1];
            float t = curve.time_stamp()[e.vert1];
            if(seleciton.in_range(t))
            {
                speed = std::max(speed, curve.get_stats().speed[e.vert1]);
            }
        }

        float norm_speed =
            (speed - curve.get_stats().min_speed) /
            (curve.get_stats().max_speed - curve.get_stats().min_speed);
        float log_speed = Global::log_speed(norm_speed);

        return log_speed;
    };

    std::vector<Cube> cubes = state_->tesseract->split();
    std::vector<Square> squares = Cube::split(cubes);

    /*pen.setColor(QColor("#000000"));
    pen.setWidthF(1.5);
    painter.setPen(pen);*/

    auto average_range = [&range](int i) {
        if(i == 0)
            return 0.5f * (std::get<0>(range.x) + std::get<1>(range.x));
        else if(i == 1)
            return 0.5f * (std::get<0>(range.y) + std::get<1>(range.y));
        else if(i == 2)
            return 0.5f * (std::get<0>(range.z) + std::get<1>(range.z));
        else if(i == 3)
            return 0.5f * (std::get<0>(range.w) + std::get<1>(range.w));
        
        throw std::logic_error("Invalid lambda! The variable `i` should be in "
                               "the range from 0 to 3");
        return 0.f;
    };

    std::unique_ptr<Cube> cube;
    std::unique_ptr<Square> square;
    std::unique_ptr<Tesseract> tesseract;

    if(dim == "xyz")
    {
        if(average_range(3) > 0)
            cube = std::make_unique<Cube>(cubes[0]);
        else
            cube = std::make_unique<Cube>(cubes[1]);
    }
    else if(dim == "xyw")
    {
        if(average_range(2) > 0)
            cube = std::make_unique<Cube>(cubes[2]);
        else
            cube = std::make_unique<Cube>(cubes[3]);
    }
    else if(dim == "xzw")
    {
        if(average_range(1) > 0)
            cube = std::make_unique<Cube>(cubes[5]);
        else
            cube = std::make_unique<Cube>(cubes[4]);
    }
    else if(dim == "yzw")
    {
        if(average_range(0) > 0)
            cube = std::make_unique<Cube>(cubes[7]);
        else
            cube = std::make_unique<Cube>(cubes[6]);
    }
    else if(dim.size() == 2)
    {
        // This code below generates mask to acess the right tesseract plane.
        // It might be hard to understand, sorry.

        // 'n' = not defined
        std::string mask = "nnnn";

        for(char i = 0; i < 2; ++i)
        {
            // fix axes
            if(dim[i] == 'x')
                mask[0] = 'x';
            else if(dim[i] == 'y')
                mask[1] = 'y';
            else if(dim[i] == 'z')
                mask[2] = 'z';
            else if(dim[i] == 'w')
                mask[3] = 'w';
        }

        for(char i = 0; i < 4; ++i)
        {
            if(mask[i] == 'n')
                mask[i] = average_range(i) > 0 ? '1' : '0';
        }
        square = std::make_unique<Square>(state_->tesseract->get_plain(mask));
    }
    else if(dim.size() == 4)
    {
        tesseract = std::make_unique<Tesseract>(*state_->tesseract.get());
    }

    if(cube)
    {
        // Draw a cube
        project_point_array(
            cube->get_vertices(),
            size,
            state_->tesseract_size[0]);
        fill_wireframe_obj(*cube.get(), get_curve_speed(*state_->curve.get()));
        // draw_wireframe_obj(*cube.get());
    }
    else if(square)
    {
        // Draw a plain
        project_point_array(
            square->get_vertices(),
            size,
            state_->tesseract_size[0]);
        fill_wireframe_obj(
            *square.get(),
            get_curve_speed(*state_->curve.get()));
        // draw_wireframe_obj(*square.get());
    }

    if(tesseract)
    {
        // Draw a tesseract
        project_point_array(
            tesseract->get_vertices(),
            size,
            state_->tesseract_size[0]);
        fill_wireframe_obj(
            *tesseract.get(),
            get_curve_speed(*state_->curve.get()));
        // draw_wireframe_obj(*tesseract.get());
    }

    /*pen.setColor(QColor(0, 0, 0, 63));
    pen.setWidthF(1.2);
    painter.setPen(pen);*/

    Tesseract t = *state_->tesseract.get();
    project_point_array(t.get_vertices(), size, state_->tesseract_size[0]);
    draw_wireframe_obj(t);

    /*pen.setColor(QColor("#000000"));
    pen.setWidthF(1.5);
    painter.setPen(pen);*/

    Curve c = *state_->simple_curve.get();
    project_point_array(c.get_vertices(), size, state_->tesseract_size[0]);
    draw_curve(c);
}

//******************************************************************************
// make_selection
//******************************************************************************

void Timeline_renderer::make_selection(const Mouse_selection& s)
{
    const float mouse_epsilon = 3.5f;

    if(state_->curve == nullptr)
        return;

    // Measuring the distance between start and the end point is helpful for
    // selecting from mobile devices
    if(glm::length(s.start_pnt - s.end_pnt) < mouse_epsilon)
    {
        // Reset selection
        state_->curve_selection = std::make_unique<Curve_selection>();
        state_->curve_selection->t_start =
            state_->curve->t_min();
        state_->curve_selection->t_end = state_->curve->t_max();
        return;
    }

    auto get_local_coord = [this](float x_coord) {
        return (x_coord - plot_region_.left()) / (plot_region_.width());
    };

    // Currently, we are interested only in X coordinates of the user selection
    float x_min, x_max;
    if(s.start_pnt.x < s.end_pnt.x)
    {
        // The selection was done from left to right
        x_min = get_local_coord(s.start_pnt.x);
        x_max = get_local_coord(s.end_pnt.x  );
    }
    else
    {
        // The selection was done from right to left
        x_min = get_local_coord(s.end_pnt.x  );
        x_max = get_local_coord(s.start_pnt.x);
    }

    auto t_start  = state_->curve->t_min();
    auto t_end    = state_->curve->t_max();
    auto t_length = t_end - t_start;

    // FIXME: the current approach works only if the curve is defined by points
    // with fixed delta time
    state_->curve_selection = std::make_unique<Curve_selection>();
    state_->curve_selection->t_start = t_start + x_min * t_length;
    state_->curve_selection->t_end = t_start + x_max * t_length;
}

//******************************************************************************
// calculate_switch_points
//******************************************************************************

void Timeline_renderer::calculate_switch_points(
    std::vector<float>& out_points,
    const Region& region)
{
    if(state_->curve->time_stamp().size() == 0)
        return;

    float t_min = state_->curve->t_min();
    float t_max = state_->curve->t_max();
    float t_duration = t_max - t_min;

    for(auto s : state_->curve->get_stats().switches_inds)
    {
        float x_pos = region.left() +
                      region.width() * state_->curve->time_stamp()[s] /
                      t_duration;
        out_points.push_back(x_pos);
    }
}

//******************************************************************************
// project_point
//******************************************************************************

void Timeline_renderer::project_point(
    Scene_wireframe_vertex& point,
    float size,
    float tesseract_size)
{
    if(point.size() < 4)
        return;

    const auto axis_size = size * std::sin(static_cast<float>(PI / 8));
    const auto dist_to_origin =
        0.5f * size - 2 * size * std::pow(static_cast<float>(std::sin(PI / 8)), 2);

    Scene_wireframe_vertex origin(2);
    origin(0) = dist_to_origin * std::cos(static_cast<float>(7 * PI / 8));
    origin(1) = -dist_to_origin * std::sin(static_cast<float>(7 * PI / 8));

    auto copy_p = point;

    // sin and cos of 45 degrees
    const auto sin_45 = std::sin(static_cast<float>(PI / 4));
    const auto cos_45 = std::cos(static_cast<float>(PI / 4));

    // X-axis
    point(0) = origin(0) + axis_size * (0.5f + copy_p(0) / tesseract_size);
    // Y-axis
    point(1) = origin(1) - axis_size * (0.5f + copy_p(1) / tesseract_size);
    // Z -axis
    point(0) += axis_size * (0.5f + copy_p(2) / tesseract_size) * cos_45;
    point(1) += axis_size * (0.5f + copy_p(2) / tesseract_size) * sin_45;
    // W-axis
    point(0) -= axis_size * (0.5f + copy_p(3) / tesseract_size) * cos_45;
    point(1) += axis_size * (0.5f + copy_p(3) / tesseract_size) * sin_45;
}

//******************************************************************************
// project_point_array
//******************************************************************************

void Timeline_renderer::project_point_array(
    std::vector<Scene_wireframe_vertex>& points,
    float size,
    float tesseract_size)
{
    for(auto& p : points)
        project_point(p, size, tesseract_size);
}

//******************************************************************************
// update_regions
//******************************************************************************

void Timeline_renderer::update_regions()
{
    const float margin = 10.f;

    plot_region_ = Region(margin,
                          pictogram_size_ + 2 * margin,
                          region_.width() - margin,
                          region_.height() - margin);

    pictogram_region_ = Region(margin,
                               margin,
                               region_.width() - margin,
                               pictogram_size_);
}
