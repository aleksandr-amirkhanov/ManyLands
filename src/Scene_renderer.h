#pragma once
// local
#include "Base_renderer.h"
#include "Scene_state.h"
#include "Mesh.h"
#include "Line_2D.h"
#include "Diffuse_shader.h"
#include "Screen_shader.h"
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
    void render() override;

    void set_line_thickness(float t_thickness, float c_thickness);
    void set_sphere_diameter(float diameter);

private:
    void project_to_3D(
        boost::numeric::ublas::vector<double>& point,
        const boost::numeric::ublas::matrix<double>& rot_mat);
    void project_to_3D(
        std::vector<boost::numeric::ublas::vector<double>>& verts,
        const boost::numeric::ublas::matrix<double>& rot_mat);

    void draw_tesseract(Wireframe_object& t);
    void draw_curve(Curve& c, float opacity);
    void draw_annotations(Curve& c);
    void move_curves_to_3D_plots(double coeff, std::vector<Curve>& curves);
    void move_curves_to_2D_plots(double coeff, std::vector<Curve>& curves);
    void tesseract_unfolding(
        double coeff,
        std::vector<Cube>& plots_3D,
        std::vector<Curve>& curves_3D);
    boost::numeric::ublas::matrix<double> get_rotation_matrix();
    boost::numeric::ublas::matrix<double>
    get_rotation_matrix(double view_straightening);
    void draw_3D_plot(Cube& cube, double opacity);
    void draw_2D_plot(Wireframe_object& plot);
    void plots_unfolding(
        double coeff,
        std::vector<Square>& plots_2D,
        std::vector<Curve>& curves_2D);    

    std::vector<double> split_animation(double animation_pos, int sections);

    // Drawing parameters
    float tesseract_thickness_,
          curve_thickness_,
          sphere_diameter_;

    std::shared_ptr<Diffuse_shader> diffuse_shader;
    std::shared_ptr<Screen_shader> screen_shader;

    std::vector<std::unique_ptr<Diffuse_shader::Mesh_geometry>>
        back_geometry_, front_geometry_;

    bool optimize_performance_;
    int visibility_mask_;
    const int number_of_animations_;
};
