#pragma once
// local
#include "Scene_state.h"
// std
#include <memory>

class Scene
{
public:
    Scene(std::shared_ptr<Scene_state> state);
    void load_ode(
        const std::vector<std::string>& fnames,
        float cuve_min_rad,
        float tesseract_size = 200.f);

private:
    std::shared_ptr<Curve> load_curve(std::string filename);
    void create_tesseract(
        Scene_wireframe_vertex shift,
        const Curve& curve);

    std::shared_ptr<Scene_state> state_;
    Scene_wireframe_vertex c_origin;
    Scene_wireframe_vertex c_size;
};
