#pragma once
// Local
#include "Curve_annotations.h"
#include "Curve_selection.h"
#include "Curve_stats.h"
#include "Wireframe_object.h"
// boost
#include "boost/tuple/tuple.hpp"
#include <boost/numeric/ublas/vector.hpp>
// std
#include <vector>

class Curve : public Wireframe_object
{
public:
    typedef boost::numeric::ublas::vector<double> Point_type;
    typedef boost::tuple<float, int> Arrow_type;

    void add_point(Point_type vertex, float time);
    Point_type get_point(float time);
    const std::vector<float>& get_time_stamp() const;
    void shift_to_origin(double max_edge_length, Point_type& out_shift);
    void get_boundaries(Point_type& origin, Point_type& size) const;
    Curve get_simpified_curve(const double spacing);

    void update_stats();
    const Curve_stats& get_stats();

    std::vector<Curve_annotations> get_arrows(const Curve_selection& selection);
    std::vector<Point_type> get_markers(const Curve_selection& selection);

private:
    void calculate_general_stats();
    void calculate_annotations();

    std::vector<float> time_stamp_;
    Curve_stats stats_;

    // Annotations
    std::vector<Arrow_type> arrows_;
    std::vector<size_t> markers_;
};
