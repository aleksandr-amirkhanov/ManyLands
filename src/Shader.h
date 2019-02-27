// The original code was taken from http://www.opengl-tutorial.org/ under the
// WTFPL public licence (2004). The original code was slighly modified for the
// current project

#pragma once

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>  // Use GL ES 3
#else
#include <GL/gl3w.h>
#endif

GLuint load_shaders(const char * vertex_file_path, const char * fragment_file_path);
