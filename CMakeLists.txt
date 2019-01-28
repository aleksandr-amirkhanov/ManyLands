cmake_minimum_required (VERSION 2.6)
project (ManyLands)

set(CMAKE_CXX_STANDARD 20)
if(WIN32)
	set(GUI_TYPE WIN32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/glfw)

find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
include_directories("include/GL3W" "include/glm" "include/imgui")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB H_FILES "src/*.h")
file(GLOB CPP_FILES "src/*.cpp")
file(GLOB GL3W_FILES "include/GL3W/GL/*.h" "include/GL3W/GL/*.c")
file(GLOB IMGUI_FILES "include/imgui/*.h" "include/imgui/*.cpp")
file(GLOB SHADERS_FILES "shaders/*.frag" "shaders/*.vert")
	
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(ManyLands ${GUI_TYPE} ${CPP_FILES} ${IMGUI_FILES} ${GL3W_FILES} ${H_FILES} ${SHADERS_FILES})
source_group("imgui" FILES ${IMGUI_FILES})
source_group("gl3w" FILES ${GL3W_FILES})
source_group("shaders" FILES ${SHADERS_FILES})

target_link_libraries(ManyLands glfw ${OPENGL_LIBRARIES})
