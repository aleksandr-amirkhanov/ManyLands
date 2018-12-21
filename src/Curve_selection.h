#pragma once

// A very simple structure secribign the curve selection
struct Curve_selection
{
    double t_start;
    double t_end;

    bool in_range(double t) const
    {
        return (t_start <= t && t <= t_end);
    }
};