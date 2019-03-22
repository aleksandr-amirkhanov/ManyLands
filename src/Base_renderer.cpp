#include "Base_renderer.h"

//******************************************************************************
// Base_renderer
//******************************************************************************

Base_renderer::Base_renderer()
    : region_(0.f, 0.f, 0.f, 0.f)
{
}

//******************************************************************************
// set_state
//******************************************************************************

void Base_renderer::set_state(std::shared_ptr<Scene_state> state)
{
    state_ = state;
}

//******************************************************************************
// set_redering_region
//******************************************************************************

void Base_renderer::set_redering_region(Region region,
                                        float scale_x,
                                        float scale_y)
{
    region_ = region;
    display_scale_x_ = scale_x;
    display_scale_y_ = scale_y;
}
