#pragma once
// local
#include "Scene_state.h"
#include "Geometry_engine.h"
#include "Mesh.h"
#include "Line_2D.h"
// std
#include <memory.h>
// boost
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

class Scene_renderer
{
private:
    struct Mesh_shader_ids
    {
        GLuint program_id,
               proj_mat_id,
               mv_mat_id,
               normal_mat_id,
               light_pos_id,
               vertex_attrib_id,
               normal_attrib_id,
               color_attrib_id;
    };

    struct Screen_shader_ids
    {
        GLuint program_id,
               proj_mat_id,
               vertex_attrib_id,
               color_attrib_id;
    };

    struct Mesh_array
    {
        glm::vec4 vert;
        glm::vec3 norm;
        glm::vec4 color;
    };

    struct Line_array
    {
        glm::vec2 vert;
        glm::vec4 color;
    };

    typedef Geometry_engine<Mesh_array> Mesh_geometry;
    typedef Geometry_engine<Line_array> Line_geometry;

public:
    Scene_renderer();
    Scene_renderer(std::shared_ptr<Scene_state> state);

    void load_shaders();
    void set_state(std::shared_ptr<Scene_state> state);
    void update_redering_region(glm::ivec2 pos, glm::ivec2 scene_size);
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

    std::unique_ptr<Mesh_geometry> create_mesh_geometry(const Mesh& m);
    void draw_mesh_geometry(const std::unique_ptr<Mesh_geometry>& geom);

    std::unique_ptr<Line_geometry> create_line_geometry(const Line_2D& line);
    void draw_line_geometry(const std::unique_ptr<Line_geometry>& geom);

    std::vector<double> split_animation(double animation_pos, int sections);

    // Drawing parameters
    float tesseract_thickness_,
          curve_thickness_,
          sphere_diameter_;

    Mesh_shader_ids mesh_shader_ids;
    Screen_shader_ids screen_shader_ids;

    std::shared_ptr<Scene_state> state_;

    std::vector<std::unique_ptr<Mesh_geometry>> back_geometry_, front_geometry_;

    glm::ivec2 scene_pos, scene_size;

    bool optimize_performance_;
    int visibility_mask_;
    const int number_of_animations_;
};
