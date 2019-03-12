#pragma once
// Local
#include "Base_renderer.h"
#include "Geometry_engine.h"
#include "Scene_state.h"
#include "Screen_shader.h"
// std
#include <memory.h>

class Timeline_renderer : public Base_renderer
{
private:
    struct Line_array
    {
        glm::vec2 vert;
        glm::vec4 color;
    };

    typedef Geometry_engine<Line_array> Line_geometry;

    Timeline_renderer() = delete;

public:
    Timeline_renderer(std::shared_ptr<Scene_state> state);

    void set_shader(std::shared_ptr<Screen_shader> screen);
    void render() override;

private:
    void draw_axes(const Rect& region);
    void draw_curve(const Rect& region);
    void draw_switches(const Rect& region);

    void calculate_switch_points(
        std::vector<float>& out_points,
        const Rect& region);

    void project_point(
        boost::numeric::ublas::vector<double>& point,
        double size,
        double tesseract_size);
    void project_point_array(
        std::vector<boost::numeric::ublas::vector<double>>& points,
        double size,
        double tesseract_size);

    std::shared_ptr<Screen_shader> screen_shader;

    int pictogram_num_;
    float pictogram_size_, pictogram_spacing_;
};
