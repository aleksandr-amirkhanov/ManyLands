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

    struct Compas_state
    {
        Compas_state(float x_pos, float scale)
        {
            this->x_pos = x_pos;
            this->scale = scale;
        }

        float x_pos, scale;
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
    void set_splitter(float splitter);

    void set_pictogram_size(float size);
    void set_pictogram_magnification(float scale, int region_size);

private:
    // Drawing functions
    void draw_axes(      const Region& region);
    void draw_curve(     const Region& region);
    void draw_switches(  const Region& region);
    void draw_marker(    const Region& region);
    void draw_selection( const Region& region, const Mouse_selection& s);
    void draw_pictograms(const Region& region,
                         const std::vector<Compas_state>& compases_state);
    void draw_pictogram(
        const glm::vec2& center,
        float size,
        const Curve_selection& seleciton,
        std::string dim,
        Curve_stats::Range range);

    std::vector<Compas_state> get_compases_state(const Region& region);

    void make_selection(const Mouse_selection& s);
    void calculate_switch_points(
        std::vector<float>& out_points,
        const Region& region);

    void project_point(
        Scene_wireframe_vertex& point,
        float size,
        float tesseract_size);
    void project_point_array(
        std::vector<Scene_wireframe_vertex>& points,
        float size,
        float tesseract_size);

    void update_regions();

    float magnification_func(float x);
    float magnification_func_area(float x_start, float x_end);

    std::shared_ptr<Screen_shader> screen_shader_;
    std::unique_ptr<Screen_shader::Screen_geometry> screen_geom_;

    float pictogram_size_, pictogram_spacing_, pictogram_scale_;
    size_t pictogram_magnification_region_;

    glm::vec2 mouse_pos_;

    Mouse_selection mouse_selection_;

    float player_pos_;

    bool track_mouse_;

    Region plot_region_, pictogram_region_;
    
    float splitter_;
};
