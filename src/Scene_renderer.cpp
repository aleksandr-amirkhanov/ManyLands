#include "Scene_renderer.h"
// local
#include "Mesh_generator.h"

Scene_renderer::Scene_renderer()
    : tesseract_thickness_(1.f)
    , curve_thickness_(1.f)
    , sphere_diameter_(1.f)
{
}

Scene_renderer::Scene_renderer(std::shared_ptr<Scene_state> state)
    : Scene_renderer()
{
    set_state(state);
}

void Scene_renderer::set_state(std::shared_ptr<Scene_state> state)
{
    state_ = state;
}

void Scene_renderer::render()
{
    assert(state_);
    if(state_ == nullptr)
        return;

    geometry_.clear();

    if(state_->tesseract)
    {
        Tesseract t = *state_->tesseract;
        project_to_3D(t.get_vertices());
        draw_tesseract(t);
    }

    if(state_->curve)
    {
        Curve projected_c = *state_->curve;
        project_to_3D(projected_c.get_vertices());
        draw_curve(projected_c, 1.f);
    }

    for(auto& g : geometry_)
        g->draw_object();
}

void Scene_renderer::project_to_3D(
    boost::numeric::ublas::vector<double>& point)
{
    boost::numeric::ublas::vector<double> tmp_vert =
        prod(point, state_->rotation_4D);
    tmp_vert = tmp_vert - state_->camera_4D;
    tmp_vert = prod(tmp_vert, state_->projection_4D);

    //if(tmp_vert(3) < 0)
    //    gui_.distanceWarning->show();
    assert(tmp_vert(3) > 0);

    tmp_vert(0) /= tmp_vert(4);
    tmp_vert(1) /= tmp_vert(4);
    tmp_vert(2) /= tmp_vert(4);
    // Important!
    // Original coordinates in tmp_vert(3) and tmp_vert(4) are kept!
    // It is required for the 4D perspective.

    point = tmp_vert;
}

void Scene_renderer::project_to_3D(
    std::vector<boost::numeric::ublas::vector<double>>& verts)
{
    for(auto& v : verts)
        project_to_3D(v);
}

void Scene_renderer::draw_tesseract(Wireframe_object& t)
{
    for(auto const& e : t.get_edges())
    {
        Mesh t_mesh;
        auto& current = t.get_vertices().at(e->vert1);
        auto& next = t.get_vertices().at(e->vert2);
        //QColor col(e->color.r, e->color.g, e->color.b);
        glm::vec4 col(
            (float)e->color->r / 255,
            (float)e->color->g / 255,
            (float)e->color->b / 255,
            1.f);

        Mesh_generator::cylinder(
            5,
            tesseract_thickness_ / current(3),
            tesseract_thickness_ / next(3),
            glm::vec3(current(0), current(1), current(2)),
            glm::vec3(next(0), next(1), next(2)),
            col,
            t_mesh);

       geometry_.push_back(std::make_unique<Geometry_engine>(t_mesh));
    }

    for(unsigned int i = 0; i < t.get_vertices().size(); ++i)
    {
        double size_coef = 1.0f;

        auto const& v = t.get_vertices()[i];
        glm::vec3 pos(v(0), v(1), v(2));
        Mesh s_mesh;

        if(i == 0)
            Mesh_generator::sphere(
                16,
                16,
                size_coef * sphere_diameter_ / v(3),
                pos,
                glm::vec4(1.f, 0.f, 0.f, 1.f),
                s_mesh);
        else
            Mesh_generator::sphere(
                16,
                16,
                size_coef * sphere_diameter_ / v(3),
                pos,
                glm::vec4(0.59f, 0.59f, 0.59f, 1.f),
                s_mesh);

        geometry_.push_back(std::make_unique<Geometry_engine>(s_mesh));
    }
}

void Scene_renderer::draw_curve(Curve& c, float opacity)
{
    const double marker_size = 3.f; //gui_.markerSize->value();

    auto log_speed = [](float speed) {
        return std::log2(3 * speed + 1) / 2;
    };

    auto get_speed_color =
        [&opacity, &log_speed, this](float normalized_speed) {
        float speed = log_speed(normalized_speed);
        return glm::vec4(
            (float)((1 - speed) * state_->get_color(Curve_low_speed)->r +
                    speed * state_->get_color(Curve_high_speed)->r) / 255,
            (float)((1 - speed) * state_->get_color(Curve_low_speed)->g +
                    speed * state_->get_color(Curve_high_speed)->r) / 255,
            (float)((1 - speed) * state_->get_color(Curve_low_speed)->b +
                    speed * state_->get_color(Curve_high_speed)->r) / 255,
            opacity);
    };

    // Curve
    Mesh curve_mesh;
    for(size_t i = 0; i < c.get_edges().size(); ++i)
    {
        const auto& e = c.get_edges()[i];

        auto& current = c.get_vertices().at(e->vert1);
        auto& next = c.get_vertices().at(e->vert2);

        // We are interested only in some interval of the curve
        if(state_->curve_selection &&
           !state_->curve_selection->in_range(c.get_time_stamp().at(e->vert1)))

        {
            continue;
        }

        /*QColor col = get_speed_color(
            (stats.speed[i] - stats.min_speed) /
            (stats.max_speed - stats.min_speed));*/
        auto& stats = c.get_stats();
        double speed_coeff = (stats.speed[i] - stats.min_speed) /
                             (stats.max_speed - stats.min_speed);

        Mesh_generator::cylinder(
            5,
            curve_thickness_ / current(3),
            curve_thickness_ / next(3),
            glm::vec3(current(0), current(1), current(2)),
            glm::vec3(next(0), next(1), next(2)),
            get_speed_color(speed_coeff),
            curve_mesh);
    }
    //gui_.Renderer->add_mesh(curve_mesh, opacity < 1.);
    geometry_.push_back(std::make_unique<Geometry_engine>(curve_mesh));

    /*boost::numeric::ublas::vector<double> marker = c.get_point(player_pos_);

    Mesh marker_mesh;
    Mesh_generator::sphere(
        16,
        16,
        marker_size / marker(3),
        glm::vec3(marker(0), marker(1), marker(2)),
        glm::vec4(1, 0, 0, 1),
        marker_mesh);

    //gui_.Renderer->add_mesh(marker_mesh);
    geometry_.push_back(std::make_unique<Geometry_engine>(marker_mesh));*/
}

void Scene_renderer::set_line_thickness(float t_thickness, float c_thickness)
{
    tesseract_thickness_ = t_thickness;
    curve_thickness_ = c_thickness;
}

void Scene_renderer::set_sphere_diameter(float diameter)
{
    sphere_diameter_ = diameter;
}
