#pragma once
// local
#include "Scene_state.h"
#include "Geometry_engine.h"
// std
#include <memory.h>
// boost
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

class Scene_renderer
{
public:
    Scene_renderer();
    Scene_renderer(std::shared_ptr<Scene_state> state);

    void set_state(std::shared_ptr<Scene_state> state);
    void render();

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

    // drawing parameters
    float tesseract_thickness_,
          curve_thickness_,
          sphere_diameter_;

    std::shared_ptr<Scene_state> state_;
    std::vector<std::unique_ptr<Geometry_engine>> back_geometry_;
    std::vector<std::unique_ptr<Geometry_engine>> front_geometry_;

    bool optimize_performance_;
    int visibility_mask_;
    const int number_of_animations_;
};
