#pragma once

#include <vector>
#include <filesystem>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader.h"

// This class only allows for float-valued vertex attributes. That's all I need right now, and I'm too lazy to generalize.
class Mesh
{
    public:
        Mesh();
        ~Mesh();

        void SetShader(Shader &shader);
        Shader *GetShader();

        void SetVertexAttribs(const std::vector<size_t> &attrib_counts);

        void SetVertices(const std::vector<float> &vertices);
        std::vector<float> &GetVertices();

        void SetTexture(const std::filesystem::path &texture_path, GLenum filtering = GL_NEAREST);
        void SetTexture(unsigned char *texture_data, size_t width, size_t height, int num_channels = 4, GLenum filtering = GL_NEAREST);

        void Render(const glm::mat4 &mvp_matrix);

    private:
        Shader *_shader;
        std::vector<float> _vertices;
        size_t _floats_per_vertex = 0;
        GLuint _vao;
        GLuint _vbo;
        GLuint _tex;
};
