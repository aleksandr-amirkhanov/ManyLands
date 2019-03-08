// The original code was taken from http://www.opengl-tutorial.org/ under the
// WTFPL public licence (2004). The original code was slighly modified for the
// current project

#pragma once

#if defined(USE_GL_ES3)
#include <GLES3/gl3.h>  // Use GL ES 3
#else
#include <GL/gl3w.h>
#endif

class Base_shader
{
public:
    virtual void initialize() = 0;
protected:
    GLuint load_shaders(const char* vertex_file_path,
                        const char* fragment_file_path);
};
