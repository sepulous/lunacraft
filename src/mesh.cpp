
#include <stb_image/stb_image.h>

#include "mesh.h"
#include "storage.h"

Mesh::Mesh()
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenTextures(1, &tex_);
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteTextures(1, &tex_);
}

void Mesh::SetShader(Shader &shader)
{
    if (shader_ != nullptr && shader_->GetID() != shader.GetID())
        setup_attribs_ = false;

    shader_ = &shader;
}

Shader *Mesh::GetShader()
{
    return shader_;
}

void Mesh::SetVertexData(void *vertex_data, size_t vertex_count, int usage)
{
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    // Set up vertex attributes, if necessary
    if (!setup_attribs_)
    {
        auto vertex_attribs = shader_->GetVertexAttribs();

        // Determine bytes/vertex
        bytes_per_vertex_ = 0;
        for (auto &[count, type] : vertex_attribs)
            bytes_per_vertex_ += count * GetTypeSize(type);

        // Set up attributes
        size_t offset = 0;
        for (int i = 0; i < vertex_attribs.size(); i++)
        {
            auto &[count, type] = vertex_attribs[i];
            glVertexAttribPointer(i, count, type, false, bytes_per_vertex_, (void*)offset);
            glEnableVertexAttribArray(i);
            offset += count * GetTypeSize(type);
        }

        setup_attribs_ = true;
    }

    bool must_reallocate = vertex_count > vertex_count_;
    vertex_count_ = vertex_count;

    if (must_reallocate)
        glBufferData(GL_ARRAY_BUFFER, vertex_count * bytes_per_vertex_, vertex_data, usage);
    else
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * bytes_per_vertex_, vertex_data);
}

void Mesh::SetTexture(const std::filesystem::path &texture_path, GLenum filtering)
{
    int width, height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *texture_data = stbi_load(reinterpret_cast<const char *>(texture_path.u8string().c_str()), &width, &height, &num_channels, 0);

    glBindTexture(GL_TEXTURE_2D, tex_);
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
    glBindTexture(GL_TEXTURE_2D, tex_);
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
    shader_->Use();
    pre_draw_function(shader_);
    glBindVertexArray(vao_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
}

void Mesh::Render()
{
    shader_->Use();
    glBindVertexArray(vao_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
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
