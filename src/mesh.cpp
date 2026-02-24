
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
    _shader = &shader;
}

Shader *Mesh::GetShader()
{
    return _shader;
}

void Mesh::SetVertexAttribs(const std::vector<size_t> &attrib_counts)
{
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // Determine stride and floats/vertex
    size_t stride = 0;
    _floats_per_vertex = 0;
    for (size_t count : attrib_counts)
    {
        stride += count * sizeof(float);
        _floats_per_vertex += count;
    }

    // Set up attributes
    size_t offset = 0;
    for (int i = 0; i < attrib_counts.size(); i++)
    {
        size_t count = attrib_counts[i];
        glVertexAttribPointer(i, count, GL_FLOAT, false, stride, (void*)offset);
        glEnableVertexAttribArray(i);
        offset += count * sizeof(float);
    }
}

void Mesh::SetVertices(const std::vector<float> &vertices)
{
    bool must_reallocate = vertices.size() > _vertices.size();
    _vertices = vertices;

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    if (must_reallocate)
        glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(float), _vertices.data(), GL_STATIC_DRAW);
    else
        glBufferSubData(GL_ARRAY_BUFFER, 0, _vertices.size() * sizeof(float), _vertices.data());
}

std::vector<float> &Mesh::GetVertices()
{
    return _vertices;
}

void Mesh::SetTexture(const std::filesystem::path &texture_path, GLenum filtering)
{
    int width, height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *texture_data = stbi_load(reinterpret_cast<const char *>(texture_path.u8string().c_str()), &width, &height, &num_channels, 0);

    glBindTexture(GL_TEXTURE_2D, _tex);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GLenum format = (num_channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, texture_data);
}

void Mesh::Render(const glm::mat4 &mvp_matrix)
{
    _shader->Use();
    _shader->SetMat4("u_mvp_matrix", mvp_matrix);
    glBindVertexArray(_vao);
    glBindTexture(GL_TEXTURE_2D, _tex);
    glDrawArrays(GL_TRIANGLES, 0, _vertices.size() / _floats_per_vertex);
}
