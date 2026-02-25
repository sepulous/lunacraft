
#include <stb_image/stb_image.h>

#include "mesh.h"
#include "storage.h"

Mesh::Mesh()
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenTextures(1, &_tex);
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteTextures(1, &_tex);
}

void Mesh::SetShader(Shader &shader)
{
    if (_shader != nullptr && _shader->GetID() != shader.GetID())
        _setup_attribs = false;

    _shader = &shader;
}

Shader *Mesh::GetShader()
{
    return _shader;
}

void Mesh::SetVertexData(void *vertex_data, size_t vertex_count, int usage)
{
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // Set up vertex attributes, if necessary
    if (!_setup_attribs)
    {
        auto vertex_attribs = _shader->GetVertexAttribs();

        // Determine bytes/vertex
        _bytes_per_vertex = 0;
        for (auto &[count, type] : vertex_attribs)
            _bytes_per_vertex += count * GetTypeSize(type);

        // Set up attributes
        size_t offset = 0;
        for (int i = 0; i < vertex_attribs.size(); i++)
        {
            auto &[count, type] = vertex_attribs[i];
            glVertexAttribPointer(i, count, type, false, _bytes_per_vertex, (void*)offset);
            glEnableVertexAttribArray(i);
            offset += count * GetTypeSize(type);
        }

        _setup_attribs = true;
    }

    bool must_reallocate = vertex_count > _vertex_count;
    _vertex_count = vertex_count;

    if (must_reallocate)
        glBufferData(GL_ARRAY_BUFFER, vertex_count * _bytes_per_vertex, vertex_data, usage);
    else
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * _bytes_per_vertex, vertex_data);
}

void Mesh::SetTexture(const std::filesystem::path &texture_path, GLenum filtering)
{
    int width, height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *texture_data = stbi_load(reinterpret_cast<const char *>(texture_path.u8string().c_str()), &width, &height, &num_channels, 0);

    glBindTexture(GL_TEXTURE_2D, _tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GLenum format = (num_channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, texture_data);

    stbi_image_free(texture_data);
}

void Mesh::SetTexture(unsigned char *texture_data, size_t width, size_t height, int num_channels, GLenum filtering)
{
    glBindTexture(GL_TEXTURE_2D, _tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GLenum format = (num_channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, texture_data);
}

void Mesh::Render(std::function<void(Shader *)> pre_draw_function)
{
    _shader->Use();
    pre_draw_function(_shader);
    glBindVertexArray(_vao);
    glBindTexture(GL_TEXTURE_2D, _tex);
    glDrawArrays(GL_TRIANGLES, 0, _vertex_count);
}

void Mesh::Render()
{
    _shader->Use();
    glBindVertexArray(_vao);
    glBindTexture(GL_TEXTURE_2D, _tex);
    glDrawArrays(GL_TRIANGLES, 0, _vertex_count);
}

size_t Mesh::GetTypeSize(int type)
{
    if (type == GL_INT)
        return sizeof(int);
    else if (type == GL_UNSIGNED_INT)
        return sizeof(unsigned);
    else if (type == GL_FLOAT)
        return sizeof(float);
    else if (type == GL_DOUBLE)
        return sizeof(double);
    else if (type == GL_BYTE)
        return sizeof(char);
    else if (type == GL_UNSIGNED_BYTE)
        return sizeof(unsigned char);
    else
        return sizeof(bool);
}
