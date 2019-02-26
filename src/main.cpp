// dear imgui: standalone example application for SDL2 + OpenGL
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

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

// About OpenGL function loaders: modern OpenGL doesn't have a standard header file and requires individual function pointers to be loaded manually.
// Helper libraries are often used for this purpose! Here we are supporting a few common ones: gl3w, glew, glad.
// You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

int main(int, char**)
{
    // UI settings
    
    const bool enable_keyboard = false,   // Controllers
               enable_gamepad  = false,
               win_maximized   = true;
    const float app_scale = 1.f,          // Global app scale
                width_scale = 0.9f,       // Window size
                height_scale = 0.9f;
    // Panel sizes
    int left_panel_size         = 300 * app_scale;
    const int bottom_panel_size = 36 * app_scale;

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    auto Color_to_ImVec4 = [](Color c) {
        return ImVec4(c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f);
    };

    ImVec4 clear_color,
           x_axis_color,
           y_axis_color,
           z_axis_color,
           w_axis_color,
           low_speed_color,
           high_speed_color;

    auto set_bright_theme = [&]() {
        clear_color      = Color_to_ImVec4(Color(255, 255, 255));
        x_axis_color     = Color_to_ImVec4(Color(215,  25,  28));
        y_axis_color     = Color_to_ImVec4(Color(253, 174,  97));
        z_axis_color     = Color_to_ImVec4(Color(171, 217, 233));
        w_axis_color     = Color_to_ImVec4(Color( 44, 123, 182));
        low_speed_color  = Color_to_ImVec4(Color(  0,   0,   0));
        high_speed_color = Color_to_ImVec4(Color(220, 220, 220));
    };

    auto set_dark_theme = [&]() {
        clear_color      = Color_to_ImVec4(Color(  0,   0,   0));
        x_axis_color     = Color_to_ImVec4(Color(215,  25,  28));
        y_axis_color     = Color_to_ImVec4(Color(253, 174,  97));
        z_axis_color     = Color_to_ImVec4(Color(171, 217, 233));
        w_axis_color     = Color_to_ImVec4(Color( 44, 123, 182));
        low_speed_color  = Color_to_ImVec4(Color(255, 255, 255));
        high_speed_color = Color_to_ImVec4(Color( 35,  35,  35));
    };

    set_bright_theme();

    GLuint program_id = 
        load_shaders("shaders\\Diffuse.vert", "shaders\\Diffuse.frag");

    GLuint proj_mat_id   = glGetUniformLocation(program_id,   "projMatrix"),
           mv_mat_id     = glGetUniformLocation(program_id,     "mvMatrix"),
           normal_mat_id = glGetUniformLocation(program_id, "normalMatrix"),
           light_pos_id  = glGetUniformLocation(program_id,     "lightPos");

    std::shared_ptr<Scene_state>
        state = std::make_shared<Scene_state>();
    std::unique_ptr<Scene_renderer>
        renderer = std::make_unique<Scene_renderer>(state);
    std::unique_ptr<Scene>
        scene = std::make_unique<Scene>(state);

    //Controls::set_scene_state(state);

    state->camera_4D(0) =   0.;
    state->camera_4D(1) =   0.;
    state->camera_4D(2) =   0.;
    state->camera_4D(3) = 550.;
    state->camera_4D(4) =   0.;

    bool show_tesseract = true,
         show_curve = true;
    float fov_y = 45.f * DEG_TO_RAD;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        {
            static float tesseract_size[4] = { 200.f, 200.f, 200.f, 200.f },
                         tesseract_thickness = 3.f,
                         curve_thickness = 3.f,
                         sphere_diameter = 3.f,
                         camera_3D_dist = 3.f,
                         xy_rot = 0.f,
                         yz_rot = 0.f,
                         zx_rot = 0.f,
                         xw_rot = 0.f,
                         yw_rot = 0.f,
                         zw_rot = 0.f,
                         fov_4d[3] = { 30.f * DEG_TO_RAD,
                                       30.f * DEG_TO_RAD,
                                       30.f * DEG_TO_RAD };

            int width, height;
            //glfwGetWindowSize(window, &width, &height);
            width = (int)io.DisplaySize.x;
            height = (int)io.DisplaySize.y;
            ImGui::SetNextWindowSize(ImVec2(left_panel_size, height));
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSizeConstraints(
                ImVec2(0, height), ImVec2(FLT_MAX, height));

            ImGui::Begin(
                "Control panel",
                0,
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

            ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);

            if(ImGui::Button("Load ODE"))
            {
                std::string filename;
#ifdef _WIN32
                char fn[MAX_PATH];

                OPENFILENAMEA ofn;
                ZeroMemory( &fn, sizeof( fn ) );
                ZeroMemory( &ofn, sizeof( ofn ) );
                ofn.lStructSize  = sizeof(ofn);
                ofn.hwndOwner    = NULL; //glfwGetWin32Window(window);
                ofn.lpstrFilter  = "Text Files\0*.txt\0Any File\0*.*\0";
                ofn.lpstrFile    = fn;
                ofn.nMaxFile     = MAX_PATH;
                ofn.lpstrTitle   = "Select an ODE";
                ofn.Flags        = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

                GetOpenFileNameA(&ofn);

                filename = std::string(fn);
#else
                # Error: file open dialog has to implemented for this platform
#endif
                if(!filename.empty())
                    scene->load_ode(filename);
            }

            if (ImGui::CollapsingHeader("Rendering",
                ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text("Scene:");
                ImGui::Checkbox("Tesseract##visibility", &show_tesseract);
                ImGui::SameLine();
                ImGui::Checkbox("Curve", &show_curve);

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
                    "Curve", &curve_thickness, 0.1f, 10.0f);
                ImGui::SliderFloat(
                    "Sphere diameter", &sphere_diameter, 0.1f, 10.0f);

                ImGui::ColorEdit3("Background", (float*)&clear_color);
                ImGui::ColorEdit3("X-axis", (float*)&x_axis_color);
                ImGui::ColorEdit3("Y-axis", (float*)&y_axis_color);
                ImGui::ColorEdit3("Z-axis", (float*)&z_axis_color);
                ImGui::ColorEdit3("W-axis", (float*)&w_axis_color);

                ImGui::ColorEdit3(
                    "Curve slow", (float*)&low_speed_color
                );
                ImGui::ColorEdit3(
                    "Curve fast", (float*)&high_speed_color
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
                camera_3D_dist = std::clamp((float)(-state->camera_3D.z), dist_min, dist_max);
                ImGui::SliderFloat("Distance", &camera_3D_dist, dist_min, dist_max);
                ImGui::SliderAngle("Field of view", &fov_y, 1.f, 180.f);
               
                static float euler[3];
                static bool active = false;
                if(!active)
                    glm::extractEulerAngleXYZ(glm::toMat4(state->rotation_3D), euler[0], euler[1], euler[2]);
                
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

                state->rotation_3D = glm::eulerAngleXYZ(euler[0], euler[1], euler[2]);
            }

            left_panel_size = ImGui::GetWindowSize().x;
            ImGui::End();

            ImGui::SetNextWindowSize(ImVec2(width - left_panel_size, bottom_panel_size));
            ImGui::SetNextWindowPos(ImVec2(left_panel_size, height - bottom_panel_size));
            static float animation = 0.f;
            ImGui::Begin(
                "##animation",
                0,
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize);
            ImGui::PushItemWidth(-1);
            ImGui::SliderFloat("##label", &animation, 0.f, 1.f);
            ImGui::End();

            state->camera_3D.z = -camera_3D_dist;

            state->projection_4D = Matrix_lib::get4DProjectionMatrix(
                fov_4d[0], fov_4d[1], fov_4d[2], 1, 10);

            state->xy_rot = xy_rot;
            state->yz_rot = yz_rot;
            state->zx_rot = zx_rot;
            state->xw_rot = xw_rot;
            state->yw_rot = yw_rot;
            state->zw_rot = zw_rot;

            // Helper lambda expression
            auto ImVec4_to_Color = [](ImVec4 v) {
                return Color(v.x * 255, v.y * 255, v.z * 255, v.w * 255);
            };

            state->update_color(Background,       ImVec4_to_Color(clear_color));
            state->update_color(X_axis,           ImVec4_to_Color(x_axis_color));
            state->update_color(Y_axis,           ImVec4_to_Color(y_axis_color));
            state->update_color(Z_axis,           ImVec4_to_Color(z_axis_color));
            state->update_color(W_axis,           ImVec4_to_Color(w_axis_color));
            state->update_color(Curve_low_speed,  ImVec4_to_Color(low_speed_color));
            state->update_color(Curve_high_speed, ImVec4_to_Color(high_speed_color));

            std::copy(std::begin(
                tesseract_size),
                std::end(tesseract_size),
                std::begin(state->tesseract_size));
            state->unfolding_anim_ = animation;
            state->show_tesseract  = show_tesseract;
            state->show_curve      = show_curve;

            renderer->set_line_thickness(tesseract_thickness, curve_thickness);
            renderer->set_sphere_diameter(sphere_diameter);
        }

        // Rendering
        ImGui::Render();
        SDL_GL_MakeCurrent(window, gl_context);
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(state->get_color(Background)->r / 255.f,
                     state->get_color(Background)->g / 255.f,
                     state->get_color(Background)->b / 255.f,
                     state->get_color(Background)->a / 255.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program_id);
        /*if(!io.WantCaptureMouse)
            Controls::update(window);*/
        int width, height;
        //glfwGetWindowSize(window, &width, &height);
        width = (int)io.DisplaySize.x;
        height = (int)io.DisplaySize.y;

        glm::ivec2 scene_pos(left_panel_size, bottom_panel_size),
                   scene_size(width - left_panel_size,
                              height - bottom_panel_size);

		glm::mat4 proj_mat = glm::perspective(
            fov_y,
            (float)scene_size[0] / scene_size[1],
            0.1f,
            100.f);
		
        auto camera_mat = glm::translate(glm::mat4(1.f), state->camera_3D);

        glm::vec3 light_pos(0.f, 0.f, 70.f);

		auto world_mat = glm::toMat4(state->rotation_3D);
        auto norm_mat = glm::transpose(glm::inverse(glm::mat3(world_mat)));

        glUniformMatrix4fv(proj_mat_id, 1, GL_FALSE, glm::value_ptr(proj_mat));
        glUniformMatrix4fv(
            mv_mat_id, 1, GL_FALSE, glm::value_ptr(camera_mat * world_mat));
        glUniformMatrix3fv(
            normal_mat_id, 1, GL_FALSE, glm::value_ptr(norm_mat));
        glUniform3fv(light_pos_id, 1, glm::value_ptr(light_pos));



        
        glViewport(scene_pos[0], scene_pos[1], scene_size[0], scene_size[1]);
        renderer->render();
        glViewport(0, 0, width, height);




        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
