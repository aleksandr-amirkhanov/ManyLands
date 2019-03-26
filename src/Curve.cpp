#include "Curve.h"

#include "Scene_wireframe_object.h"

// std
#include <algorithm>
#include <cmath>
#include <list>
// boost
#include <boost/numeric/ublas/assignment.hpp>
// VVV needed for boost::geometry::simplify VVV
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point.hpp>

const Color Curve::default_color_ = Color(0, 0, 0, 255);

//******************************************************************************
// add_point
//******************************************************************************

void Curve::add_point(Scene_wireframe_vertex vertex, float time)
{
    // Adding vertex
    get_vertices().push_back(vertex);

    // Adding edge
    if(get_vertices().size() > 1)
        add_edge(Scene_wireframe_edge(get_vertices().size() - 2,
                                      get_vertices().size() - 1,
                                      default_color_));

    // Adding time stamp
    time_stamp_.push_back(time);
}

//******************************************************************************
// get_point
//******************************************************************************

Scene_wireframe_vertex Curve::get_point(float time)
{
    // We assume that points are already sorted by the time stamp value
    if(time <= time_stamp_.front())
        return get_vertices().front();

    if(time >= time_stamp_.back())
        return get_vertices().back();

    size_t range[2];
    range[0] = 0;
    range[1] = time_stamp_.size() - 1;

    while(range[1] - range[0] > 1)
    {
        size_t middle = static_cast<size_t>(0.5f * (range[1] + range[0]));
        if(time_stamp_[middle] > time)
            range[1] = middle;
        else
            range[0] = middle;
    }

    float coeff = (time - time_stamp_[range[0]]) /
                  (time_stamp_[range[1]] - time_stamp_[range[0]]);

    return get_vertices()[range[0]] +
           coeff * (get_vertices()[range[1]] - get_vertices()[range[0]]);
}

//******************************************************************************
// time_stamp
//******************************************************************************

const std::vector<float>& Curve::time_stamp() const
{
    return time_stamp_;
}

//******************************************************************************
// t_min
//******************************************************************************

float Curve::t_min() const
{
    return time_stamp_.front();
}

//******************************************************************************
// t_max
//******************************************************************************

float Curve::t_max() const
{
    return time_stamp_.back();
}

//******************************************************************************
// t_duration
//******************************************************************************

float Curve::t_duration() const
{
    return (t_max() - t_min());
}

//******************************************************************************
// shift_to_origin
//******************************************************************************

void Curve::shift_to_origin(Scene_wireframe_vertex& out_shift)
{
    Scene_wireframe_vertex origin(5), size(5);
    get_boundaries(origin, size);

    translate_vertices(-0.5f * size - origin);
}

//******************************************************************************
// get_boundaries
//******************************************************************************

void Curve::get_boundaries(
    Scene_wireframe_vertex& origin,
    Scene_wireframe_vertex& size) const
{
    // Finding minimum and maximum values of the curve
    Scene_wireframe_vertex min(5);
    Scene_wireframe_vertex max(5);

    if(vertices().size() > 0)
    {
        const auto& first = vertices().at(0);
        min = first;
        max = first;
    }

    for(const auto& v : vertices())
    {
        for(unsigned char i = 0; i < 4; ++i)
        {
            if(v(i) < min(i))
                min(i) = v(i);
            if(v(i) > max(i))
                max(i) = v(i);
        }
    }

    origin = min;
    size = max - min;
}

//******************************************************************************
// get_simpified_curve
//******************************************************************************

/* TODO: old method, delete?

Curve Curve::get_simpified_curve(const float min_radius)
{
    std::list<size_t> points_to_use;
    for(size_t i = 0; i < vertices().size(); ++i)
        points_to_use.push_back(i);

    auto get_distance = [](const Scene_wireframe_vertex& v1,
                           const Scene_wireframe_vertex& v2)
    {
        auto d = v2 - v1;
        return std::sqrt(
            d(0) * d(0) + d(1) * d(1) + d(2) * d(2) + d(3) * d(3));
    };

    size_t num_removed_verts = 1;
    while(num_removed_verts != 0)
    {
        num_removed_verts = 0;
        if(points_to_use.size() < 3)
            continue;

        auto first = std::next(points_to_use.begin());
        auto last = std::prev(points_to_use.end());
        for(auto iter = first; iter != last; ++iter)
        {
            auto& a_vert = vertices()[*std::prev(iter)];
            auto& b_vert = vertices()[*iter           ];
            auto& c_vert = vertices()[*std::next(iter)];
            
            auto a = get_distance(b_vert, c_vert);
            auto b = get_distance(a_vert, c_vert);
            auto c = get_distance(a_vert, b_vert);
            auto p = 0.5f * (a + b + c);

            float r = std::sqrt((p - a) * (p - b) * (p - c) / p);

            if(r < min_radius)
            {
                num_removed_verts++;
                points_to_use.remove(*iter);
                break;
            }
        }
    }

    Curve simple_curve;
    for(auto i: points_to_use)
        simple_curve.add_point(vertices()[i], time_stamp()[i]);
    
    return simple_curve;
}*/

Curve Curve::get_simpified_curve_RDP(const float max_deviation)
{
    typedef boost::geometry::model::
        point<float, 4, boost::geometry::cs::cartesian>
            pt4d_t;
    typedef boost::geometry::model::linestring<pt4d_t> linestring4d_t;
    linestring4d_t line;

    auto to_boost = [](const Scene_wireframe_vertex& v) -> pt4d_t {
        pt4d_t out;
        out.set<0>(v(0));
        out.set<1>(v(1));
        out.set<2>(v(2));
        out.set<3>(v(3));
        return out;
    };
    for(auto v : vertices_)
        boost::geometry::append(line, to_boost(v));

    linestring4d_t simplified;
    boost::geometry::simplify(line, simplified, max_deviation);

    Curve simple_curve;
    size_t i = 0;
    auto is_equal = [](const pt4d_t& p,
                       const Scene_wireframe_vertex& v) -> bool {
        return p.get<0>() == v(0) && p.get<1>() == v(1) && p.get<2>() == v(2) &&
               p.get<3>() == v(3);
    };
    for(const auto& p4d : simplified)
    {
        while(!is_equal(p4d, vertices()[i]))
            ++i;
        simple_curve.add_point(vertices()[i], time_stamp()[i]);
    }
    return simple_curve;
}

//******************************************************************************
// update_stats
//******************************************************************************

void Curve::update_stats()
{
    calculate_general_stats();
    calculate_annotations();
}

//******************************************************************************
// get_stats
//******************************************************************************

const Curve_stats& Curve::get_stats()
{
    return stats_;
}

//******************************************************************************
// calculate_general_stats
//******************************************************************************

void Curve::calculate_general_stats()
{
    float kernel_size = 10.f;
    float epsilon = 10.f;

    stats_ = Curve_stats();

    // Fill with default values
    // out_stats.dimensionality.clear();
    stats_.dimensionality.resize(vertices_.size());
    for(auto& d : stats_.dimensionality)
        d = "xyzw";

    auto min_speed = std::numeric_limits<float>::max();
    auto max_speed = std::numeric_limits<float>::min();

    // Calculate speed
    for(const auto& e : edges_)
    {
        auto diff = vertices_[e.vert1] - vertices_[e.vert2];

        auto s = 0.f;
        for(int j = 0; j < 4; ++j)
            s += diff(j) * diff(j);
        s = std::sqrt(s) /
            std::abs(time_stamp_[e.vert2] - time_stamp_[e.vert1]);

        min_speed = std::min(s, min_speed);
        max_speed = std::max(s, max_speed);

        stats_.speed.push_back(s);
    }

    // Assign max and min speed to the statistic instance
    stats_.min_speed = min_speed;
    stats_.max_speed = max_speed;

    // Find how many dimension curve segments have
    Scene_wireframe_vertex min(5), max(5);
    float start_t = 0.f, end_t = 0.f;
    for(size_t i = 0; i < time_stamp_.size(); ++i)
    {
        start_t = time_stamp_[i];
        min = max = vertices_[i];
        for(size_t j = i; j < time_stamp_.size(); ++j)
        {
            end_t = time_stamp_[j];
            // Update min and max
            for(int k = 0; k < 5; ++k)
            {
                min(k) = std::min(min(k), vertices_[j](k));
                max(k) = std::max(max(k), vertices_[j](k));
            }

            if(end_t - start_t > kernel_size)
            {
                bool activeness[4];
                for(int k = 0; k < 4; ++k)
                    activeness[k] = (max(k) - min(k)) > epsilon;

                std::string dim = "";
                dim += activeness[0] ? "x" : "";
                dim += activeness[1] ? "y" : "";
                dim += activeness[2] ? "z" : "";
                dim += activeness[3] ? "w" : "";

                for(size_t k = i; k <= j; ++k)
                {
                    if(dim.length() < stats_.dimensionality[k].length())
                    {
                        stats_.dimensionality[k] = dim;
                    }
                }

                break;
            }
        }
    }

    for(size_t i = 1; i < stats_.dimensionality.size(); ++i)
    {
        if(stats_.dimensionality[i - 1] != stats_.dimensionality[i])
        {
            stats_.switches_inds.push_back(i);
        }
    }

    auto compute_range = [this](size_t ind1, size_t ind2) {
        Curve_stats::Range r;
        for(size_t i = ind1; i < ind2; ++i)
        {
            std::get<0>(r.x) = std::min(std::get<0>(r.x), vertices_[i](0));
            std::get<1>(r.x) = std::max(std::get<0>(r.x), vertices_[i](0));

            std::get<0>(r.y) = std::min(std::get<0>(r.y), vertices_[i](1));
            std::get<1>(r.y) = std::max(std::get<0>(r.y), vertices_[i](1));

            std::get<0>(r.z) = std::min(std::get<0>(r.z), vertices_[i](2));
            std::get<1>(r.z) = std::max(std::get<0>(r.z), vertices_[i](2));

            std::get<0>(r.w) = std::min(std::get<0>(r.w), vertices_[i](3));
            std::get<1>(r.w) = std::max(std::get<0>(r.w), vertices_[i](3));
        }
        stats_.range.push_back(r);
    };

    if(stats_.switches_inds.size() > 0)
        compute_range(0, stats_.switches_inds.front());

    for(size_t i = 1; i < stats_.switches_inds.size(); ++i)
        compute_range(stats_.switches_inds[i - 1], stats_.switches_inds[i]);

    if(stats_.switches_inds.size() > 0)
        compute_range(stats_.switches_inds.back(), vertices_.size());
}

//******************************************************************************
// calculate_annotations
//******************************************************************************

void Curve::calculate_annotations()
{
    arrows_.clear();
    markers_.clear();

    auto make_center_point = [&](size_t start, size_t end, std::string dim) {
        const float epsilon = 0.2f;

        Scene_wireframe_vertex center_pnt(5);
        center_pnt <<= 0, 0, 0, 0, 0;

        auto start_t = time_stamp_[start];
        auto end_t = time_stamp_[end];

        auto avrg_t = 0.5f * (start_t + end_t);

        Scene_wireframe_vertex current_point = get_point(avrg_t);
        Scene_wireframe_vertex dir = get_point(avrg_t + epsilon);

        Arrow_type a(avrg_t, static_cast<int>(dim.length()));
        arrows_.push_back(a);
    };

    size_t start_ind = 0, end_ind = 0;
    for(size_t i = 1; i < stats_.dimensionality.size(); ++i)
    {
        if(stats_.dimensionality[start_ind] == stats_.dimensionality[i])
        {
            end_ind = i;
        }
        else
        {
            make_center_point(start_ind, end_ind, stats_.dimensionality[i - 1]);
            markers_.push_back(i);
            start_ind = i;
        }
    }
    make_center_point(start_ind, end_ind, stats_.dimensionality[start_ind]);

    // Filter annotation points
    /*const double min_dist = 20;
    std::vector<Curve_annotations> filtered_annotations;
    for(size_t i = 0; i < annotations.size(); ++i)
    {
        auto& current = annotations[i];
        double dist = std::numeric_limits<double>::max();

        for(size_t j = i + 1; j < annotations.size(); ++j)
        {
            auto& next = annotations[j];
            auto v = next.point - current.point;
            double length = std::sqrt(
                v(0) * v(0) + v(1) * v(1) + v(2) * v(2) + v(3) * v(3));

            dist = std::min(length, dist);
        }

        if(dist > min_dist)
        {
            filtered_annotations.push_back(current);
        }
    }

    out_arrows = annotations;*/
}

//******************************************************************************
// get_arrows
//******************************************************************************

std::vector<Curve_annotations>
Curve::get_arrows(const Curve_selection& selection)
{
    std::vector<Curve_annotations> annotations;

    for(auto& a : arrows_)
    {
        float t = a.get<0>();

        if(!selection.in_range(t))
            continue;

        Curve_annotations annotation;
        annotation.point = get_point(t);
        annotation.dir = get_point(t + 0.01f);
        annotation.dimensionality = a.get<1>();

        annotations.push_back(annotation);
    }

    return annotations;
}

//******************************************************************************
// get_markers
//******************************************************************************

std::vector<Scene_wireframe_vertex>
Curve::get_markers(const Curve_selection& selection)
{
    std::vector<Scene_wireframe_vertex> res;

    for(auto& m : markers_)
    {
        if(selection.in_range(time_stamp_[m]))
            res.push_back(vertices_[m]);
    }

    return res;
}

//******************************************************************************
// translate_vertices
//******************************************************************************

void Curve::translate_vertices(const Scene_wireframe_vertex& translate)
{
    for(auto& v : vertices_)
        v += translate;
}

//******************************************************************************
// scale_vertices
//******************************************************************************

void Curve::scale_vertices(float scale_factor)
{
    for(auto& v : vertices_)
        v *= scale_factor;
}

//******************************************************************************
// scale_vertices
//******************************************************************************

void Curve::scale_vertices(const Scene_wireframe_vertex& scale_factor)
{
    for(auto& v : vertices_)
    {
        assert(v.size() == scale_factor.size());

        if(v.size() == scale_factor.size())
        {
            for(int i = 0; i < v.size(); i++)
                v[i] *= scale_factor[i];
        }
    }
}
