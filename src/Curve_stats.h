#pragma once
// std
#include <string>
#include <vector>
#include <tuple>
#include <limits>

struct Curve_stats
{
    struct Range
    {
        typedef std::tuple<float, float> Min_and_max;
        Range()
        {
            std::get<0>(x) = std::numeric_limits<float>::max();
            std::get<1>(x) = std::numeric_limits<float>::min();

            std::get<0>(y) = std::numeric_limits<float>::max();
            std::get<1>(y) = std::numeric_limits<float>::min();

            std::get<0>(z) = std::numeric_limits<float>::max();
            std::get<1>(z) = std::numeric_limits<float>::min();

            std::get<0>(w) = std::numeric_limits<float>::max();
            std::get<1>(w) = std::numeric_limits<float>::min();
        }

        Min_and_max x;
        Min_and_max y;
        Min_and_max z;
        Min_and_max w;
    };

    float min_speed, max_speed;
    // These two vectors should be of the size of the curve (number of points)
    std::vector<float> speed;
    std::vector<std::string> dimensionality;
    // Vectors bellow have the size depending from number of switches
    std::vector<size_t> switches_inds;
    std::vector<Range> range; 
};
