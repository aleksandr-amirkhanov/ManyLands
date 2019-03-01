#include "Curve.h"
// std
#include <algorithm>
#include <cmath>
// boost
#include <boost/numeric/ublas/assignment.hpp>

void Curve::add_point(Point_type vertex, float time)
{
    // Adding vertex
    get_vertices().push_back(vertex);

    // Adding edge
    if(get_vertices().size() > 1)
        add_edge(get_vertices().size() - 2, get_vertices().size() - 1);

    // Adding time stamp
    time_stamp_.push_back(time);
}

boost::numeric::ublas::vector<double> Curve::get_point(float time)
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
        size_t middle = 0.5 * (range[1] + range[0]);
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

std::vector<float> Curve::get_time_stamp() const
{
    return time_stamp_;
}

void Curve::shift_to_origin(double max_edge_length, Point_type& out_shift)
{
    boost::numeric::ublas::vector<double> origin(5);
    boost::numeric::ublas::vector<double> size(5);
    get_boundaries(origin, size);

    translate_vertices(-0.5f * size - origin);
}

void Curve::get_boundaries(Point_type& origin, Point_type& size) const
{
    // Finding minimum and maximum values of the curve
    boost::numeric::ublas::vector<double> min(5);
    boost::numeric::ublas::vector<double> max(5);

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

Curve Curve::get_simpified_curve(const double spacing)
{
    Curve simplified_curve;

    const size_t num_verts = get_vertices().size();
    // We always add the first points
    if(num_verts > 0)
        simplified_curve.add_point(get_vertices()[0], get_time_stamp()[0]);

    // Ading points that are the the minimum distance from each other
    double dist = 0; // Distance from the last point of the simplified curve
    for(size_t i = 1; i < num_verts - 1; ++i)
    {
        auto get_distance = [](boost::numeric::ublas::vector<double> v1,
                               boost::numeric::ublas::vector<double> v2) {
            auto d = v2 - v1;
            return std::sqrt(
                d(0) * d(0) + d(1) * d(1) + d(2) * d(2) + d(3) * d(3));
        };

        dist += get_distance(get_vertices()[i - 1], get_vertices()[i]);

        if(dist > spacing)
        {
            dist = 0;
            // Ok, it's time to add a curve point
            simplified_curve.add_point(get_vertices()[i], get_time_stamp()[i]);
        }
        else
        {
            continue;
        }
    }

    // We always add the last point
    if(num_verts > 1)
    {
        simplified_curve.add_point(
            get_vertices()[num_verts - 1], get_time_stamp()[num_verts - 1]);
    }

    return simplified_curve;
}

void Curve::update_stats()
{
    calculate_general_stats();
    calculate_annotations();
}

const Curve_stats& Curve::get_stats()
{
    return stats_;
}

void Curve::calculate_general_stats()
{
    double kernel_size = 10.;
    double epsilon = 10;

    stats_ = Curve_stats();

    // Fill with default values
    // out_stats.dimensionality.clear();
    stats_.dimensionality.resize(vertices_.size());
    for(auto& d : stats_.dimensionality)
        d = "xyzw";

    double min_speed = std::numeric_limits<double>::max();
    double max_speed = std::numeric_limits<double>::min();

    // Calculate speed
    for(const auto& e : edges_)
    {
        auto diff = vertices_[e->vert1] - vertices_[e->vert2];

        double s = 0.;
        for(int j = 0; j < 4; ++j)
            s += diff(j) * diff(j);
        s = std::sqrt(s) /
            std::abs(time_stamp_[e->vert2] - time_stamp_[e->vert1]);

        min_speed = std::min(s, min_speed);
        max_speed = std::max(s, max_speed);

        stats_.speed.push_back(s);
    }

    // Assign max and min speed to the statistic instance
    stats_.min_speed = min_speed;
    stats_.max_speed = max_speed;

    // Find how many dimension curve segments have
    boost::numeric::ublas::vector<double> min(5), max(5);
    double start_t = 0., end_t = 0.;
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
            std::get<0>(r.x) =
                std::min((double)std::get<0>(r.x), vertices_[i](0));
            std::get<1>(r.x) =
                std::max((double)std::get<0>(r.x), vertices_[i](0));

            std::get<0>(r.y) =
                std::min((double)std::get<0>(r.y), vertices_[i](1));
            std::get<1>(r.y) =
                std::max((double)std::get<0>(r.y), vertices_[i](1));

            std::get<0>(r.z) =
                std::min((double)std::get<0>(r.z), vertices_[i](2));
            std::get<1>(r.z) =
                std::max((double)std::get<0>(r.z), vertices_[i](2));

            std::get<0>(r.w) =
                std::min((double)std::get<0>(r.w), vertices_[i](3));
            std::get<1>(r.w) =
                std::max((double)std::get<0>(r.w), vertices_[i](3));
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

void Curve::calculate_annotations()
{
    arrows_.clear();
    markers_.clear();

    auto make_center_point = [&](size_t start, size_t end, std::string dim) {
        const double epsilon = 0.2;

        Point_type center_pnt(5);
        center_pnt <<= 0, 0, 0, 0, 0;

        double start_t = time_stamp_[start];
        double end_t = time_stamp_[end];

        double avrg_t = 0.5 * (start_t + end_t);

        Point_type current_point = get_point(avrg_t);
        Point_type dir = get_point(avrg_t + epsilon);

        Arrow_type a(avrg_t, dim.length());
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
        annotation.dir = get_point(t + 0.01);
        annotation.dimensionality = a.get<1>();

        annotations.push_back(annotation);
    }

    return annotations;
}

std::vector<Curve::Point_type>
Curve::get_markers(const Curve_selection& selection)
{
    std::vector<Point_type> res;

    for(auto& m : markers_)
    {
        if(selection.in_range(time_stamp_[m]))
            res.push_back(vertices_[m]);
    }

    return res;
}
