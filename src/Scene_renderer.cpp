#include "Scene_renderer.h"
// local
#include "Consts.h"
#include "Mesh_generator.h"
#include "Matrix_lib.h"
// boost
#include <boost/numeric/ublas/assignment.hpp>
// std
#include <stdexcept>
// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
// ImGui
#include "imgui.h"

//******************************************************************************
// Scene_renderer
//******************************************************************************

Scene_renderer::Scene_renderer(std::shared_ptr<Scene_state> state)
    : Base_renderer()
    , tesseract_thickness_(1.f)
    , curve_thickness_(1.f)
    , sphere_diameter_(1.f)
    , number_of_animations_(6)
    , visibility_mask_(0)
    , track_mouse_(false)
    , filter_arrow_annotations_(true)
    , show_labels_(true)
{
    set_state(state);
}

//******************************************************************************
// set_shaders
//******************************************************************************

void Scene_renderer::set_shaders(std::shared_ptr<Diffuse_shader> diffuse,
                                 std::shared_ptr<Screen_shader> screen)
{
    diffuse_shader_ = diffuse;
    screen_shader_  = screen;
}

//******************************************************************************
// set_text_renderer
//******************************************************************************

void Scene_renderer::set_text_renderer(std::shared_ptr<Text_renderer> tex_ren)
{
    text_renderer_ = tex_ren;
}

//******************************************************************************
// render
//******************************************************************************

void Scene_renderer::render()
{
    // Scene rendering ---------------------------------------------------------

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    
    if(state_            == nullptr ||
       state_->tesseract == nullptr ||
       state_->curve     == nullptr)
    {
        return;
    }

    back_geometry_  = std::make_unique<Diffuse_shader::Mesh_geometry>();
    front_geometry_ = std::make_unique<Diffuse_shader::Mesh_geometry>();
    screen_geometry_ = std::make_unique<Screen_shader::Screen_geometry>();

    label_points_.clear();

    glUseProgram(diffuse_shader_->program_id);

    glViewport(static_cast<GLint>(display_scale_x_ * region_.left()),
               static_cast<GLint>(display_scale_y_ * region_.bottom()),
               static_cast<GLsizei>(display_scale_x_ * region_.width()),
               static_cast<GLsizei>(display_scale_y_ * region_.height()));

    std::vector<float> anims =
        split_animation(state_->unfolding_anim, number_of_animations_);
    float hide_4D = anims[0],
          project_curve_4D = anims[1],
          unfold_4D = anims[2],
          hide_3D = anims[3],
          project_curve_3D = anims[4],
          unfold_3D = anims[5];

    glm::mat4 proj_mat = glm::perspective(
        state_->fov_y,
        region_.width() / region_.height(),
        0.1f,
        100.f);
    auto camera_mat = glm::translate(glm::mat4(1.f), state_->camera_3D);
    glm::vec3 light_pos(0.f, 0.f, 70.f);
    auto world_mat = glm::toMat4(glm::lerp(state_->rotation_3D,
                                           glm::quat(),
                                           static_cast<float>(unfold_3D)));
    auto norm_mat = glm::transpose(glm::inverse(glm::mat3(world_mat)));

    // Chache the Model-view-projection matrix for arrow drawing
    auto mvp_mat = proj_mat * camera_mat * world_mat;

    glUniformMatrix4fv(diffuse_shader_->proj_mat_id,
                       1,
                       GL_FALSE,
                       glm::value_ptr(proj_mat));
    glUniformMatrix4fv(diffuse_shader_->mv_mat_id,
                       1,
                       GL_FALSE,
                       glm::value_ptr(camera_mat * world_mat));
    glUniformMatrix3fv(diffuse_shader_->normal_mat_id,
                       1,
                       GL_FALSE,
                       glm::value_ptr(norm_mat));
    glUniform3fv(diffuse_shader_->light_pos_id,
                 1,
                 glm::value_ptr(light_pos));
    glUniform2fv(diffuse_shader_->fog_range_id,
                 1,
                 glm::value_ptr(fog_range_));

    //gui_.Renderer->remove_all_meshes();
    //gui_.distanceWarning->hide();
    //gui_.Renderer->show_labels(false);
    //gui_.Renderer->remove_annotation_points();

    auto rot_m = get_rotation_matrix();

    // Project the tesseract from 4D to 3D
    Scene_wireframe_object projected_t = *state_->tesseract.get();
    project_to_3D(projected_t.get_vertices(), rot_m);

    // Choosing the high-resolution or the low-resolution curve
    Curve projected_c;

    // Array of curves used to project into 3D spaces
    std::vector<Curve> curves_3D;

    projected_c = *state_->curve.get();
    for(int i = 0; i < 8; ++i)
        curves_3D.push_back(*state_->curve.get());    

    // Project curves from 4D to 3D
    project_to_3D(projected_c.get_vertices(), rot_m);

    // Animation unfolding the tesseract to the Dali-cross
    if(state_->unfolding_anim == 0)
    {
        // Draw tesseract
        if(state_->show_tesseract)
            draw_tesseract(projected_t);

        // Draw 4D curve
        if(state_->show_curve)
        {
            draw_curve(projected_c, 1.);
            draw_annotations(projected_c, mvp_mat);
        }
    }
    else
    {
        std::vector<Cube> plots_3D = state_->tesseract->split();

        move_curves_to_3D_plots(project_curve_4D, curves_3D);

        if(unfold_4D > 0)
            tesseract_unfolding(unfold_4D, plots_3D, curves_3D);

        // Project 3D plots from 4D to 3D
        auto rot = get_rotation_matrix(unfold_4D);
        for(auto& p : plots_3D)
            project_to_3D(p.get_vertices(), rot);

        auto visibility_coeff = [this](size_t i) {
            if(visibility_mask_ == 0 || visibility_mask_ & 1 << i)
                return 1.f;
            else
                return 0.1f;
        };

        if(0 < hide_4D && project_curve_3D == 0)
        {
            // Draw 3D plots
            if(state_->show_tesseract)
            {
                for(size_t i = 0; i < plots_3D.size(); ++i)
                {
                    auto& c = plots_3D[i];
                    if(state_->use_simple_dali_cross && i != 1 &&
                       i != 2 && i != 5 && i != 7)
                    {
                        // Sometimes it is dangerous to draw to transparent
                        // objects with direrent at different animation stages,
                        // because they both are drawn first. Therefore we make
                        // this ckecup here to avoid possible drawing issues
                        if(hide_4D != 1)
                        {
                            draw_3D_plot(
                                c, visibility_coeff(i) * (1.f - hide_4D));
                        }
                    }
                    else
                    {
                        draw_3D_plot(c, visibility_coeff(i) * (1.f - hide_3D));
                    }
                }
            }

            // Draw curves
            if(state_->show_curve)
            {
                // for(auto& c : curves_3D)
                for(size_t i = 0; i < curves_3D.size(); ++i)
                {
                    if(state_->use_simple_dali_cross && i != 1 &&
                       i != 2 && i != 5 && i != 7)
                    {
                        continue;
                    }

                    auto c = curves_3D[i];
                    project_to_3D(c.get_vertices(), rot);

                    draw_curve(c, visibility_coeff(i) * (1.f - hide_3D));
                    if(visibility_coeff(i) == 1. && hide_3D < 0.5)
                        draw_annotations(c, mvp_mat);
                }
            }
        }

        // Create meshes for the cubes representign the Tesseract / Dali-cross
        if(hide_3D > 0)
        {
            // Get the source plots
            std::vector<Square> plots_2D = Cube::split(plots_3D);

            std::vector<Curve> curves_2D;
            curves_2D.push_back(curves_3D[5]);
            curves_2D.push_back(curves_3D[1]);
            curves_2D.push_back(curves_3D[7]);
            curves_2D.push_back(curves_3D[1]);
            curves_2D.push_back(curves_3D[7]);
            curves_2D.push_back(curves_3D[7]);

            move_curves_to_2D_plots(project_curve_3D, curves_2D);
            for(auto& c : curves_2D)
            {
                project_to_3D(c.get_vertices(), rot);
            }

            plots_unfolding(unfold_3D, plots_2D, curves_2D);

            // Draw 2D plots
            if(state_->show_tesseract)
            {
                for(auto& p : plots_2D)
                    draw_2D_plot(p);
            }

            label_points_.push_back(plots_2D[0].get_vertices()[3]);
            label_points_.push_back(plots_2D[3].get_vertices()[0]);
            label_points_.push_back(plots_2D[5].get_vertices()[1]);

            // Draw 2D curves
            if(state_->show_curve)
            {
                // TODO: uncomment lines below (rewrite it first ;) )!
                /*if(unfold_3D == 1.)
                    gui_.Renderer->show_labels(true);*/

                for(auto& c : curves_2D)
                {
                    draw_curve(c, 1.);
                    draw_annotations(c, mvp_mat);
                }
            }
        }
    }

    if(back_geometry_->data_array.size() > 0)
    {
        back_geometry_->init_buffers();
        diffuse_shader_->draw_geometry(back_geometry_);
    }
    if(front_geometry_->data_array.size() > 0)
    {
        front_geometry_->init_buffers();
        diffuse_shader_->draw_geometry(front_geometry_);
    }

    // On screen rendering -----------------------------------------------------

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

    if(state_->show_legend)
        draw_legend(region_);

    if(show_labels_ && unfold_3D > 0.66f)
        draw_labels_in_2D(mvp_mat);

    screen_geometry_->init_buffers();
    screen_shader_->draw_geometry(*screen_geometry_.get());
}

//******************************************************************************
// process_input
//******************************************************************************

void Scene_renderer::process_input(const Base_renderer::Renderer_io& io)
{
    if(io.mouse_down && region_.contains(io.mouse_pos))
        track_mouse_ = true;

    if(io.mouse_up)
        track_mouse_ = false;

    if(track_mouse_ && glm::length(io.mouse_move) > 0)
    {
        glm::vec3 axis(-io.mouse_move.y, io.mouse_move.x, 0.f);
        float length = glm::length(axis);
        state_->rotation_3D =
            glm::angleAxis(
                glm::radians(0.25f * length),
                glm::normalize(axis)) *
            state_->rotation_3D;
    }

    if(io.mouse_wheel && region_.contains(io.mouse_pos))
        state_->camera_3D.z += io.mouse_wheel_y * 0.05f;

    if(io.key_pressed)
    {
        switch(io.key)
        {
        case Base_renderer::Renderer_io::Key_0:
            visibility_mask_ = 0;
            break;
        case Base_renderer::Renderer_io::Key_1:
            visibility_mask_ ^= 1 << 0;
            break;
        case Base_renderer::Renderer_io::Key_2:
            visibility_mask_ ^= 1 << 1;
            break;
        case Base_renderer::Renderer_io::Key_3:
            visibility_mask_ ^= 1 << 2;
            break;
        case Base_renderer::Renderer_io::Key_4:
            visibility_mask_ ^= 1 << 3;
            break;
        case Base_renderer::Renderer_io::Key_5:
            visibility_mask_ ^= 1 << 4;
            break;
        case Base_renderer::Renderer_io::Key_6:
            visibility_mask_ ^= 1 << 5;
            break;
        case Base_renderer::Renderer_io::Key_7:
            visibility_mask_ ^= 1 << 6;
            break;
        case Base_renderer::Renderer_io::Key_8:
            visibility_mask_ ^= 1 << 7;
            break;
        case Base_renderer::Renderer_io::Key_F1:
            visibility_mask_ = 1 << 0;
            break;
        case Base_renderer::Renderer_io::Key_F2:
            visibility_mask_ = 1 << 1;
            break;
        case Base_renderer::Renderer_io::Key_F3:
            visibility_mask_ = 1 << 2;
            break;
        case Base_renderer::Renderer_io::Key_F4:
            visibility_mask_ = 1 << 3;
            break;
        case Base_renderer::Renderer_io::Key_F5:
            visibility_mask_ = 1 << 4;
            break;
        case Base_renderer::Renderer_io::Key_F6:
            visibility_mask_ = 1 << 5;
            break;
        case Base_renderer::Renderer_io::Key_F7:
            visibility_mask_ = 1 << 6;
            break;
        case Base_renderer::Renderer_io::Key_F8:
            visibility_mask_ = 1 << 7;
            break;
        }
    }
}

//******************************************************************************
// project_to_3D
//******************************************************************************

void Scene_renderer::project_to_3D(
    Scene_wireframe_vertex& point,
    const boost::numeric::ublas::matrix<float>& rot_mat)
{
    Scene_wireframe_vertex tmp_vert = prod(point, rot_mat);
    tmp_vert = tmp_vert - state_->camera_4D;
    tmp_vert = prod(tmp_vert, state_->projection_4D);

    //if(tmp_vert(3) < 0)
    //    gui_.distanceWarning->show();
    assert(tmp_vert(3) > 0);

    tmp_vert(0) /= tmp_vert(4);
    tmp_vert(1) /= tmp_vert(4);
    tmp_vert(2) /= tmp_vert(4);
    // Important!
    // Original coordinates in tmp_vert(3) and tmp_vert(4) are kept!
    // It is required for the 4D perspective.

    point = tmp_vert;
}

//******************************************************************************
// project_to_3D
//******************************************************************************

void Scene_renderer::project_to_3D(
    std::vector<Scene_wireframe_vertex>& verts,
    const boost::numeric::ublas::matrix<float>& rot_mat)
{
    auto project = [&](Scene_wireframe_vertex& v)
    {
        project_to_3D(v, rot_mat);
    };
    std::for_each(verts.begin(), verts.end(), project);
}

namespace
{
//******************************************************************************
// ColorToGlm
//******************************************************************************

glm::vec4 ColorToGlm(const Color& c, const float alpha)
{
    return glm::vec4(c.r_norm(), c.g_norm(), c.b_norm(), alpha);
}
} // namespace

//******************************************************************************
// draw_tesseract
//******************************************************************************

void Scene_renderer::draw_tesseract(Scene_wireframe_object& t)
{
    Mesh t_mesh;
    for(auto const& e : t.edges())
    {
        auto& current = t.get_vertices()[e.vert1];
        auto& next = t.get_vertices()[e.vert2];
        const glm::vec4 col = ColorToGlm(e.color, 1.f);

        Mesh_generator::cylinder(
            5,
            tesseract_thickness_ / current(3),
            tesseract_thickness_ / next(3),
            glm::vec3(current(0), current(1), current(2)),
            glm::vec3(next(0), next(1), next(2)),
            col,
            t_mesh);

    }

    for(unsigned int i = 0; i < t.get_vertices().size(); ++i)
    {
        float size_coef = 1.f;

        auto const& v = t.get_vertices()[i];
        glm::vec3 pos(v(0), v(1), v(2));

        if(i == 0)
            Mesh_generator::sphere(
                6,
                6,
                size_coef * sphere_diameter_ / v(3),
                pos,
                glm::vec4(1.f, 0.f, 0.f, 1.f),
                t_mesh);
        else
            Mesh_generator::sphere(
                6,
                6,
                size_coef * sphere_diameter_ / v(3),
                pos,
                glm::vec4(0.59f, 0.59f, 0.59f, 1.f),
                t_mesh);

    }

    diffuse_shader_->append_to_geometry(*back_geometry_.get(), t_mesh);
}

//******************************************************************************
// draw_curve
//******************************************************************************

void Scene_renderer::draw_curve(Curve& c, float opacity)
{
    const float marker_size = 8.f; //gui_.markerSize->value();

    auto log_speed = [](float speed) {
        return std::log2(3 * speed + 1) / 2;
    };

    auto get_speed_color =
        [&opacity, &log_speed, this](float normalized_speed) {
            float speed = log_speed(normalized_speed);
            return glm::vec4(
                (1 - speed) * state_->get_color(Curve_low_speed).r_norm() +
                    speed * state_->get_color(Curve_high_speed).r_norm(),
                (1 - speed) * state_->get_color(Curve_low_speed).g_norm() +
                    speed * state_->get_color(Curve_high_speed).g_norm(),
                (1 - speed) * state_->get_color(Curve_low_speed).b_norm() +
                    speed * state_->get_color(Curve_high_speed).b_norm(),
                opacity);
        };

    // Curve
    Mesh curve_mesh;
    std::vector<glm::vec3> point_directions(c.vertices().size());
    { // first
        const auto& a = c.get_vertices()[0];
        const auto& b = c.get_vertices()[1];
        point_directions.front() = glm::normalize(
            glm::vec3(b[0], b[1], b[2]) - glm::vec3(a[0], a[1], a[2]));
    }
    for(size_t i = 1; i < c.vertices().size() - 1; ++i)
    {
        const auto& p1 = c.vertices()[i - 1];
        const auto& p2 = c.vertices()[i];
        const auto& p3 = c.vertices()[i + 1];
        const glm::vec3 dir1 = glm::normalize(
            glm::vec3(p2[0], p2[1], p2[2]) - glm::vec3(p1[0], p1[1], p1[2]));
        const glm::vec3 dir2 = glm::normalize(
            glm::vec3(p3[0], p3[1], p3[2]) - glm::vec3(p2[0], p2[1], p2[2]));
        point_directions[i] = glm::normalize(dir1 + dir2);
    }
    { // last
        const auto& a = c.vertices()[c.vertices().size() - 2];
        const auto& b = c.vertices()[c.vertices().size() - 1];
        point_directions.back() = glm::normalize(
            glm::vec3(b[0], b[1], b[2]) - glm::vec3(a[0], a[1], a[2]));
    }

    for(size_t i = 0; i < c.edges().size(); ++i)
    {
        const auto& e = c.edges()[i];

        auto& current = c.get_vertices()[e.vert1];
        auto& next = c.get_vertices()[e.vert2];

        // We are interested only in some interval of the curve
        if(state_->curve_selection &&
           !state_->curve_selection->in_range(c.time_stamp()[e.vert1]))
        {
            continue;
        }

        auto& stats = c.get_stats();
        float speed_coeff = (stats.speed[i] - stats.min_speed) /
                            (stats.max_speed - stats.min_speed);

        Mesh_generator::cylinder_v2(
            5,
            curve_thickness_ / current(3),
            curve_thickness_ / next(3),
            glm::vec3(current(0), current(1), current(2)),
            glm::vec3(next(0), next(1), next(2)),
            point_directions[i],
            point_directions[i + 1],
            get_speed_color(speed_coeff),
            curve_mesh);
    }
    // TODO: fix the line below
    //gui_.Renderer->add_mesh(curve_mesh, opacity < 1.);
    if(opacity < 1.f)
        diffuse_shader_->append_to_geometry(*front_geometry_.get(), curve_mesh);
    else
        diffuse_shader_->append_to_geometry(*back_geometry_.get(), curve_mesh);


    if(state_->is_timeplayer_active)
    {
        auto marker =
            c.get_point(c.t_min() + state_->timeplayer_pos * c.t_duration());

        Mesh marker_mesh;
        Mesh_generator::sphere(
            5,
            5,
            marker_size / marker(3),
            glm::vec3(marker(0), marker(1), marker(2)),
            glm::vec4(1, 0, 0, 1),
            marker_mesh);

        diffuse_shader_->append_to_geometry(*back_geometry_.get(), marker_mesh);
    }
}

//******************************************************************************
// set_line_thickness
//******************************************************************************

void Scene_renderer::set_line_thickness(float t_thickness, float c_thickness)
{
    tesseract_thickness_ = t_thickness;
    curve_thickness_ = c_thickness;
}

//******************************************************************************
// set_sphere_diameter
//******************************************************************************

void Scene_renderer::set_sphere_diameter(float diameter)
{
    sphere_diameter_ = diameter;
}

//******************************************************************************
// set_fog
//******************************************************************************

void Scene_renderer::set_fog(float fog_dist, float fog_range)
{
    fog_range_.x = fog_dist - 0.5f * fog_range;
    fog_range_.y = fog_dist + 0.5f * fog_range;
}

//******************************************************************************
// split_animation
//******************************************************************************

std::vector<float> Scene_renderer::split_animation(float animation,
                                                   int    sections)
{
    float section_length = 1.f / sections;

    std::vector<float> splited_animations(sections);

    int current_section =
        animation == 1.f ? sections - 1 : (int)std::floor(animation * sections);

    for(int i = 0; i < sections; ++i)
    {
        if(i < current_section)
            splited_animations[i] = 1.f;
        else if(i == current_section)
            splited_animations[i] = animation * sections - current_section;
        else
            splited_animations[i] = 0.f;
    }

    return splited_animations;
}

//******************************************************************************
// draw_annotations
//******************************************************************************

void Scene_renderer::draw_annotations(Curve& c, const glm::mat4& projection)
{
    // Parameters
    const float min_arrow_dist(0.1f),
                arrow_spacing(6.f),
                arrow_size(8.f),
                sphere_diam(4.f);

    const glm::vec4 arrow_color(0.f, 0.f, 0.f, 1.f),
                    sphere_color(0.f, 0.f, 0.f, 1.f);

    auto annot_arrows = c.get_arrows(*state_->curve_selection.get());
    auto annot_dots = c.get_markers(*state_->curve_selection.get());

    // This variable points either to the filtered or original arrows
    std::vector<Curve_annotations>* annot_ptr;

    // If necessary, filter the annotations
    std::vector<Curve_annotations> filtered_annotations;
    if(filter_arrow_annotations_)
    {
        annot_ptr = &filtered_annotations;

        for(size_t i = 0; i < annot_arrows.size(); ++i)
        {
            auto& current = annot_arrows[i];
            double dist = std::numeric_limits<double>::max();

            for(size_t j = i + 1; j < annot_arrows.size(); ++j)
            {
                auto& next = annot_arrows[j];

                auto v = next.point - current.point;
                double length =
                    std::sqrt(v(0) * v(0) + v(1) * v(1) + v(2) * v(2));

                dist = std::min(length, dist);
            }

            if(dist > min_arrow_dist)
            {
                filtered_annotations.push_back(current);
            }
        }
    }
    else
    {
        annot_ptr = &annot_arrows;
    }

    // Draw annotation points

    for(auto& a : *annot_ptr)
    {
        // Copy and project point

        glm::vec4 point(a.point(0), a.point(1), a.point(2), 1.f);
        point = projection * point;
        for(char i = 0; i < 3; ++i)
            point[i] /= point[3];

        glm::vec4 dir(a.dir(0), a.dir(1), a.dir(2), 1.f);
        dir = projection * dir;
        for(char i = 0; i < 3; ++i)
            dir[i] /= dir[3];

        glm::vec4 scale(region_.width() / 2,  region_.height() / 2, 0.f, 0.f);
        glm::vec4 disp( region_.width() / 2,  region_.height() / 2, 0.f, 0.f);
        point = point * scale + disp;
        dir   = dir   * scale + disp;

        dir = dir - point;
        dir = glm::normalize(dir);

        auto draw_arrow = [&](glm::vec4 pos, glm::vec4 dir, float size)
        {
            glm::vec4 left_side =
                glm::rotateZ(dir, glm::radians(30.f)) * size;
            glm::vec4 right_side =
                glm::rotateZ(dir, glm::radians(-30.f)) * size;     

            Screen_shader::Line_strip line;
            line.emplace_back(Screen_shader::Line_point(glm::vec2(pos.x -  left_side.x, pos.y -  left_side.y), 1.f, arrow_color));
            line.emplace_back(Screen_shader::Line_point(glm::vec2(pos.x,                pos.y               ), 1.f, arrow_color));
            line.emplace_back(Screen_shader::Line_point(glm::vec2(pos.x - right_side.x, pos.y - right_side.y), 1.f, arrow_color));
            screen_shader_->append_to_geometry(*screen_geometry_.get(), line);
        };

        switch(a.dimensionality)
        {
        case 1:
            draw_arrow(point, dir, arrow_size);
            break;
        case 2:
            draw_arrow(point + dir * 0.5f * arrow_spacing, dir, arrow_size);
            draw_arrow(point - dir * 0.5f * arrow_spacing, dir, arrow_size);
            break;
        case 3:
            draw_arrow(point, dir, arrow_size);
            draw_arrow(point - dir * arrow_spacing, dir, arrow_size);
            draw_arrow(point + dir * arrow_spacing, dir, arrow_size);
            break;
        case 4:
            draw_arrow(point - dir * 1.5f * arrow_spacing, dir, arrow_size);
            draw_arrow(point - dir * 0.5f * arrow_spacing, dir, arrow_size);
            draw_arrow(point + dir * 0.5f * arrow_spacing, dir, arrow_size);
            draw_arrow(point + dir * 1.5f * arrow_spacing, dir, arrow_size);
            break;
        }
    }

    // Draw switch points

    for(auto& a : annot_dots)
    {
        Mesh mesh;
        Mesh_generator::sphere(
            5,
            5,
            sphere_diam / a(3),
            glm::vec3(a(0), a(1), a(2)),
            sphere_color,
            mesh);
        diffuse_shader_->append_to_geometry(*back_geometry_.get(), mesh);
    }
}

//******************************************************************************
// draw_legend
//******************************************************************************

void Scene_renderer::draw_legend(const Region& region)
{
    // Parameters

    const auto rect_size(20.f),
               spacing(7.5f);
    const glm::vec4 col(1.f, 0.f, 0.f, 1.f);

    auto draw_axis_legened = [&](
        char pos,
        const std::string& text,
        glm::vec4 color)
    {
        // Make background

        const auto x = region.width()  - (pos + 1) * (rect_size + spacing);
        const auto y = region.height() - rect_size - spacing;

        screen_shader_->append_to_geometry(
        *screen_geometry_.get(),
        Screen_shader::Rectangle(
            x,
            y,
            x + rect_size,
            y + rect_size,
            color));

        // Output text


        const auto symbol_width(6.f);
        const auto symbol_half_height(8.5f);
        const auto x_displacement = text.size() * 0.5f * symbol_width;

        text_renderer_->add_text(
            region_,
            glm::vec2(
                x + 0.5f * rect_size - x_displacement,
                y + 0.5f * rect_size + symbol_half_height),
            text);
    };

    auto color_to_vec4 = [](const Color& c)
    {
        return glm::vec4(c.r_norm(), c.g_norm(), c.b_norm(), c.a_norm());
    };

    draw_axis_legened(
        3,
        "x",
        color_to_vec4(state_->get_color(X_axis)));

    draw_axis_legened(
        2,
        "y",
        color_to_vec4(state_->get_color(Y_axis)));

    draw_axis_legened(
        1,
        "z",
        color_to_vec4(state_->get_color(Z_axis)));

    draw_axis_legened(
        0,
        "w",
        color_to_vec4(state_->get_color(W_axis)));
}

//******************************************************************************
// move_curves_to_3D_plots
//******************************************************************************

void Scene_renderer::move_curves_to_3D_plots(float coeff,
                                             std::vector<Curve>& curves)
{
    // Curve 1
    for(auto& v : curves[0].get_vertices())
        v(3) = v(3) + coeff * (state_->tesseract_size[3] / 2 - v(3));
    // Curve 2
    for(auto& v : curves[1].get_vertices())
        v(3) = v(3) + coeff * (-state_->tesseract_size[3] / 2 - v(3));
    // Curve 3
    for(auto& v : curves[2].get_vertices())
        v(2) = v(2) + coeff * (state_->tesseract_size[2] / 2 - v(2));
    // Curve 4
    for(auto& v : curves[3].get_vertices())
        v(2) = v(2) + coeff * (-state_->tesseract_size[2] / 2 - v(2));
    // Curve 5
    for(auto& v : curves[4].get_vertices())
        v(1) = v(1) + coeff * (-state_->tesseract_size[1] / 2 - v(1));
    // Curve 6
    for(auto& v : curves[5].get_vertices())
        v(1) = v(1) + coeff * (state_->tesseract_size[1] / 2 - v(1));
    // Curve 7
    for(auto& v : curves[6].get_vertices())
        v(0) = v(0) + coeff * (-state_->tesseract_size[0] / 2 - v(0));
    // Curve 8
    for(auto& v : curves[7].get_vertices())
        v(0) = v(0) + coeff * (state_->tesseract_size[0] / 2 - v(0));
}

//******************************************************************************
// move_curves_to_2D_plots
//******************************************************************************

void Scene_renderer::move_curves_to_2D_plots(
    float coeff,
    std::vector<Curve>& curves)
{
    // Curve 1
    for(auto& v : curves[0].get_vertices())
        v(2) = v(2) + coeff * (-state_->tesseract_size[2] / 2 - v(2));
    // Curve 2
    for(auto& v : curves[1].get_vertices())
        v(2) = v(2) + coeff * (-state_->tesseract_size[2] / 2 - v(2));
    // Curve 3
    for(auto& v : curves[2].get_vertices())
        v(2) = v(2) + coeff * (-state_->tesseract_size[2] / 2 - v(2));
    // Curve 4
    for(auto& v : curves[3].get_vertices())
        v(1) = v(1) + coeff * (-state_->tesseract_size[1] / 2 - v(1));
    // Curve 5
    for(auto& v : curves[4].get_vertices())
        v(1) = v(1) + coeff * (-state_->tesseract_size[1] / 2 - v(1));
    // Curve 6
    for(auto& v : curves[5].get_vertices())
        v(0) = v(0) + coeff *
        (0.5f * state_->tesseract_size[0] + state_->tesseract_size[3] - v(0));
}

//******************************************************************************
// tesseract_unfolding
//******************************************************************************

void Scene_renderer::tesseract_unfolding(
    float coeff,
    std::vector<Cube>& plots_3D,
    std::vector<Curve>& curves_3D)
{
    auto transform_3D_plot =
        [](Scene_wireframe_object& c,
           boost::numeric::ublas::matrix<float>& rot,
           Scene_wireframe_vertex disp)
    {
        for(auto& v : c.get_vertices())
        {
            for(int i = 0; i < 5; ++i)
                v(i) += disp(i);

            v = prod(v, rot);

            for(int i = 0; i < 5; ++i)
                v(i) -= disp(i);
        }
    };

    // Cube and curve 1 and 5
    {
        auto rot = Matrix_lib_f::getYWRotationMatrix(
            static_cast<float>(-coeff * PI / 2));
        Scene_wireframe_vertex disp1(5);
        disp1 <<= 0,
                  state_->tesseract_size[1] / 2,
                  0,
                  state_->tesseract_size[3] / 2,
                  0;

        transform_3D_plot(plots_3D[0], rot, disp1);
        transform_3D_plot(plots_3D[4], rot, disp1);

        const auto vert = plots_3D[4].get_vertices()[0];
        Scene_wireframe_vertex disp2(5);
        disp2 <<= 0, -vert(1), 0, -vert(3), 0;

        transform_3D_plot(plots_3D[0], rot, disp2);

        transform_3D_plot(curves_3D[4], rot, disp1);

        transform_3D_plot(curves_3D[0], rot, disp1);
        transform_3D_plot(curves_3D[0], rot, disp2);
    }
    // Cube and curve 3
    {
        auto rot = Matrix_lib_f::getZWRotationMatrix(
            static_cast<float>(coeff * PI / 2));
        Scene_wireframe_vertex disp(5);
        disp <<= 0,
                 0,
                 -state_->tesseract_size[2] / 2,
                 state_->tesseract_size[3] / 2,
                 0;

        transform_3D_plot(plots_3D[2], rot, disp);

        transform_3D_plot(curves_3D[2], rot, disp);
    }
    // Cube and curve 4
    {
        auto rot = Matrix_lib_f::getZWRotationMatrix(
            static_cast<float>(-coeff * PI / 2));
        Scene_wireframe_vertex disp(5);
        disp <<= 0,
                 0,
                 state_->tesseract_size[2] / 2,
                 state_->tesseract_size[3] / 2,
                 0;

        transform_3D_plot(plots_3D[3], rot, disp);

        transform_3D_plot(curves_3D[3], rot, disp);
    }
    // Cube and curve 6
    {
        auto rot = Matrix_lib_f::getYWRotationMatrix(
            static_cast<float>(coeff * PI / 2));
        Scene_wireframe_vertex disp(5);
        disp <<= 0,
                 -state_->tesseract_size[1] / 2,
                 0,
                 state_->tesseract_size[3] / 2,
                 0;

        transform_3D_plot(plots_3D[5], rot, disp);

        transform_3D_plot(curves_3D[5], rot, disp);
    }
    // Cube and curve 7
    {
        auto rot = Matrix_lib_f::getXWRotationMatrix(
            static_cast<float>(coeff * PI / 2));
        Scene_wireframe_vertex disp(5);
        disp <<= state_->tesseract_size[0] / 2,
                 0,
                 0,
                 state_->tesseract_size[3] / 2,
                 0;

        transform_3D_plot(plots_3D[6], rot, disp);

        transform_3D_plot(curves_3D[6], rot, disp);
    }
    // Cube and curve 8
    {
        auto rot = Matrix_lib_f::getXWRotationMatrix(
            static_cast<float>(-coeff * PI / 2));
        Scene_wireframe_vertex disp(5);
        disp <<= -state_->tesseract_size[0] / 2,
                  0,
                  0,
                  state_->tesseract_size[3] / 2,
                  0;

        transform_3D_plot(plots_3D[7], rot, disp);

        transform_3D_plot(curves_3D[7], rot, disp);
    }
}

//******************************************************************************
// get_rotation_matrix
//******************************************************************************

boost::numeric::ublas::matrix<float> Scene_renderer::get_rotation_matrix()
{
    auto m = Matrix_lib_f::getXYRotationMatrix(state_->xy_rot);
    m = prod(m, Matrix_lib_f::getYZRotationMatrix(state_->yz_rot));
    m = prod(m, Matrix_lib_f::getZXRotationMatrix(state_->zx_rot));
    m = prod(m, Matrix_lib_f::getXWRotationMatrix(state_->xw_rot));
    m = prod(m, Matrix_lib_f::getYWRotationMatrix(state_->yw_rot));
    m = prod(m, Matrix_lib_f::getZWRotationMatrix(state_->zw_rot));

    return m;
}

//******************************************************************************
// get_rotation_matrix
//******************************************************************************

boost::numeric::ublas::matrix<float>
Scene_renderer::get_rotation_matrix(float view_straightening)
{
    auto angle_xy = (1 - view_straightening) * state_->xy_rot,
         angle_yz = (1 - view_straightening) * state_->yz_rot,
         angle_zx = (1 - view_straightening) * state_->zx_rot,
         angle_xw = (1 - view_straightening) * state_->xw_rot,
         angle_yw = (1 - view_straightening) * state_->yw_rot,
         angle_zw = (1 - view_straightening) * state_->zw_rot;

    auto m = Matrix_lib_f::getXYRotationMatrix(angle_xy);
    m = prod(m, Matrix_lib_f::getYZRotationMatrix(angle_yz));
    m = prod(m, Matrix_lib_f::getZXRotationMatrix(angle_zx));
    m = prod(m, Matrix_lib_f::getXWRotationMatrix(angle_xw));
    m = prod(m, Matrix_lib_f::getYWRotationMatrix(angle_yw));
    m = prod(m, Matrix_lib_f::getZWRotationMatrix(angle_zw));

    return m;
}

//******************************************************************************
// draw_3D_plot
//******************************************************************************

void Scene_renderer::draw_3D_plot(Cube& cube, float opacity)
{
    for(size_t i = 0; i < cube.edges().size(); ++i)
    {
        auto const& e = cube.edges()[i];

        Mesh t_mesh;

        auto& current = cube.get_vertices()[e.vert1];
        auto& next = cube.get_vertices()[e.vert2];
        const glm::vec4 col = ColorToGlm(e.color, opacity);

        Mesh_generator::cylinder(
            5,
            tesseract_thickness_ / current(3),
            tesseract_thickness_ / next(3),
            glm::vec3(current(0), current(1), current(2)),
            glm::vec3(next(0), next(1), next(2)),
            col,
            t_mesh);

        opacity < 1.0 ? diffuse_shader_->append_to_geometry(
                            *front_geometry_.get(), t_mesh)
                      : diffuse_shader_->append_to_geometry(
                            *back_geometry_.get(), t_mesh);
    }
}

//******************************************************************************
// draw_2D_plot
//******************************************************************************

void Scene_renderer::draw_2D_plot(Scene_wireframe_object& plot)
{
    for(auto const& e : plot.edges())
    {
        Mesh t_mesh;

        auto& current = plot.get_vertices()[e.vert1];
        auto& next = plot.get_vertices()[e.vert2];
        const glm::vec4 col = ColorToGlm(e.color, 1.f);

        Mesh_generator::cylinder(
            5,
            tesseract_thickness_ / current(3),
            tesseract_thickness_ / next(3),
            glm::vec3(current(0), current(1), current(2)),
            glm::vec3(next(0), next(1), next(2)),
            col,
            t_mesh);

        diffuse_shader_->append_to_geometry(*back_geometry_.get(), t_mesh);
    }
}

//******************************************************************************
// plots_unfolding
//******************************************************************************

void Scene_renderer::plots_unfolding(
    float coeff,
    std::vector<Square>& plots_2D,
    std::vector<Curve>& curves_2D)
{
    auto transform_3D_plot =
        [](Scene_wireframe_object& c,
           boost::numeric::ublas::matrix<float>& rot,
           Scene_wireframe_vertex& disp)
    {
        for(auto& v : c.get_vertices())
        {
            // We have to create a copy vector of the size of four in order
            // to multiype to the 4x4 rotation matrix
            Scene_wireframe_vertex copy_v(4);
            copy_v <<= v(0), v(1), v(2), v(3);

            for(int i = 0; i < 4; ++i)
                copy_v(i) += disp(i);

            copy_v = prod(copy_v, rot);

            for(int i = 0; i < 4; ++i)
                copy_v(i) -= disp(i);

            v <<= copy_v(0), copy_v(1), copy_v(2), copy_v(3), 0;
        }
    };

    {
        auto anchor = plots_2D[1].get_vertices()[0];
        auto rot_axis =
            plots_2D[1].get_vertices()[1] - plots_2D[1].get_vertices()[0];
        auto rot = Matrix_lib_f::getRotationMatrix(
            static_cast<float>(coeff * PI / 2),
            rot_axis(0),
            rot_axis(1),
            rot_axis(2));
        Scene_wireframe_vertex disp(5);
        disp <<= -anchor(0), -anchor(1), -anchor(2), 0, 0;

        transform_3D_plot(plots_2D[3], rot, disp);
        transform_3D_plot(plots_2D[4], rot, disp);
        transform_3D_plot(plots_2D[5], rot, disp);

        transform_3D_plot(curves_2D[3], rot, disp);
        transform_3D_plot(curves_2D[4], rot, disp);
        transform_3D_plot(curves_2D[5], rot, disp);
    }
    {
        auto anchor = plots_2D[2].get_vertices()[1];
        auto rot_axis =
            plots_2D[4].get_vertices()[2] - plots_2D[4].get_vertices()[1];
        auto rot = Matrix_lib_f::getRotationMatrix(
            static_cast<float>(coeff * PI / 2),
            rot_axis(0),
            rot_axis(1),
            rot_axis(2));
        Scene_wireframe_vertex disp(5);
        disp <<= -anchor(0), -anchor(1), -anchor(2), 0, 0;

        transform_3D_plot(plots_2D[5], rot, disp);

        transform_3D_plot(curves_2D[5], rot, disp);
    }
}

//******************************************************************************
// plots_unfolding
//******************************************************************************

void Scene_renderer::draw_labels_in_2D(const glm::mat4& projection)
{
    // Draw labels in 2D
    if(label_points_.size() != 3)
        return;

    glm::vec4 upper_left(
        label_points_[0](0), label_points_[0](1), label_points_[0](2), 1.f);
    glm::vec4 bottom_left(
        label_points_[1](0), label_points_[1](1), label_points_[1](2), 1.f);
    glm::vec4 bottom_right(
        label_points_[2](0), label_points_[2](1), label_points_[2](2), 1.f);

    upper_left = projection * upper_left;
    bottom_left = projection * bottom_left;
    bottom_right = projection * bottom_right;

    for(char i = 0; i < 3; ++i)
    {
        upper_left[i]   /= upper_left[3];
        bottom_left[i]  /= bottom_left[3];
        bottom_right[i] /= bottom_right[3];
    }

    glm::vec4 scale(region_.width() / 2, region_.height() / 2, 0.f, 0.f);
    glm::vec4 disp(region_.width() / 2, region_.height() / 2, 0.f, 0.f);
    upper_left = upper_left * scale + disp;
    bottom_left = bottom_left * scale + disp;
    bottom_right = bottom_right * scale + disp;

    std::vector<glm::vec4> points;
    auto horiz_dir = bottom_right - bottom_left;

    auto lenght = glm::length(horiz_dir);

    glm::vec4 horiz_disp(-3.f, -0.025f * lenght, 0.f, 0.f);
    for(int i = 0; i < 3; ++i)
        points.push_back(
            bottom_left + horiz_dir * static_cast<float>((2.f * i + 1.f) / 6.f) + horiz_disp);

    auto vert_dir = upper_left - bottom_left;
    glm::vec4 vert_disp(-0.08f * lenght - 3.f, 6.f, 0.f, 0.f);
    for(int i = 0; i < 3; ++i)
        points.push_back(
            bottom_left + vert_dir * static_cast<float>((2.f * i + 1.f) / 6.f) + vert_disp);


    text_renderer_->add_text(
        region_,
        glm::vec2(
            points[0].x,
            points[0].y),
        std::string("X"));

    text_renderer_->add_text(
        region_,
        glm::vec2(
            points[1].x,
            points[1].y),
        std::string("W"));

    text_renderer_->add_text(
        region_,
        glm::vec2(
            points[2].x,
            points[2].y),
        std::string("Y"));

    text_renderer_->add_text(
        region_,
        glm::vec2(
            points[3].x,
            points[3].y),
        std::string("-Z"));

    text_renderer_->add_text(
        region_,
        glm::vec2(
            points[4].x,
            points[4].y),
        std::string("Y"));

    text_renderer_->add_text(
        region_,
        glm::vec2(
            points[5].x,
            points[5].y),
        std::string("W"));
}
