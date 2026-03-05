#pragma once

#include <vector>
#include <filesystem>
#include <variant>
#include <functional>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader.h"

class Mesh
{
    public:
        Mesh();
        ~Mesh();

        void SetShader(Shader &shader);
        Shader *GetShader();

        void SetVertexData(void *vertex_data, size_t vertex_count, int usage = GL_STATIC_DRAW);

        void SetTexture(const std::filesystem::path &texture_path, GLenum filtering = GL_NEAREST);
        void SetTexture(unsigned char *texture_data, size_t width, size_t height, int num_channels = 4, GLenum filtering = GL_NEAREST);

        void Render(std::function<void(Shader *)> pre_draw_function);
        void Render();

    private:
        size_t GetTypeSize(int type);

    private:
        Shader *shader_ = nullptr;
        bool setup_attribs_ = false;
        size_t bytes_per_vertex_ = 0;
        size_t vertex_count_ = 0;
        GLuint vao_;
        GLuint vbo_;
        GLuint tex_;
};
