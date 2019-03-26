#pragma once
// Local
#include "Curve_annotations.h"
#include "Curve_selection.h"
#include "Curve_stats.h"
#include "Color.h"
#include "Scene_wireframe_object.h"
// boost
#include "boost/tuple/tuple.hpp"
#include <boost/numeric/ublas/vector.hpp>
// std
#include <vector>

class Curve : public Scene_wireframe_object
{
public:
    typedef boost::tuple<float, int> Arrow_type;

    void add_point(Scene_wireframe_vertex vertex, float time);
    Scene_wireframe_vertex get_point(float time);

    // Timestamp-related functions
    const std::vector<float>& time_stamp() const;
    float t_min() const;
    float t_max() const;
    float t_duration() const;

    void shift_to_origin(Scene_wireframe_vertex& out_shift);
    void get_boundaries(Scene_wireframe_vertex& origin,
                        Scene_wireframe_vertex& size) const;
    Curve get_simpified_curve(const float min_radius);
    Curve get_simpified_curve_RDP(const float max_deviation);

    void update_stats();
    const Curve_stats& get_stats();

    std::vector<Curve_annotations> get_arrows(const Curve_selection& selection);
    std::vector<Scene_wireframe_vertex> get_markers(const Curve_selection& selection);

    void translate_vertices(const Scene_wireframe_vertex& translate);
    void scale_vertices(float scale_factor);
    void scale_vertices(const Scene_wireframe_vertex& scale_factor);

private:
    void calculate_general_stats();
    void calculate_annotations();

    std::vector<float> time_stamp_;
    Curve_stats stats_;

    // Annotations
    std::vector<Arrow_type> arrows_;
    std::vector<size_t>     markers_;

    const static Color default_color_;
};
