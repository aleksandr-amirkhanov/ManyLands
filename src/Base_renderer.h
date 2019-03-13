#pragma once

// Local
#include "Scene_state.h"
// glm
#include <glm/glm.hpp>
// std
#include <memory>

class Base_renderer
{
public:
    struct Region
    {
    public:
        Region(float left, float bottom, float right, float top)
        {
            left_   = left;
            bottom_ = bottom;
            right_  = right;
            top_    = top;
        }

        float left() const
        {
            return left_;
        }
        float bottom() const
        {
            return bottom_;
        }
        float right() const
        {
            return right_;
        }
        float top() const
        {
            return top_;
        }
        float width() const
        {
            return right_ - left_;
        }
        float height() const
        {
            return top_ - bottom_;
        }
    private:
        float left_, bottom_, right_, top_;
    };

    Base_renderer();

    virtual void set_state(std::shared_ptr<Scene_state> state);
    virtual void set_redering_region(Region region,
                                     float scale_x,
                                     float scale_y);
    virtual void render() = 0;

protected:
    std::shared_ptr<Scene_state> state_;
    Region region_;
    float display_scale_x_, display_scale_y_;
};
