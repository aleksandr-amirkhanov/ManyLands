#pragma once

struct Color
{
public:
    Color();
    Color(int r, int g, int b, int a = 255);

    int r() const;
    int g() const;
    int b() const;
    int a() const;

    float r_norm() const;
    float g_norm() const;
    float b_norm() const;
    float a_norm() const;

    /*
    TODO: remove this code
    void copy(const Color& other)
    {
        this->r_ = other.r_;
        this->g_ = other.g_;
        this->b_ = other.b_;
        this->a_ = other.a_;
    }*/

private:
    int r_, g_, b_, a_; // Red, green, blue, alpha
};
