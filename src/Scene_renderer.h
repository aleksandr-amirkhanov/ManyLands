#pragma once
// local
#include "Base_renderer.h"
#include "Scene_state.h"
#include "Mesh.h"
#include "Diffuse_shader.h"
#include "Screen_shader.h"
#include "Scene_wireframe_object.h"
#include "Text_renderer.h"
// std
#include <memory.h>
// boost
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

class Scene_renderer : public Base_renderer
{
private:
    Scene_renderer() = delete;

public:
    Scene_renderer(std::shared_ptr<Scene_state> state);

    void set_shaders(std::shared_ptr<Diffuse_shader> diffuse,
                     std::shared_ptr<Screen_shader> screen);
    void set_text_renderer(std::shared_ptr<Text_renderer> tex_ren);
    void render() override;
    void process_input(const Renderer_io& io) override;

    void set_line_thickness(float t_thickness, float c_thickness);
    void set_sphere_diameter(float diameter);
    void set_fog(float fog_dist, float fog_range); 

private:
    void project_to_3D(
        Scene_vertex_t& point,
        const boost::numeric::ublas::matrix<float>& rot_mat);
    void project_to_3D(
        std::vector<Scene_vertex_t>& verts,
        const boost::numeric::ublas::matrix<float>& rot_mat);

    void draw_tesseract(Scene_wireframe_object& t);
    void draw_curve(Curve& c, float opacity, const Color& color);
    void draw_curve(
        Curve& c,
        float opacity,
        const Color& slow_c,
        const Color& fast_c);
    void draw_annotations(Curve& c, const glm::mat4& projection);
    void draw_legend(const Region& region);

    void move_curves_to_3D_plots(float coeff, std::vector<Curve>& curves);
    void move_curves_to_2D_plots(float coeff, std::vector<Curve>& curves);
    void tesseract_unfolding(
        float coeff,
        std::vector<Cube>& plots_3D,
        std::vector<std::vector<Curve>>& curves_3D);
    boost::numeric::ublas::matrix<float> get_rotation_matrix();
    boost::numeric::ublas::matrix<float>
    get_rotation_matrix(float view_straightening);
    void draw_3D_plot(Cube& cube, float opacity);
    void draw_2D_plot(Scene_wireframe_object& plot);
    void plots_unfolding(
        float coeff,
        std::vector<Square>& plots_2D,
        std::vector<std::vector<Curve>>& curves_2D);
    void draw_labels_in_2D(const glm::mat4& projection);

    std::vector<float> split_animation(float animation_pos, int sections);

    // Drawing parameters
    float tesseract_thickness_,
          curve_thickness_,
          sphere_diameter_;

    glm::vec2 fog_range_;

    std::shared_ptr<Diffuse_shader> diffuse_shader_;
    std::shared_ptr<Screen_shader> screen_shader_;

    std::shared_ptr<Text_renderer> text_renderer_;

    std::unique_ptr<Diffuse_shader::Mesh_geometry> back_geometry_,
                                                   front_geometry_;
    std::unique_ptr<Screen_shader::Screen_geometry> screen_geometry_;

    int visibility_mask_;
    const int number_of_animations_;

    bool track_mouse_;

    bool filter_arrow_annotations_;

    std::vector<boost::numeric::ublas::vector<double>> label_points_;
    bool show_labels_;
};
