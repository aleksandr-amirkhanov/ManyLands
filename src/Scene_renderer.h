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

    void set_line_thickness(float thickness);
    void set_sphere_diameter(float diameter);

private:
    void project_to_3D(
        boost::numeric::ublas::vector<double>& point);
    void project_to_3D(
        std::vector<boost::numeric::ublas::vector<double>>& verts);

    void draw_tesseract(Wireframe_object& t);
    void draw_curve(Curve& c, float opacity);

    // drawing parameters
    float line_thickness_;
    float sphere_diameter_;

    std::shared_ptr<Scene_state> state_;
    std::vector<std::unique_ptr<Geometry_engine>> geometry_;
};
