#include "Line_2D_renderer.h"

Line_2D_renderer::Line_2D_renderer()
{
    glGenBuffers(1, &array_buff_id_);
    glGenBuffers(1, &index_buff_id_);
}

Line_2D_renderer::~Line_2D_renderer()
{
    glDeleteBuffers(1, &array_buff_id_);
    glDeleteBuffers(1, &index_buff_id_);
}

void Line_2D_renderer::add_line(Line_2D line)
{
    data_vector.clear();
    indices.clear();

    Array_data d1, d2;

    d1.vert = line.start_pos;
    d1.color = line.color;

    d2.vert = line.end_pos;
    d2.color = line.color;

    data_vector.push_back(d1);
    data_vector.push_back(d2);

    indices.push_back(0);
    indices.push_back(1);

    // Allocating buffers
    glBindBuffer(GL_ARRAY_BUFFER, array_buff_id_);
	glBufferData(GL_ARRAY_BUFFER,
                 data_vector.size() * sizeof(Array_data),
                 &data_vector[0],
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buff_id_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(GLuint),
                 &indices[0],
                 GL_STATIC_DRAW);
}

void Line_2D_renderer::add_lines(std::vector<Line_2D> lines)
{
    // TODO: implement
}

void Line_2D_renderer::draw_object()
{
    glBindBuffer(GL_ARRAY_BUFFER, array_buff_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buff_id_);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    GLsizei stride = sizeof(Array_data);
    void* ptr = reinterpret_cast<void*>(2 * sizeof(GLfloat));
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, ptr);

    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
