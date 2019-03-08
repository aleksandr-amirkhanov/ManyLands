// The original code was taken from http://www.opengl-tutorial.org/ under the
// WTFPL public licence (2004). The original code was slighly modified for the
// current project

#include "Base_shader.h"
// std
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <stdlib.h>
#include <string>

using namespace std;

GLuint Base_shader::load_shaders(const char* vertex_file_path,
                    const char* fragment_file_path)
{
	// Create the shaders
	GLuint vertex_shader_id   = glCreateShader(GL_VERTEX_SHADER),
	       fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string vertex_shader_code;
	std::ifstream vertex_shader_stream(vertex_file_path, std::ios::in);
	if(vertex_shader_stream.is_open())
    {
		std::stringstream sstr;
		sstr << vertex_shader_stream.rdbuf();
		vertex_shader_code = sstr.str();
		vertex_shader_stream.close();
	}
    else
    {
		printf("Impossible to open %s.\n", vertex_file_path);
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string fragment_shader_code;
	std::ifstream fragment_shader_stream(fragment_file_path, std::ios::in);
	if(fragment_shader_stream.is_open())
    {
		std::stringstream sstr;
		sstr << fragment_shader_stream.rdbuf();
		fragment_shader_code = sstr.str();
		fragment_shader_stream.close();
	}
    else
    {
        printf("Impossible to open %s.\n", fragment_file_path);
		return 0;
    }

	GLint result = GL_FALSE;
	int info_log_length;


	// Compile Vertex Shader
	printf("Compiling shader: %s\n", vertex_file_path);
	char const* vertex_source_pointer = vertex_shader_code.c_str();
	glShaderSource(vertex_shader_id, 1, &vertex_source_pointer , NULL);
	glCompileShader(vertex_shader_id);

	// Check Vertex Shader
	glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if(info_log_length > 0)
    {
		std::vector<char> VertexShaderErrorMessage(info_log_length + 1);
		glGetShaderInfoLog(vertex_shader_id, info_log_length, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}


	// Compile Fragment Shader
	printf("Compiling shader: %s\n", fragment_file_path);
	char const* fragment_source_pointer = fragment_shader_code.c_str();
	glShaderSource(fragment_shader_id, 1, &fragment_source_pointer , NULL);
	glCompileShader(fragment_shader_id);

	// Check Fragment Shader
	glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if(info_log_length > 0)
    {
		std::vector<char> fragment_shader_error_message(info_log_length+1);
		glGetShaderInfoLog(fragment_shader_id, info_log_length, NULL, &fragment_shader_error_message[0]);
		printf("%s\n", &fragment_shader_error_message[0]);
	}


	// Link the program
	printf("Linking program\n");
	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);

	// Check the program
	glGetProgramiv(program_id, GL_LINK_STATUS, &result);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if(info_log_length > 0)
    {
		std::vector<char> program_error_message(info_log_length+1);
		glGetProgramInfoLog(program_id, info_log_length, NULL, &program_error_message[0]);
		printf("%s\n", &program_error_message[0]);
	}

	
	glDetachShader(program_id, vertex_shader_id);
	glDetachShader(program_id, fragment_shader_id);
	
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	return program_id;
}
