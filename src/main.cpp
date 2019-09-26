#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

// Win
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <windows.h>
#include <commdlg.h>
#endif
// ImGui
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
// SDL
#include <SDL.h>
#include <SDL_syswm.h>
// std
#include <chrono>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#ifdef __EMSCRIPTEN__
#include <fstream>
#endif
// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
// Local
#include "Mesh_generator.h"
#include "Geometry_engine.h"
#include "Scene.h"
#include "Scene_state.h"
#include "Scene_renderer.h"
#include "Consts.h"
#include "Matrix_lib.h"
#include "Tesseract.h"
#include "Text_renderer.h"
#include "Timeline_renderer.h"
#include "Diffuse_shader.h"
#include "Screen_shader.h"
// Boost
#include <boost/numeric/ublas/assignment.hpp>

#if defined(USE_GL_ES3)
#include <GLES3/gl3.h>  // Use GL ES 3
#else
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif
#endif

namespace
{
//******************************************************************************
// Variables
//******************************************************************************

bool done = false;

SDL_DisplayMode Current;
SDL_WindowFlags Window_flags;
SDL_Window*     MainWindow;
SDL_GLContext   Gl_context;

ImVec4 Clear_color,
       X_axis_color,
       Y_axis_color,
       Z_axis_color,
       W_axis_color,
       Low_speed_color,
       High_speed_color;

const bool Enable_keyboard = true,   // Controllers
           Enable_gamepad  = false;

const float App_scale = 1.f;          // Global app scale

const int Bottom_panel_size = static_cast<int>(36 * App_scale);
int Left_panel_size = static_cast<int>(300 * App_scale);

const std::shared_ptr<Scene_state> State = std::make_shared<Scene_state>();
Scene_renderer Renderer(State);
Timeline_renderer Timeline(State);

const std::shared_ptr<Diffuse_shader> Diffuse_shad =
    std::make_shared<Diffuse_shader>();
const std::shared_ptr<Screen_shader> Screen_shad =
    std::make_shared<Screen_shader>();
const std::shared_ptr<Text_renderer> Text_ren =
    std::make_shared<Text_renderer>();

Scene Scene_objs(State);

Base_renderer::Region Scene_region, Timeline_region;
Base_renderer::Renderer_io Previous_io;

std::chrono::time_point<std::chrono::system_clock> Last_timepoint;

// Timeplayer
auto Is_player_active(false);
auto Player_speed(0.1f);

auto Curve_max_deviation(0.8f);

//******************************************************************************
// Color_to_ImVec4
//******************************************************************************

ImVec4 Color_to_ImVec4(Color c)
{
    return ImVec4(c.r_norm(), c.g_norm(), c.b_norm(), c.a_norm());
}

//******************************************************************************
// set_bright_theme
//******************************************************************************

void set_bright_theme()
{
    Clear_color      = Color_to_ImVec4(Color(255, 255, 255));
    X_axis_color     = Color_to_ImVec4(Color(215, 25,   28));
    Y_axis_color     = Color_to_ImVec4(Color(253, 174,  97));
    Z_axis_color     = Color_to_ImVec4(Color(171, 217, 233));
    W_axis_color     = Color_to_ImVec4(Color( 44, 123, 182));
    Low_speed_color  = Color_to_ImVec4(Color( 0,    0,   0));
    High_speed_color = Color_to_ImVec4(Color(220, 220, 220));
}

//******************************************************************************
// set_dark_theme
//******************************************************************************

void set_dark_theme()
{
    Clear_color      = Color_to_ImVec4(Color(  0,   0,   0));
    X_axis_color     = Color_to_ImVec4(Color(215,  25,  28));
    Y_axis_color     = Color_to_ImVec4(Color(253, 174,  97));
    Z_axis_color     = Color_to_ImVec4(Color(171, 217, 233));
    W_axis_color     = Color_to_ImVec4(Color( 44, 123, 182));
    Low_speed_color  = Color_to_ImVec4(Color(255, 255, 255));
    High_speed_color = Color_to_ImVec4(Color( 35,  35,  35));
}

//******************************************************************************
// process_mouse_input
//******************************************************************************

void process_mouse_input(const SDL_Event& event)
{
    int height = (int)ImGui::GetIO().DisplaySize.y;
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    mouse_y = height - mouse_y;

    Base_renderer::Renderer_io io;
    io.mouse_pos  = glm::vec2(mouse_x, mouse_y);
    io.mouse_move = glm::vec2(io.mouse_pos.x - Previous_io.mouse_pos.x,
                              io.mouse_pos.y - Previous_io.mouse_pos.y);
    io.mouse_down = (event.button.button == SDL_BUTTON_LEFT &&
                     event.type == SDL_MOUSEBUTTONDOWN);
    io.mouse_up = (event.type == SDL_MOUSEBUTTONUP);
    io.mouse_wheel = (event.type == SDL_MOUSEWHEEL);
    io.mouse_wheel_y = 0;
    if(io.mouse_wheel)
        io.mouse_wheel_y = event.wheel.y > 0.f ? 3.f : -3.f;

    if(event.type == SDL_KEYDOWN)
    {
        io.key_pressed = true;
        switch(event.key.keysym.sym)
        {
            case SDLK_0:
                io.key = Base_renderer::Renderer_io::Key_0;
                break;
            case SDLK_1:
                io.key = Base_renderer::Renderer_io::Key_1;
                break;
            case SDLK_2:
                io.key = Base_renderer::Renderer_io::Key_2;
                break;
            case SDLK_3:
                io.key = Base_renderer::Renderer_io::Key_3;
                break;
            case SDLK_4:
                io.key = Base_renderer::Renderer_io::Key_4;
                break;
            case SDLK_5:
                io.key = Base_renderer::Renderer_io::Key_5;
                break;
            case SDLK_6:
                io.key = Base_renderer::Renderer_io::Key_6;
                break;
            case SDLK_7:
                io.key = Base_renderer::Renderer_io::Key_7;
                break;
            case SDLK_8:
                io.key = Base_renderer::Renderer_io::Key_8;
                break;
            case SDLK_F1:
                io.key = Base_renderer::Renderer_io::Key_F1;
                break;
            case SDLK_F2:
                io.key = Base_renderer::Renderer_io::Key_F2;
                break;
            case SDLK_F3:
                io.key = Base_renderer::Renderer_io::Key_F3;
                break;
            case SDLK_F4:
                io.key = Base_renderer::Renderer_io::Key_F4;
                break;
            case SDLK_F5:
                io.key = Base_renderer::Renderer_io::Key_F5;
                break;
            case SDLK_F6:
                io.key = Base_renderer::Renderer_io::Key_F6;
                break;
            case SDLK_F7:
                io.key = Base_renderer::Renderer_io::Key_F7;
                break;
            case SDLK_F8:
                io.key = Base_renderer::Renderer_io::Key_F8;
                break;
            default:
                break;
        }
    }
    else
    {
        io.key_pressed = false;
    }

    Renderer.process_input(io);
    Timeline.process_input(io);

    Previous_io = io;
}

//******************************************************************************
// update_timer
//******************************************************************************

void update_timer()
{
    auto current = std::chrono::system_clock::now();
    std::chrono::duration<double> delta_t = current - Last_timepoint;
    Last_timepoint = current;

    if(Is_player_active)
    {
        State->timeplayer_pos = std::fmod(
            State->timeplayer_pos +
            static_cast<float>(delta_t.count()) * Player_speed, 1.f);
    }
}

const char* Message = "";

void MessageCallback(GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  //fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
  //         ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
  //          type, severity, message );

  Message = message;
}

//******************************************************************************
// mainloop
//******************************************************************************

void mainloop()
{
    update_timer();

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
        {
            done = true;
        }
        if (   event.type == SDL_WINDOWEVENT
            && event.window.event == SDL_WINDOWEVENT_CLOSE
            && event.window.windowID == SDL_GetWindowID(MainWindow))
        {
            done = true;
        }

        if(!io.WantCaptureMouse)
        {
            process_mouse_input(event);
        }
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(MainWindow);
    ImGui::NewFrame();

    static float fov_y = 45.f * static_cast<float>(DEG_TO_RAD);

    int width = (int)io.DisplaySize.x;
    int height = (int)io.DisplaySize.y;

    static float timeline_height(200.f),
                 pictograms_size(50.f),
                 pictogram_scale(1.5f),
                 splitter(0.5f);

    static int pictogram_region = 4;

    // ImGui windows start
    {
        static float tesseract_thickness = 3.f,
                     curve_thickness = 3.f,
                     sphere_diameter = 3.f,
                     camera_3D_dist  = 3.f,
                     xy_rot = 0.f,
                     yz_rot = 0.f,
                     zx_rot = 0.f,
                     xw_rot = 0.f,
                     yw_rot = 0.f,
                     zw_rot = 0.f,
                     fov_4d[3] = { 30.f * static_cast<float>(DEG_TO_RAD),
                                   30.f * static_cast<float>(DEG_TO_RAD),
                                   30.f * static_cast<float>(DEG_TO_RAD) },
                     fog_dist = 10.f,
                     fog_range = 2.f;

        ImGui::SetNextWindowPos(ImVec2(400, 0));
        ImGui::Begin("Debug output");
        ImGui::Text((char*)glGetString(GL_VERSION));
        ImGui::Text("OpenGL error: %d", glGetError());
        ImGui::Text(Message);
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(Left_panel_size),
                                        static_cast<float>(height)));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(0.f, static_cast<float>(height)),
            ImVec2(FLT_MAX, static_cast<float>(height)));

        ImGui::Begin(
            "Control panel",
            0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize);

        ImGui::Text("%.1f FPS", io.Framerate);

        if(ImGui::Button("Load ODE"))
        {
#if __EMSCRIPTEN__
            EM_ASM(
                uploadFile();
            );
#else
#ifdef _WIN32
            std::vector<std::string> fnames;
            char fn[MAX_PATH];

            SDL_SysWMinfo wm_info;
            SDL_VERSION(&wm_info.version);
            SDL_GetWindowWMInfo(MainWindow, &wm_info);

            OPENFILENAMEA ofn;
            ZeroMemory( &fn, sizeof( fn ) );
            ZeroMemory( &ofn, sizeof( ofn ) );
            ofn.lStructSize  = sizeof(ofn);
            ofn.hwndOwner    = wm_info.info.win.window;
            ofn.lpstrFilter  = "Text Files\0*.txt\0Any File\0*.*\0";
            ofn.lpstrFile    = fn;
            ofn.nMaxFile     = MAX_PATH;
            ofn.lpstrTitle   = "Select an ODE";
            ofn.Flags        = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST |
                               OFN_ALLOWMULTISELECT | OFN_EXPLORER;

            GetOpenFileNameA(&ofn);

            std::vector<std::string> out_dialog_data;
            char* str = fn;
            if(*str)
            {
                out_dialog_data.emplace_back(str);
                str += out_dialog_data.back().length() + 1;
                while(*str)
                {
                    out_dialog_data.emplace_back(str);
                    str += out_dialog_data.back().length() + 1;
                }
            }

            if(out_dialog_data.size() == 1)
            {
                fnames.emplace_back(out_dialog_data.front());
            }
            else if(out_dialog_data.size() >= 3)
            {
                // If two of more files are opened, then the first elemens
                // always indicates the directory of the files
                for(size_t i = 1; i < out_dialog_data.size(); ++i)
                {
                    fnames.emplace_back(
                        out_dialog_data[0] + "\\" + out_dialog_data[i]);
                }
            }
#else
            std::vector<std::string> fnames;
            fnames.push_back("assets/model2-default.txt");
#endif
            if(!fnames.empty())
            {
                Scene_objs.load_ode(
                    fnames,
                    Curve_max_deviation);
            }
#endif
        }
        ImGui::SameLine();
        ImGui::Checkbox("Scale tesseract", &State->scale_tesseract);

        if (ImGui::CollapsingHeader("Player"))
        {
            static auto time(0.f);
            if(!Is_player_active)
            {
                if(ImGui::Button("Start"))
                {
                    Is_player_active = true;
                }
            }
            else
            {
                if(ImGui::Button("Stop"))
                {
                    Is_player_active = false;
                }

            }
            ImGui::SameLine();
            ImGui::Checkbox("Show timepoint", &State->is_timeplayer_active);
            if(!State->is_timeplayer_active) Is_player_active = false;

            ImGui::SliderFloat("Time", &State->timeplayer_pos, 0.f, 1.f);
            ImGui::SliderFloat("Speed", &Player_speed, 0.f, 0.5f);
        }

        if (ImGui::CollapsingHeader("Curve simplification"))
        {
            ImGui::SliderFloat("Max. deviation", &Curve_max_deviation, 0.f, 3.f);
        }

        // We cannot use std::vector<bool> becase it is impossible to get
        // a reference from such structure and pass it to ImGui (mistake in std)
        static bool show_x(true), show_y(true), show_z(true), show_w(true);

        if (ImGui::CollapsingHeader("Rendering"))
        {
            ImGui::Text("Visibility:");
            ImGui::Checkbox("Tesseract##visibility", &State->show_tesseract);
            ImGui::SameLine();
            ImGui::Checkbox("Curve##visibility", &State->show_curve);
            ImGui::SameLine();
            ImGui::Checkbox("Legend##visibility", &State->show_legend);

            ImGui::Checkbox("Simple Dali", &State->use_simple_dali_cross);

            ImGui::Separator();
            ImGui::Text("Geometry thickness:");

            ImGui::SliderFloat(
                "Tesseract##thickness", &tesseract_thickness, 0.1f, 10.0f);
            ImGui::SliderFloat(
                "Curve##thickness", &curve_thickness, 0.1f, 10.0f);
            ImGui::SliderFloat(
                "Spheres", &sphere_diameter, 0.1f, 10.0f);

            ImGui::Separator();
            ImGui::Text("Layout:");

            ImGui::SliderFloat(
                "Splitter", &splitter, 0.05f, 0.95f);
            ImGui::SliderFloat(
                "Timeline", &timeline_height, 10.f, 1000.0f);
            ImGui::SliderFloat(
                "Pictograms", &pictograms_size, 10.f, 100.0f);

            ImGui::Separator();
            ImGui::Text("Show axes:");
            ImGui::Checkbox("X##line", &show_x);
            ImGui::SameLine();
            ImGui::Checkbox("Y##line", &show_y);
            ImGui::SameLine();
            ImGui::Checkbox("Z##line", &show_z);
            ImGui::SameLine();
            ImGui::Checkbox("W##line", &show_w);

            ImGui::Separator();
            ImGui::Text("Pictogram magnification:");
            ImGui::SliderFloat(
                "Scale", &pictogram_scale, 0.5f, 10.f);
            ImGui::SliderInt(
                "Region", &pictogram_region, 1, 10);

            ImGui::Separator();

            ImGui::Text("Colors:");

            ImGui::Text("Set theme:");
            ImGui::SameLine();
            if(ImGui::Button("Bright")) set_bright_theme();
            ImGui::SameLine();
            if(ImGui::Button("Dark"))   set_dark_theme();

            ImGui::Checkbox(
                "Use unique curve colors",
                &State->use_unique_curve_colors);

            ImGui::ColorEdit3("Background", (float*)&Clear_color);
            ImGui::ColorEdit3("X-axis", (float*)&X_axis_color);
            ImGui::ColorEdit3("Y-axis", (float*)&Y_axis_color);
            ImGui::ColorEdit3("Z-axis", (float*)&Z_axis_color);
            ImGui::ColorEdit3("W-axis", (float*)&W_axis_color);

            ImGui::ColorEdit3(
                "Curve slow", (float*)&Low_speed_color
            );
            ImGui::ColorEdit3(
                "Curve fast", (float*)&High_speed_color
            );

            ImGui::Separator();

            ImGui::Text("Fog:");
            ImGui::SliderFloat("Distance##fog", &fog_dist,  0.1f, 10.f);
            ImGui::SliderFloat("Range##fog",    &fog_range, 0.1f, 10.f);
        }

        Timeline.show_axes(std::vector<bool> {show_x, show_y, show_z, show_w});

        if (ImGui::CollapsingHeader("4D projection"))
        {
            ImGui::Text("Field of view:");
            ImGui::SliderAngle("Height", &fov_4d[0], 1.f, 180.f);
            ImGui::SliderAngle("Width", &fov_4d[1], 1.f, 180.f);
            ImGui::SliderAngle("Depth", &fov_4d[2], 1.f, 180.f);

            ImGui::Separator();
            ImGui::Text("Rotations (Euler angles):");
            ImGui::SliderAngle("XY", &xy_rot, -180.f, 180.f);
            ImGui::SliderAngle("YZ", &yz_rot, -180.f, 180.f);
            ImGui::SliderAngle("ZX", &zx_rot, -180.f, 180.f);
            ImGui::SliderAngle("XW", &xw_rot, -180.f, 180.f);
            ImGui::SliderAngle("YW", &yw_rot, -180.f, 180.f);
            ImGui::SliderAngle("ZW", &zw_rot, -180.f, 180.f);
            if(ImGui::Button("Reset##4D"))
            {
                xy_rot = yz_rot = zx_rot = xw_rot = yw_rot = zw_rot = 0.f;
            }
        }

        static float euler[3];
        static bool active = false;
        if(!active)
        {
            glm::extractEulerAngleXYZ(glm::toMat4(State->rotation_3D),
                                        euler[0],
                                        euler[1],
                                        euler[2]);
        }

        if (ImGui::CollapsingHeader("Cameras"))
        {
            ImGui::Text("Basics:");
            const float dist_min = 0.5f, dist_max = 5.f;
            camera_3D_dist = std::clamp((float)(-State->camera_3D.z),
                                        dist_min,
                                        dist_max);
            ImGui::SliderFloat("Distance", &camera_3D_dist, dist_min, dist_max);
            State->camera_3D.z = -camera_3D_dist;

            ImGui::SliderAngle("Field of view", &fov_y, 1.f, 180.f);

            ImGui::Separator();
            ImGui::Text("Rotations (Euler angles):");
            active = false;
            ImGui::SliderAngle("X", &euler[0], -180.f, 180.f);
            active |= ImGui::IsItemActive();
            ImGui::SliderAngle("Y", &euler[1], -180.f, 180.f);
            active |= ImGui::IsItemActive();
            ImGui::SliderAngle("Z", &euler[2], -180.f, 180.f);
            active |= ImGui::IsItemActive();
            if(ImGui::Button("Reset##3D"))
            {
                for(int i = 0; i < 3; ++i)
                    euler[i] = 0.f;
            }
        }

        if (ImGui::CollapsingHeader("Switch detection"))
        {
            ImGui::SliderFloat(
                "Kernel size", &State->stat_kernel_size, 0.f, 0.1f);
            ImGui::SliderFloat(
                "Max movement", &State->stat_max_movement, 0.f, 0.05f);
            ImGui::SliderFloat(
                "Value threshold", &State->stat_max_value, 0.f, 0.1f);
            if(ImGui::Button("Update"))
            {
                for(auto& c: State->curves)
                {
                    c->update_stats(
                        State->stat_kernel_size,
                        State->stat_max_movement,
                        State->stat_max_value);
                }
            }
        }

        State->rotation_3D = glm::eulerAngleXYZ(euler[0],
                                                euler[1],
                                                euler[2]);
        State->fov_y = fov_y;

        Left_panel_size = static_cast<int>(ImGui::GetWindowSize().x);
        ImGui::End();

        ImGui::SetNextWindowSize(
            ImVec2(static_cast<float>(width - Left_panel_size),
                   static_cast<float>(Bottom_panel_size)));
        ImGui::SetNextWindowPos(
            ImVec2(static_cast<float>(Left_panel_size),
                   static_cast<float>(height - Bottom_panel_size)));
        ImGui::Begin(
            "##animation",
            0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize);
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("##label", &State->unfolding_anim, 0.f, 1.f);
        static bool was_unfolding_slider_active = false;
        if(was_unfolding_slider_active && !ImGui::IsItemActive())
        {
            const float epsilon(0.01f);

            std::vector<float> milestones;
            for(char i = 0; i <= 6; ++i) milestones.push_back(i / 6.f);

            for(auto mstone : milestones)
            {
                if(std::abs(State->unfolding_anim - mstone) < epsilon)
                    State->unfolding_anim = mstone;
            }
        }
        was_unfolding_slider_active = ImGui::IsItemActive();
        ImGui::End();

        State->projection_4D = Matrix_lib_f::get4DProjectionMatrix(
            fov_4d[0], fov_4d[1], fov_4d[2], 1.f, 10.f);

        State->xy_rot = xy_rot;
        State->yz_rot = yz_rot;
        State->zx_rot = zx_rot;
        State->xw_rot = xw_rot;
        State->yw_rot = yw_rot;
        State->zw_rot = zw_rot;

        // Helper lambda expression
        auto ImVec4_to_Color = [](ImVec4 v) {
            return Color(static_cast<int>(v.x * 255),
                         static_cast<int>(v.y * 255),
                         static_cast<int>(v.z * 255),
                         static_cast<int>(v.w * 255));
        };

        State->update_color(Background,       ImVec4_to_Color(Clear_color));
        State->update_color(X_axis,           ImVec4_to_Color(X_axis_color));
        State->update_color(Y_axis,           ImVec4_to_Color(Y_axis_color));
        State->update_color(Z_axis,           ImVec4_to_Color(Z_axis_color));
        State->update_color(W_axis,           ImVec4_to_Color(W_axis_color));
        State->update_color(Curve_low_speed,  ImVec4_to_Color(Low_speed_color));
        State->update_color(
            Curve_high_speed, ImVec4_to_Color(High_speed_color));

        Renderer.set_line_thickness(tesseract_thickness, curve_thickness);
        Renderer.set_sphere_diameter(sphere_diameter);
        Renderer.set_fog(fog_dist, fog_range);
    } // ImGui windows end

    // Rendering
    SDL_GL_MakeCurrent(MainWindow, Gl_context);
    glViewport(
        0,
        0,
        static_cast<int>(io.DisplayFramebufferScale.x * io.DisplaySize.x),
        static_cast<int>(io.DisplayFramebufferScale.y * io.DisplaySize.y));
    glClearColor(State->get_color(Background).r_norm(),
                 State->get_color(Background).g_norm(),
                 State->get_color(Background).b_norm(),
                 State->get_color(Background).a_norm());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto separator_thickness(4.f);

    Timeline_region = Base_renderer::Region(
        static_cast<float>(Left_panel_size),
        static_cast<float>(Bottom_panel_size),
        static_cast<float>(width),
        static_cast<float>(timeline_height - 0.5f * separator_thickness));
    Scene_region = Base_renderer::Region(
        static_cast<float>(Left_panel_size),
        static_cast<float>(0.5f * separator_thickness + timeline_height),
        static_cast<float>(width),
        static_cast<float>(height));

    Renderer.set_redering_region(Scene_region,
                                 io.DisplayFramebufferScale.x,
                                 io.DisplayFramebufferScale.y);
    Timeline.set_redering_region(Timeline_region,
                                 io.DisplayFramebufferScale.x,
                                 io.DisplayFramebufferScale.y);

    Renderer.set_text_renderer(Text_ren);
    Timeline.set_text_renderer(Text_ren);

    Timeline.set_splitter(splitter);

    Timeline.set_pictogram_size(pictograms_size);
    Timeline.set_pictogram_magnification(pictogram_scale, pictogram_region);

    glm::mat4 proj_ortho = glm::ortho(0.f,
                                      static_cast<float>(width),
                                      0.f,
                                      static_cast<float>(height));
    glUseProgram(Screen_shad->program_id);
    glUniformMatrix4fv(Screen_shad->proj_mat_id,
                       1,
                       GL_FALSE,
                       glm::value_ptr(proj_ortho));
    Screen_shader::Screen_geometry separator;
    Screen_shader::Line_strip line;
    line.emplace_back(Screen_shader::Line_point(
        glm::vec2(Left_panel_size, timeline_height),
        4.f,
        glm::vec4(0.f, 0.f, 0.f, 0.15f)));
    line.emplace_back(Screen_shader::Line_point
    (glm::vec2(width, timeline_height),
        4.f,
        glm::vec4(0.f, 0.f, 0.f, 0.15f)));
    Screen_shad->append_to_geometry(separator, line);

    separator.init_buffers();
    glUseProgram(Screen_shad->program_id);
    Screen_shad->draw_geometry(separator);

    Text_ren->clear();

    glUseProgram(Diffuse_shad->program_id);
    Renderer.render();
    glUseProgram(Screen_shad->program_id);
    Timeline.render();

    Text_ren->render(width, height);
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(MainWindow);
}

#if __EMSCRIPTEN__
#include <cstdio>

extern "C"
{
//******************************************************************************
// js_load_ode
//
// This function is called from JavaScript to load user-defined ODE
//******************************************************************************
void js_load_ode()
{
    std::vector<std::string> fnames;
    
    for(size_t i = 0; i < 100; ++i)
    {
        std::string fn("user_ode" + std::to_string(i) + ".txt");
        std::ifstream f(fn);
        if(f.good())
            fnames.push_back(fn);
        else
            break;
    }

    Scene_objs.load_ode(fnames, Curve_max_deviation);

    // The file has to removed to be able to load a new file with the same name
    for(auto& fn: fnames)
        remove(fn.c_str());
}
}
#endif
} // namespace end

//******************************************************************************
// main
//******************************************************************************

int main(int, char**)
{
    // Setup SDL
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#elif __EMSCRIPTEN__
    // Nothing
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GetCurrentDisplayMode(0, &Current);
    Window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL |
                                     SDL_WINDOW_RESIZABLE |
                                     SDL_WINDOW_ALLOW_HIGHDPI);

    MainWindow = SDL_CreateWindow(
        "ManyLands",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        Window_flags);
    Gl_context = SDL_GL_CreateContext(MainWindow);
    SDL_GL_SetSwapInterval(1); // Enable vsync

#if defined(WIN32)
    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to
    // requires some form of initialization.
    bool err = false;
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }
#endif

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if(Enable_keyboard) io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    if(Enable_gamepad ) io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Disable saving the ini-file
    io.IniFilename = NULL;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(MainWindow, Gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Font
    ImGui::GetStyle().ScaleAllSizes(App_scale);
    ImGui::GetStyle().WindowRounding = 0.f;
    io.Fonts->AddFontFromFileTTF("assets/Roboto-Regular.ttf", 14.f * App_scale);

    set_bright_theme();

    Diffuse_shad->initialize();
    Screen_shad->initialize();

    Renderer.set_shaders(Diffuse_shad, Screen_shad);
    Timeline.set_shader(Screen_shad);

    State->camera_4D <<= 0., 0., 0., 550., 0.;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
#else

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);


    Last_timepoint = std::chrono::system_clock::now();
    // Main loop
    while(!done)
    {
        mainloop();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(Gl_context);
    SDL_DestroyWindow(MainWindow);
    SDL_Quit();
#endif

    return EXIT_SUCCESS;
}
