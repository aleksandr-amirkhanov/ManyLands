cmake_minimum_required (VERSION 2.6)
project (ManyLands)

set(CMAKE_CXX_STANDARD 17)
if(WIN32)
    set(GUI_TYPE WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

#find_package(Boost REQUIRED )
if(NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)
    find_package(SDL2 REQUIRED)
endif()

if(NOT WIN32 OR EMSCRIPTEN)
    add_compile_definitions(USE_GL_ES3)
endif()

include_directories("include" "include/glm" "include/imgui")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
if(WIN32)
    include_directories(${OPENGL_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
    include_directories("include/GL3W")
endif()

file(GLOB H_FILES "src/*.h")
file(GLOB CPP_FILES "src/*.cpp")
file(GLOB IMGUI_FILES "include/imgui/*.h" "include/imgui/*.cpp")
file(GLOB SHADERS_FILES "shaders/*.frag" "shaders/*.vert")
if(WIN32)
    file(GLOB GL3W_FILES "include/GL3W/GL/*.h" "include/GL3W/GL/*.c")
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
    add_executable(ManyLands ${GUI_TYPE} ${CPP_FILES} ${GL3W_FILES} ${IMGUI_FILES} ${H_FILES} ${SHADERS_FILES})
else()
    add_executable(ManyLands ${GUI_TYPE} ${CPP_FILES} ${IMGUI_FILES} ${H_FILES} ${SHADERS_FILES})
endif()

source_group("imgui" FILES ${IMGUI_FILES})
source_group("shaders" FILES ${SHADERS_FILES})

if(WIN32)
    source_group("gl3w" FILES ${GL3W_FILES})
endif()

if(EMSCRIPTEN)
    # Emscripten
    target_link_libraries(ManyLands "-s USE_SDL=2 -s FULL_ES3=1 -s USE_WEBGL2=1 -o ManyLands.html")
    set_target_properties(ManyLands PROPERTIES COMPILE_FLAGS "-s USE_SDL=2 -s FULL_ES3=1 -s USE_WEBGL2=1")
    set_target_properties(ManyLands PROPERTIES LINK_FLAGS "-s ALLOW_MEMORY_GROWTH=1 --preload-file assets")
else()
    string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
    target_link_libraries(ManyLands ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES})
endif()
