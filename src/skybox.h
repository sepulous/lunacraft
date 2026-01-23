#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>

class Skybox
{
    private:
        GLuint _vao;
        GLuint _vbo;
        GLuint _texture;

    public:
        Skybox();
        ~Skybox();
        void Update(const glm::mat4 &view_projection, float skybox_angle);
        void Render();
};
