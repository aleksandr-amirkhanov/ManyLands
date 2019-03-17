#pragma once

// A very simple structure secribign the curve selection
struct Curve_selection
{
    // The curve selection is normalized to [0, 1]
    float t_start, t_end;

    bool in_range(double t) const
    {
        return (t_start <= t && t <= t_end);
    }
};
