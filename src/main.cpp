// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

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
// Local
#include "Mesh_generator.h"
#include "Geometry_engine.h"
#include "Shader.h"
#include "Consts.h"
#include "Controls.h"
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

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h> 

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma. 
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void project_to_3D(
    boost::numeric::ublas::vector<double>& point,
    const boost::numeric::ublas::matrix<double>& rot_mat,
    const boost::numeric::ublas::matrix<double>& proj_mat,
    const boost::numeric::ublas::vector<double>& camera)
{
    boost::numeric::ublas::vector<double> tmp_vert = prod(point, rot_mat);
    tmp_vert = tmp_vert - camera;
    tmp_vert = prod(tmp_vert, proj_mat);

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

void project_to_3D(
    std::vector<boost::numeric::ublas::vector<double>>& verts,
    const boost::numeric::ublas::matrix<double>& rot_mat,
    const boost::numeric::ublas::matrix<double>& proj_mat,
    const boost::numeric::ublas::vector<double>& camera)
{
    for(auto& v : verts)
        project_to_3D(v, rot_mat, proj_mat, camera);
}

void draw_tesseract(Wireframe_object& t, std::vector<std::unique_ptr<Geometry_engine>>& geoms)
{
    //const double tesseract_thickness = gui_.tesseractThickness->value();
    //const double sphere_size = gui_.sphereSize->value();
    const float tesseract_thickness = 3;
    const float sphere_size = 3;

    for(auto const& e : t.get_edges())
    {
        Mesh t_mesh;
        auto& current = t.get_vertices().at(e->vert1);
        auto& next = t.get_vertices().at(e->vert2);
        //QColor col(e->color.r, e->color.g, e->color.b);
        glm::vec4 col((float)e->color.r / 255, (float)e->color.g / 255, (float)e->color.b / 255, 1.f);

        Mesh_generator::cylinder(
            5,
            tesseract_thickness / current(3),
            tesseract_thickness / next(3),
            glm::vec3(current(0), current(1), current(2)),
            glm::vec3(next(0), next(1), next(2)),
            col,
            t_mesh);

       geoms.push_back(std::make_unique<Geometry_engine>(t_mesh));
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
                size_coef * sphere_size / v(3),
                pos,
                glm::vec4(1.f, 0.f, 0.f, 1.f),
                s_mesh);
        else
            Mesh_generator::sphere(
                16,
                16,
                size_coef * sphere_size / v(3),
                pos,
                glm::vec4(0.59f, 0.59f, 0.59f, 1.f),
                s_mesh);

        geoms.push_back(std::make_unique<Geometry_engine>(s_mesh));
    }
}

int main(int, char**)
{
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
    GLFWwindow* window = glfwCreateWindow(mode->width * 0.9, mode->height * 0.9, "ManyLands", NULL, NULL);
    glfwSetWindowPos(window, mode->width * 0.05, mode->height * 0.05);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glfwSetMouseButtonCallback(window, Controls::mouse_button_callback);

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
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup Style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    const float app_scale = 1.25f;
    ImGui::GetStyle().ScaleAllSizes(app_scale);
    io.Fonts->AddFontFromFileTTF("fonts\\Roboto-Medium.ttf", 16.f * app_scale);

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

    bool show_tesseract = false;
    bool show_curve = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    GLuint programID = LoadShaders("shaders\\Diffuse.vert", "shaders\\Diffuse.frag");
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
    auto geom = Geometry_engine(m);
    float fov_y = 45.f;

    boost::numeric::ublas::matrix<double> proj_m_;
    boost::numeric::ublas::matrix<double> rot_m_;
    boost::numeric::ublas::vector<double> cam_4D_(5);

    cam_4D_(0) = 0;
    cam_4D_(1) = 0;
    cam_4D_(2) = 0;
    cam_4D_(3) = 550.;
    cam_4D_(4) = 0;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 3.0f;

            static float fov_4d_h = 0.5f;
            static float fov_4d_w = 0.5f;
            static float fov_4d_d = 0.5f;


            static float xy_rot = 0.f;
            static float yz_rot = 0.f;
            static float zx_rot = 0.f;
            static float xw_rot = 0.f;
            static float yw_rot = 0.f;
            static float zw_rot = 0.f;

            ImGui::Begin("Settings");

            ImGui::Text("Rendering");
            ImGui::Checkbox("show tesseract", &show_tesseract);
            ImGui::SameLine();
            ImGui::Checkbox("show curve", &show_curve);
            ImGui::SliderFloat("line thickness", &f, 0.1f, 10.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            ImGui::SliderFloat("field of view", &fov_y, 0.001f, 180.f);


            ImGui::Text("Cameras");
            ImGui::SliderFloat("4D FOV (h)", &fov_4d_h, 0.001f, 2.0f);
            ImGui::SliderFloat("4D FOV (w)", &fov_4d_w, 0.001f, 2.0f);
            ImGui::SliderFloat("4D FOV (d)", &fov_4d_d, 0.001f, 2.0f);

            ImGui::Text("4D rotations");
            ImGui::SliderFloat("XY-plane", &xy_rot, -180.f, 180.f);
            ImGui::SliderFloat("YZ-plane", &yz_rot, -180.f, 180.f);
            ImGui::SliderFloat("ZX-plane", &zx_rot, -180.f, 180.f);
            ImGui::SliderFloat("XW-plane", &xw_rot, -180.f, 180.f);
            ImGui::SliderFloat("YW-plane", &yw_rot, -180.f, 180.f);
            ImGui::SliderFloat("ZW-plane", &zw_rot, -180.f, 180.f);

            proj_m_ = Matrix_lib::get4DProjectionMatrix(fov_4d_h, fov_4d_w, fov_4d_d, 1, 10);
            rot_m_ = Matrix_lib::getXYRotationMatrix(xy_rot * DEG_TO_RAD);
            rot_m_ = prod(rot_m_, Matrix_lib::getYZRotationMatrix(yz_rot * DEG_TO_RAD));
            rot_m_ = prod(rot_m_, Matrix_lib::getZXRotationMatrix(zx_rot * DEG_TO_RAD));
            rot_m_ = prod(rot_m_, Matrix_lib::getXWRotationMatrix(xw_rot * DEG_TO_RAD));
            rot_m_ = prod(rot_m_, Matrix_lib::getYWRotationMatrix(yw_rot * DEG_TO_RAD));
            rot_m_ = prod(rot_m_, Matrix_lib::getZWRotationMatrix(zw_rot * DEG_TO_RAD));

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        boost::numeric::ublas::vector<double> origin(4);
        boost::numeric::ublas::vector<double> size(4);
        origin(0) = -100;
        origin(1) = -100;
        origin(2) = -100;
        origin(3) = -100;
        size(0) = 200;
        size(1) = 200;
        size(2) = 200;
        size(3) = 200;
        Tesseract t(origin, size);
        project_to_3D(t.get_vertices(), rot_m_, proj_m_, cam_4D_);
        std::vector<std::unique_ptr<Geometry_engine>> geoms;
        draw_tesseract(t, geoms);

        glUseProgram(programID);
        if(!io.WantCaptureMouse)
            Controls::update(window);
        int width, height;
        glfwGetWindowSize(window, &width, &height);

		glm::mat4 proj_mat = glm::perspective(fov_y * (float)DEG_TO_RAD, (float)width / height, 0.1f, 100.f);
		
        glm::vec3 camera_pos(0.f, 0.f, -3.f);
        glm::mat4 camera_mat = glm::mat4(1.f);
        camera_mat = glm::translate(camera_mat, camera_pos);

        glm::vec3 light_pos(0.f, 0.f, 70.f);

		auto world_mat = glm::toMat4(Controls::get_rotation_quat());
        auto norm_mat = glm::transpose(glm::inverse(glm::mat3(world_mat)));

        glUniformMatrix4fv(proj_mat_id, 1, GL_FALSE, glm::value_ptr(proj_mat));
        glUniformMatrix4fv(mv_mat_id, 1, GL_FALSE, glm::value_ptr(camera_mat * world_mat));
        glUniformMatrix3fv(normal_mat_id, 1, GL_FALSE, glm::value_ptr(norm_mat));
        glUniform3fv(light_pos_id, 1, glm::value_ptr(light_pos));

        //geom.draw_object();
        for(auto& g : geoms)
            g->draw_object();



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
