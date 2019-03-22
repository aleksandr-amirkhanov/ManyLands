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
#include <stdio.h>
#include <memory.h>
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

bool Show_tesseract = true,
     Show_curve     = true;

ImVec4 Clear_color,
       X_axis_color,
       Y_axis_color,
       Z_axis_color,
       W_axis_color,
       Low_speed_color,
       High_speed_color;

const bool Enable_keyboard = false,   // Controllers
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

Scene Scene_objs(State);

Base_renderer::Region Scene_region, Timeline_region;
Base_renderer::Renderer_io Previous_io;

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
    io.mouse_wheel_y = static_cast<float>(event.wheel.y);

    Renderer.process_input(io);
    Timeline.process_input(io);

    Previous_io = io;
}

//******************************************************************************
// mainloop
//******************************************************************************

void mainloop()
{
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

    static float timeline_height = 200.f,
                 pictograms_size = 50.f,
                 pictogram_scale = 1.5f,
                 splitter        = 0.5f;
    
    static int pictogram_region = 4;

    // ImGui windows start
    {
        static float tesseract_size[4] = { 200.f, 200.f, 200.f, 200.f },
                     tesseract_thickness = 3.f,
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

        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(Left_panel_size),
                                        static_cast<float>(height)));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(0.f, static_cast<float>(height)),
            ImVec2(FLT_MAX, static_cast<float>(height)));

        ImGui::Begin(
            "Control panel",
            0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        ImGui::Text("%.1f FPS", io.Framerate);

        if(ImGui::Button("Load ODE"))
        {
#if __EMSCRIPTEN__
            EM_ASM(
                var input = document.createElement('input');
                input.type = 'file';
                input.onchange = e => {
                    var file = e.target.files[0];
                    var textType = /text.*/;
                    if (file.type.match(textType)) {
                        var reader = new FileReader();
                        reader.onload = function () {
                            Module['FS_createDataFile'](
                                '/',
                                'user_ode.txt',
                                reader.result,
                                true,
                                true);

                            var js_load_ode = Module.cwrap(
                                'js_load_ode',
                                'null',
                                ['null']);
                            js_load_ode();
                        };
                        reader.readAsText(file);
                    }
                };
                input.click();
            );
#else
#ifdef _WIN32
            std::string filename;
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
            ofn.Flags        = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

            GetOpenFileNameA(&ofn);

            filename = std::string(fn);
#else
            const std::string filename = "assets/model2-default.txt";
#endif
            if(!filename.empty())
                Scene_objs.load_ode(filename);
#endif
        }

        if (ImGui::CollapsingHeader("Rendering",
            ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Scene:");
            ImGui::Checkbox("Tesseract##visibility", &Show_tesseract);
            ImGui::SameLine();
            ImGui::Checkbox("Curve##visibility", &Show_curve);

            ImGui::SliderFloat4(
                "Tesseract size", tesseract_size, 1.f, 500.f);

            ImGui::Separator();

            ImGui::Text("Style:");
            ImGui::Text("Set theme");
            ImGui::SameLine();
            if(ImGui::Button("Bright"))
                set_bright_theme();
            ImGui::SameLine();
            if(ImGui::Button("Dark"))
                set_dark_theme();
            ImGui::SliderFloat(
                "Tesseract##thickness", &tesseract_thickness, 0.1f, 10.0f);
            ImGui::SliderFloat(
                "Curve##thickness", &curve_thickness, 0.1f, 10.0f);
            ImGui::SliderFloat(
                "Sphere diameter", &sphere_diameter, 0.1f, 10.0f);
            ImGui::SliderFloat(
                "Splitter", &splitter, 0.05f, 0.95f);
            ImGui::SliderFloat(
                "Timeline", &timeline_height, 10.f, 1000.0f);
            ImGui::SliderFloat(
                "Pictograms", &pictograms_size, 10.f, 100.0f);

            ImGui::Separator();
            ImGui::Text("Pictogram magnification");
            ImGui::SliderFloat(
                "Scale", &pictogram_scale, 0.5f, 10.f);
            ImGui::SliderInt(
                "Region", &pictogram_region, 1, 10);

            ImGui::Separator();

            ImGui::Text("Colors:");
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

            ImGui::SliderFloat("Fog distance", &fog_dist,  0.1f, 10.f);
            ImGui::SliderFloat("Fog range",    &fog_range, 0.1f, 10.f);
        }

        if (ImGui::CollapsingHeader("4D projection",
            ImGuiTreeNodeFlags_DefaultOpen))
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

        if (ImGui::CollapsingHeader("Cameras",
            ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Basics:");
            const float dist_min = 0.5f, dist_max = 5.f;
            camera_3D_dist = std::clamp((float)(-State->camera_3D.z),
                                        dist_min,
                                        dist_max);
            ImGui::SliderFloat("Distance", &camera_3D_dist, dist_min, dist_max);
            ImGui::SliderAngle("Field of view", &fov_y, 1.f, 180.f);

            static float euler[3];
            static bool active = false;
            if(!active)
                glm::extractEulerAngleXYZ(glm::toMat4(State->rotation_3D),
                                          euler[0],
                                          euler[1],
                                          euler[2]);

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

            State->rotation_3D = glm::eulerAngleXYZ(euler[0],
                                                    euler[1],
                                                    euler[2]);

            State->fov_y = fov_y;
        }

        Left_panel_size = static_cast<int>(ImGui::GetWindowSize().x);
        ImGui::End();

        ImGui::SetNextWindowSize(
            ImVec2(static_cast<float>(width - Left_panel_size),
                   static_cast<float>(Bottom_panel_size)));
        ImGui::SetNextWindowPos(
            ImVec2(static_cast<float>(Left_panel_size),
                   static_cast<float>(height - Bottom_panel_size)));
        static float animation = 0.f;
        ImGui::Begin(
            "##animation",
            0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize);
        ImGui::PushItemWidth(-1);
        ImGui::SliderFloat("##label", &animation, 0.f, 1.f);
        ImGui::End();

        State->camera_3D.z = -camera_3D_dist;

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

        std::copy(std::begin(
                  tesseract_size),
                  std::end(tesseract_size),
                  std::begin(State->tesseract_size));
        State->unfolding_anim  = animation;
        State->show_tesseract  = Show_tesseract;
        State->show_curve      = Show_curve;

        Renderer.set_line_thickness(tesseract_thickness, curve_thickness);
        Renderer.set_sphere_diameter(sphere_diameter);
        Renderer.set_fog(fog_dist, fog_range);
    } // ImGui windows end

    // Rendering
    ImGui::Render();
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


    Timeline_region = Base_renderer::Region(
        static_cast<float>(Left_panel_size),
        static_cast<float>(Bottom_panel_size),
        static_cast<float>(width),
        static_cast<float>(timeline_height));
    Scene_region = Base_renderer::Region(
        static_cast<float>(Left_panel_size),
        static_cast<float>(Bottom_panel_size + timeline_height),
        static_cast<float>(width),
        static_cast<float>(height));

    Renderer.set_redering_region(Scene_region,
                                 io.DisplayFramebufferScale.x,
                                 io.DisplayFramebufferScale.y);
    Timeline.set_redering_region(Timeline_region,
                                 io.DisplayFramebufferScale.x,
                                 io.DisplayFramebufferScale.y);

    Timeline.set_splitter(splitter);

    Timeline.set_pictogram_size(pictograms_size);
    Timeline.set_pictogram_magnification(pictogram_scale, pictogram_region);

    glm::mat4 proj_ortho = glm::ortho(0.f,
                                      static_cast<float>(width),
                                      0.f,
                                      static_cast<float>(height));
    glUniformMatrix4fv(Screen_shad->proj_mat_id,
                       1,
                       GL_FALSE,
                       glm::value_ptr(proj_ortho));
    Screen_shader::Screen_geometry separator;
    Screen_shader::Line_strip line;
    line.emplace_back(Screen_shader::Line_point(glm::vec2(Left_panel_size, timeline_height), 4.f, glm::vec4(0.f, 0.f, 0.f, 0.15f)));
    line.emplace_back(Screen_shader::Line_point(glm::vec2(width,           timeline_height), 4.f, glm::vec4(0.f, 0.f, 0.f, 0.15f)));
    Screen_shad->append_to_geometry(separator, line);


    separator.init_buffers();
    Screen_shad->draw_geometry(separator);

    Renderer.render();
    Timeline.render();

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
    const std::string filename = "user_ode.txt";
    Scene_objs.load_ode(filename);

    // The file has to removed to be able to load a new file with the same name
    remove(filename.c_str());
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
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
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
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
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
