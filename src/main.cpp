#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#endif

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
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
#include "Shader.h"
#include "Scene.h"
#include "Scene_state.h"
#include "Scene_renderer.h"
#include "Consts.h"
//#include "Controls.h"
#include "Matrix_lib.h"
#include "Tesseract.h"

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
SDL_Window* Window;
SDL_GLContext Gl_context;

bool Show_tesseract = true,
     Show_curve     = true;

ImVec4 Clear_color,
       X_axis_color,
       Y_axis_color,
       Z_axis_color,
       W_axis_color,
       Low_speed_color,
       High_speed_color;

GLuint Program_id,
       Proj_mat_id,
       Mv_mat_id,
       Normal_mat_id,
       Light_pos_id;

const bool Enable_keyboard = false,   // Controllers
           Enable_gamepad  = false;

const float App_scale = 1.f;          // Global app scale

const int Bottom_panel_size = 36 * App_scale;
int Left_panel_size = 300 * App_scale;

std::shared_ptr<Scene_state> State;
std::unique_ptr<Scene_renderer> Renderer;
std::unique_ptr<Scene> Scene_objs;

//******************************************************************************
// Color_to_ImVec4
//******************************************************************************

ImVec4 Color_to_ImVec4(Color c)
{
    return ImVec4(c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f);
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
            && event.window.windowID == SDL_GetWindowID(Window))
        {
            done = true;
        }

        if(!io.WantCaptureMouse)
        {
            if(   event.type == SDL_MOUSEMOTION
               && event.button.button == SDL_BUTTON_LEFT)
            {
                //event.motion.xrel
                glm::vec3 axis(event.motion.yrel, event.motion.xrel, 0.f);
                float length = glm::length(axis);
                State->rotation_3D =
                    glm::angleAxis(
                        glm::radians(0.25f * length),
                        glm::normalize(axis)) *
                    State->rotation_3D;
            }

            if(event.type == SDL_MOUSEWHEEL)
            {
                State->camera_3D.z += event.wheel.y * 0.3f;
            }
        }
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(Window);
    ImGui::NewFrame();

    static float fov_y = 45.f * DEG_TO_RAD;

    int width = (int)io.DisplaySize.x;
    int height = (int)io.DisplaySize.y;

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
                     fov_4d[3] = { 30.f * DEG_TO_RAD,
                                   30.f * DEG_TO_RAD,
                                   30.f * DEG_TO_RAD };

        ImGui::SetNextWindowSize(ImVec2(Left_panel_size, height));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(0, height), ImVec2(FLT_MAX, height));

        ImGui::Begin(
            "Control panel",
            0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        ImGui::Text("%.1f FPS", io.Framerate);

        if(ImGui::Button("Load ODE"))
        {
            std::string filename;
#ifdef _WIN32
            char fn[MAX_PATH];

            SDL_SysWMinfo wm_info;
            SDL_VERSION(&wm_info.version);
            SDL_GetWindowWMInfo(Window, &wm_info);

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
            filename = "assets/model1-default.txt";
#endif
            if(!filename.empty())
                Scene_objs->load_ode(filename);
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
            const float dist_min = 1.f, dist_max = 20.f;
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
        }

        Left_panel_size = ImGui::GetWindowSize().x;
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(width - Left_panel_size,
                                 Bottom_panel_size));
        ImGui::SetNextWindowPos(ImVec2(Left_panel_size,
                                height - Bottom_panel_size));
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

        State->projection_4D = Matrix_lib::get4DProjectionMatrix(
            fov_4d[0], fov_4d[1], fov_4d[2], 1, 10);

        State->xy_rot = xy_rot;
        State->yz_rot = yz_rot;
        State->zx_rot = zx_rot;
        State->xw_rot = xw_rot;
        State->yw_rot = yw_rot;
        State->zw_rot = zw_rot;

        // Helper lambda expression
        auto ImVec4_to_Color = [](ImVec4 v) {
            return Color(v.x * 255, v.y * 255, v.z * 255, v.w * 255);
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
        State->unfolding_anim_ = animation;
        State->show_tesseract  = Show_tesseract;
        State->show_curve      = Show_curve;

        Renderer->set_line_thickness(tesseract_thickness, curve_thickness);
        Renderer->set_sphere_diameter(sphere_diameter);
    } // ImGui windows end

    // Rendering
    ImGui::Render();
    SDL_GL_MakeCurrent(Window, Gl_context);
    glViewport(
        0,
        0,
        static_cast<int>(io.DisplayFramebufferScale.x * io.DisplaySize.x),
        static_cast<int>(io.DisplayFramebufferScale.y * io.DisplaySize.y));
    glClearColor(State->get_color(Background)->r / 255.f,
                 State->get_color(Background)->g / 255.f,
                 State->get_color(Background)->b / 255.f,
                 State->get_color(Background)->a / 255.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(Program_id);

    glm::ivec2 scene_pos(Left_panel_size, Bottom_panel_size),
               scene_size(width - Left_panel_size,
                          height - Bottom_panel_size);

	glm::mat4 proj_mat = glm::perspective(
        fov_y,
        (float)scene_size[0] / scene_size[1],
        0.1f,
        100.f);
		
    auto camera_mat = glm::translate(glm::mat4(1.f), State->camera_3D);

    glm::vec3 light_pos(0.f, 0.f, 70.f);

	auto world_mat = glm::toMat4(State->rotation_3D);
    auto norm_mat = glm::transpose(glm::inverse(glm::mat3(world_mat)));

    glUniformMatrix4fv(Proj_mat_id, 1, GL_FALSE, glm::value_ptr(proj_mat));
    glUniformMatrix4fv(
        Mv_mat_id, 1, GL_FALSE, glm::value_ptr(camera_mat * world_mat));
    glUniformMatrix3fv(
        Normal_mat_id, 1, GL_FALSE, glm::value_ptr(norm_mat));
    glUniform3fv(Light_pos_id, 1, glm::value_ptr(light_pos));
        
    glViewport(io.DisplayFramebufferScale.x * scene_pos[0],
               io.DisplayFramebufferScale.y * scene_pos[1],
               io.DisplayFramebufferScale.x * scene_size[0],
               io.DisplayFramebufferScale.y * scene_size[1]);
    Renderer->render();
    glViewport(0,
               0,
               io.DisplayFramebufferScale.x * width,
               io.DisplayFramebufferScale.y * height);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(Window);
}
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
    Window = SDL_CreateWindow(
        "Dear ImGui SDL2+OpenGL3 example",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        Window_flags);
    Gl_context = SDL_GL_CreateContext(Window);
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
    if(Enable_gamepad) io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(Window, Gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    //ImGui_ImplOpenGL3_Init(NULL);

    // Font
    ImGui::GetStyle().ScaleAllSizes(App_scale);
    ImGui::GetStyle().WindowRounding = 0.f;
    io.Fonts->AddFontFromFileTTF("assets/Roboto-Regular.ttf", 14.f * App_scale); 

    set_bright_theme();

#ifdef __EMSCRIPTEN__
    Program_id = load_shaders("assets/Diffuse_ES.vert",
                              "assets/Diffuse_ES.frag");
#else
    Program_id = load_shaders("assets/Diffuse.vert",
                              "assets/Diffuse.frag");
#endif

    Proj_mat_id   = glGetUniformLocation(Program_id,   "projMatrix");
    Mv_mat_id     = glGetUniformLocation(Program_id,     "mvMatrix");
    Normal_mat_id = glGetUniformLocation(Program_id, "normalMatrix");
    Light_pos_id  = glGetUniformLocation(Program_id,     "lightPos");
    
    State      = std::make_shared<Scene_state>();
    Renderer   = std::make_unique<Scene_renderer>(State);
    Scene_objs = std::make_unique<Scene>(State);

    State->camera_4D(0) =   0.;
    State->camera_4D(1) =   0.;
    State->camera_4D(2) =   0.;
    State->camera_4D(3) = 550.;
    State->camera_4D(4) =   0.;
	
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
    SDL_DestroyWindow(Window);
    SDL_Quit();
#endif

    return EXIT_SUCCESS;
}
