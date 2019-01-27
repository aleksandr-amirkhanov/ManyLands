#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#endif

// imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
// std
#include <stdio.h>
#include <memory.h>
// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
// Local
#include "Mesh_generator.h"
#include "Geometry_engine.h"
#include "Shader.h"
#include "Scene.h"
#include "Scene_state.h"
#include "Scene_renderer.h"
#include "Consts.h"
#include "Controls.h"
#include "Matrix_lib.h"
#include "Tesseract.h"
// OpenGL
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#include <GLFW/glfw3native.h>
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char**)
{
    const bool enable_keyboard = false;
    const bool enable_gamepad = false;
    int panel_size = 300;

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    auto monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_SAMPLES, 16);
    GLFWwindow* window = glfwCreateWindow(
        mode->width * 0.9,
        mode->height * 0.9,
        "ManyLands",
        NULL,
        NULL);
    glfwSetWindowPos(window, mode->width * 0.05, mode->height * 0.05);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glfwSetMouseButtonCallback(window, Controls::mouse_button_callback);
    glfwSetScrollCallback(window, Controls::scroll_callback);

    // Initialize OpenGL loader
    bool err = gl3wInit() != 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if(enable_keyboard)
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    if(enable_gamepad)
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup Style
    ImGui::StyleColorsDark();

    // Font
    const float app_scale = 1.f;
    ImGui::GetStyle().ScaleAllSizes(app_scale);
    ImGui::GetStyle().WindowRounding = 0.f;
    io.Fonts->AddFontFromFileTTF("fonts\\Roboto-Regular.ttf", 14.f * app_scale);

    bool show_tesseract = true;
    bool show_curve = false;

    auto Color_to_ImVec4 = [](Color c)
    {
        return ImVec4(c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f);
    };

    ImVec4 clear_color = ImVec4(1.f, 1.f, 1.f, 1.f);
    ImVec4 x_axis_color = Color_to_ImVec4(Color(215, 25, 28));
    ImVec4 y_axis_color = Color_to_ImVec4(Color(253, 174, 97));
    ImVec4 z_axis_color = Color_to_ImVec4(Color(171, 217, 233));
    ImVec4 w_axis_color = Color_to_ImVec4(Color(44, 123, 182));

    GLuint programID = 
        LoadShaders("shaders\\Diffuse.vert", "shaders\\Diffuse.frag");
    GLuint proj_mat_id = glGetUniformLocation(programID, "projMatrix");
    GLuint mv_mat_id = glGetUniformLocation(programID, "mvMatrix");
    GLuint normal_mat_id = glGetUniformLocation(programID, "normalMatrix");
    GLuint light_pos_id = glGetUniformLocation(programID, "lightPos");

    int vertex_pos = glGetAttribLocation(programID, "vertex");
    int normal_pos = glGetAttribLocation(programID, "normal");
    int color_pos = glGetAttribLocation(programID, "color");

    glm::vec3 pos(0.f, 0.f, 0.f);
    glm::vec4 col(1, 0.f, 0.f, 1.f);
    auto m = Mesh_generator::sphere(8, 8, 0.5f, pos, col);
    float fov_y = 45.f * DEG_TO_RAD;

    std::shared_ptr<Scene_state> state =
        std::make_shared<Scene_state>();
    std::unique_ptr<Scene_renderer> renderer =
        std::make_unique<Scene_renderer>(state);
    std::unique_ptr<Scene> scene =
        std::make_unique<Scene>(state);

    Controls::set_scene_state(state);

    state->camera_4D(0) = 0;
    state->camera_4D(1) = 0;
    state->camera_4D(2) = 0;
    state->camera_4D(3) = 550.;
    state->camera_4D(4) = 0;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float tesseract_size[4] = { 200.f, 200.f, 200.f, 200.f };
            static float line_thickness = 3.f;
            static float sphere_diameter = 3.f;
            
            static float camera_3D_dist = 3.f;

            static float fov_4d[3] = {
                30.f * DEG_TO_RAD,
                30.f * DEG_TO_RAD,
                30.f * DEG_TO_RAD };

            static float xy_rot = 0.f;
            static float yz_rot = 0.f;
            static float zx_rot = 0.f;
            static float xw_rot = 0.f;
            static float yw_rot = 0.f;
            static float zw_rot = 0.f;

            int width, height;
            glfwGetWindowSize(window, &width, &height);
            ImGui::SetNextWindowSize(ImVec2(panel_size, height));
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSizeConstraints(
                ImVec2(0, height), ImVec2(FLT_MAX, height));

            ImGui::Begin(
                "Control panel",
                0,
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

            if(ImGui::Button("Load ODE"))
            {
                std::string filename;
#ifdef _WIN32
                char fn[MAX_PATH];

                OPENFILENAMEA ofn;
                ZeroMemory( &fn, sizeof( fn ) );
                ZeroMemory( &ofn, sizeof( ofn ) );
                ofn.lStructSize  = sizeof(ofn);
                ofn.hwndOwner    = glfwGetWin32Window(window);
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
                ImGui::Checkbox("show tesseract", &show_tesseract);
                ImGui::SameLine();
                ImGui::Checkbox("show curve", &show_curve);
                ImGui::SliderFloat4(
                    "tesseract size", tesseract_size, 1.f, 500.f);
                ImGui::SliderFloat(
                    "line thickness", &line_thickness, 0.1f, 10.0f);
                ImGui::SliderFloat(
                    "sphere diameter", &sphere_diameter, 0.1f, 10.0f);
                ImGui::ColorEdit3("background", (float*)&clear_color);
                ImGui::ColorEdit3("x-axis color", (float*)&x_axis_color);
                ImGui::ColorEdit3("y-axis color", (float*)&y_axis_color);
                ImGui::ColorEdit3("z-axis color", (float*)&z_axis_color);
                ImGui::ColorEdit3("w-axis color", (float*)&w_axis_color);
            }

            if (ImGui::CollapsingHeader("4D projection",
                ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::SliderAngle("4D FOV (height)", &fov_4d[0], 1.f, 180.f);
                ImGui::SliderAngle("4D FOV (width)", &fov_4d[1], 1.f, 180.f);
                ImGui::SliderAngle("4D FOV (depth)", &fov_4d[2], 1.f, 180.f);

                if(ImGui::Button("Reset##4D"))
                {
                    xy_rot = yz_rot = zx_rot = xw_rot = yw_rot = zw_rot = 0.f;
                }
                ImGui::SliderAngle("XY-plane", &xy_rot, -180.f, 180.f);
                ImGui::SliderAngle("YZ-plane", &yz_rot, -180.f, 180.f);
                ImGui::SliderAngle("ZX-plane", &zx_rot, -180.f, 180.f);
                ImGui::SliderAngle("XW-plane", &xw_rot, -180.f, 180.f);
                ImGui::SliderAngle("YW-plane", &yw_rot, -180.f, 180.f);
                ImGui::SliderAngle("ZW-plane", &zw_rot, -180.f, 180.f);
            }

            if (ImGui::CollapsingHeader("Cameras",
                ImGuiTreeNodeFlags_DefaultOpen))
            {
                const float dist_min = 1.f, dist_max = 20.f;
                camera_3D_dist = std::clamp((float)(-state->camera_3D.z), dist_min, dist_max);
                ImGui::SliderFloat("3D camera distance", &camera_3D_dist, dist_min, dist_max);
                ImGui::SliderAngle("3D FOV", &fov_y, 1.f, 180.f);
               
                static float euler[3];
                static bool active = false;
                if(!active)
                    glm::extractEulerAngleXYZ(glm::toMat4(state->rotation_3D), euler[0], euler[1], euler[2]);
                
                active = false;
                if(ImGui::Button("Reset##3D"))
                {
                    for(int i = 0; i < 3; ++i)
                        euler[i] = 0.f;
                }
                ImGui::SliderAngle("euler x", &euler[0], -180.f, 180.f);
                active |= ImGui::IsItemActive();
                ImGui::SliderAngle("euler y", &euler[1], -180.f, 180.f);
                active |= ImGui::IsItemActive();
                ImGui::SliderAngle("euler z", &euler[2], -180.f, 180.f);
                active |= ImGui::IsItemActive();

                state->rotation_3D = glm::eulerAngleXYZ(euler[0], euler[1], euler[2]);
            }

            panel_size = ImGui::GetWindowSize().x;
            ImGui::End();

            ImGui::SetNextWindowSize(ImVec2(width - panel_size, 40));
            ImGui::SetNextWindowPos(ImVec2(panel_size, height - 40));
            float val;
            ImGui::Begin(
                "##animation",
                0,
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize);
            ImGui::PushItemWidth(-1);
            ImGui::SliderFloat("##label", &val, 0.f, 1.f);
            ImGui::End();

            state->camera_3D.z = -camera_3D_dist;

            state->projection_4D = Matrix_lib::get4DProjectionMatrix(
                fov_4d[0], fov_4d[1], fov_4d[2], 1, 10);

            state->rotation_4D = Matrix_lib::getXYRotationMatrix(xy_rot);
            state->rotation_4D = prod(
                state->rotation_4D,
                Matrix_lib::getYZRotationMatrix(yz_rot));
            state->rotation_4D = prod(
                state->rotation_4D,
                Matrix_lib::getZXRotationMatrix(zx_rot));
            state->rotation_4D = prod(
                state->rotation_4D,
                Matrix_lib::getXWRotationMatrix(xw_rot));
            state->rotation_4D = prod(
                state->rotation_4D,
                Matrix_lib::getYWRotationMatrix(yw_rot));
            state->rotation_4D = prod(
                state->rotation_4D,
                Matrix_lib::getZWRotationMatrix(zw_rot));

            auto ImVec4_to_Color = [](ImVec4 v) {
                return Color(v.x * 255, v.y * 255, v.z * 255, v.w * 255);
            };

            state->clear_color = ImVec4_to_Color(clear_color);
            state->x_axis_color = ImVec4_to_Color(x_axis_color);
            state->y_axis_color = ImVec4_to_Color(y_axis_color);
            state->z_axis_color = ImVec4_to_Color(z_axis_color);
            state->w_axis_color = ImVec4_to_Color(w_axis_color);

            renderer->set_line_thickness(line_thickness);
            renderer->set_sphere_diameter(sphere_diameter);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor( state->clear_color.r / 255.f,
                      state->clear_color.g / 255.f,
                      state->clear_color.b / 255.f,
                      state->clear_color.a / 255.f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(programID);
        if(!io.WantCaptureMouse)
            Controls::update(window);
        int width, height;
        glfwGetWindowSize(window, &width, &height);

		glm::mat4 proj_mat = glm::perspective(
            fov_y,
            (float)(width - panel_size) / height,
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



        
        glViewport(panel_size, 0, width - panel_size, height);
        renderer->render();
        glViewport(0, 0, width, height);




        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
