#pragma once
// Local
#include "Base_renderer.h"
#include "Geometry_engine.h"
#include "Scene_state.h"
#include "Screen_shader.h"
// std
#include <memory.h>
// glm
#include <glm/glm.hpp>

class Timeline_renderer : public Base_renderer
{
private:
    struct Mouse_selection
    {
        Mouse_selection()
        {
            start_pnt = end_pnt = glm::vec2(0.f, 0.f);
            is_active = false;
        }

        glm::vec2 start_pnt;
        glm::vec2 end_pnt;
        bool is_active;
    };

    Timeline_renderer() = delete;

public:
    Timeline_renderer(std::shared_ptr<Scene_state> state);

    void set_shader(std::shared_ptr<Screen_shader> screen);
    void render() override;
    void process_input(const Renderer_io& io) override;

    virtual void set_redering_region(Region region,
                                     float scale_x,
                                     float scale_y) override;
    void set_pictogram_size(float size);

private:
    // Drawing functions
    void draw_axes(     const Region& region);
    void draw_curve(    const Region& region);
    void draw_switches( const Region& region);
    void draw_marker(   const Region& region);
    void draw_selection(const Region& region, const Mouse_selection& s);
    void draw_pictogram(
        const glm::vec2& center,
        float size,
        const Curve_selection& seleciton,
        std::string dim,
        Curve_stats::Range range);

    void calculate_switch_points(
        std::vector<float>& out_points,
        const Region& region);

    void project_point(
        boost::numeric::ublas::vector<double>& point,
        double size,
        double tesseract_size);
    void project_point_array(
        std::vector<boost::numeric::ublas::vector<double>>& points,
        double size,
        double tesseract_size);

    void update_regions();

    std::shared_ptr<Screen_shader> screen_shader_;
    std::unique_ptr<Screen_shader::Screen_geometry> screen_geom_;

    int pictogram_num_;
    float pictogram_size_, pictogram_spacing_;

    Mouse_selection mouse_selection_;

    float player_pos_;

    bool track_mouse_;

    Region plot_region_, pictogram_region_;
};
