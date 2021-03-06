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
    struct Renderer_io
    {
        enum Key {
            Key_0,
            Key_1,
            Key_2,
            Key_3,
            Key_4,
            Key_5,
            Key_6,
            Key_7,
            Key_8,
            Key_F1,
            Key_F2,
            Key_F3,
            Key_F4,
            Key_F5,
            Key_F6,
            Key_F7,
            Key_F8
        };
        Renderer_io()
            : mouse_pos(0.f, 0.f)
            , mouse_move(0.f, 0.f)
            , mouse_down(false)
            , mouse_wheel(0.f)
        { }

        glm::vec2 mouse_pos;
        glm::vec2 mouse_move;
        bool      mouse_down;
        bool      mouse_up;
        bool      mouse_wheel;
        float     mouse_wheel_y;

        bool key_pressed;
        Key key;
    };

    struct Region
    {
    public:
        Region()
        {
            left_   = bottom_ = right_ = top_ = 0.f;
        }

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
        bool contains(glm::vec2 pos) const
        {
            return contains(pos.x, pos.y);
        }
        bool contains(float x, float y) const
        {
            return (left_ <= x && x <= right_ && bottom_ <= y && y <= top_);
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
    virtual void process_input(const Renderer_io& io) = 0;

protected:
    std::shared_ptr<Scene_state> state_;
    Region region_;
    float display_scale_x_, display_scale_y_;
};
