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
    , optimize_performance_(true)
    , visibility_mask_(0)
    , track_mouse_(false)
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
// render
//******************************************************************************

void Scene_renderer::render()
{
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
    auto world_mat = glm::toMat4(glm::lerp(state_->rotation_3D, glm::quat(), static_cast<float>(unfold_3D)));
    auto norm_mat = glm::transpose(glm::inverse(glm::mat3(world_mat)));

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

    if(!optimize_performance_)
    {
        projected_c = *state_->curve.get();
        for(int i = 0; i < 8; ++i)
            curves_3D.push_back(*state_->curve.get());
    }
    else
    {
        projected_c = *state_->simple_curve.get();
        for(int i = 0; i < 8; ++i)
            curves_3D.push_back(*state_->simple_curve.get());
    }

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
            draw_annotations(projected_c);
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
                        draw_annotations(c);
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

            // TODO: uncomment the line below (rewrite it first ;) )!
            /*gui_.Renderer->set_label_positions(
                plots_2D[0].get_vertices()[3],
                plots_2D[3].get_vertices()[0],
                plots_2D[5].get_vertices()[1]);*/

            // Draw 2D curves
            if(state_->show_curve)
            {
                // TODO: uncomment lines below (rewrite it first ;) )!
                /*if(unfold_3D == 1.)
                    gui_.Renderer->show_labels(true);*/

                for(auto& c : curves_2D)
                {
                    draw_curve(c, 1.);
                    draw_annotations(c);
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
        state_->camera_3D.z += io.mouse_wheel_y * 0.2f;
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
    const float marker_size = 3.f; //gui_.markerSize->value();

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

        Mesh_generator::cylinder(
            5,
            curve_thickness_ / current(3),
            curve_thickness_ / next(3),
            glm::vec3(current(0), current(1), current(2)),
            glm::vec3(next(0), next(1), next(2)),
            get_speed_color(speed_coeff),
            curve_mesh);
    }
    // TODO: fix the line below
    //gui_.Renderer->add_mesh(curve_mesh, opacity < 1.);
    diffuse_shader_->append_to_geometry(*back_geometry_.get(), curve_mesh);


    /*boost::numeric::ublas::vector<double> marker = c.get_point(player_pos_);

    Mesh marker_mesh;
    Mesh_generator::sphere(
        16,
        16,
        marker_size / marker(3),
        glm::vec3(marker(0), marker(1), marker(2)),
        glm::vec4(1, 0, 0, 1),
        marker_mesh);

    //gui_.Renderer->add_mesh(marker_mesh);
    back_geometry_.push_back(std::make_unique<Geometry_engine>(marker_mesh));*/
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

void Scene_renderer::draw_annotations(Curve& c)
{
    // TODO: port the method

    /*auto annot_arrows = c.get_arrows(*curve_selection_.get());
    auto annot_dots = c.get_markers(*curve_selection_.get());

    // Draw annotation arrows
    for(auto& a : annot_arrows)
        gui_.Renderer->add_annotation(a);

    // Draw annotation dots
    for(auto& a : annot_dots)
        draw_point(a, QColor(0, 0, 0), 3.);*/
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
        v(0) = v(0) + coeff * (1.5f * state_->tesseract_size[0] - v(0));
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
    float angle_xw = (1 - view_straightening) * state_->xw_rot;
    float angle_yw = (1 - view_straightening) * state_->yw_rot;
    float angle_zw = (1 - view_straightening) * state_->zw_rot;

    auto m = Matrix_lib_f::getXYRotationMatrix(state_->xy_rot);
    m = prod(m, Matrix_lib_f::getYZRotationMatrix(state_->yz_rot));
    m = prod(m, Matrix_lib_f::getZXRotationMatrix(state_->zx_rot));
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
