#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Skybox
{
    private:
        GLuint _vao;
        GLuint _vbo;
        GLuint _texture;

    public:
        Skybox();
        void Update(const glm::mat4& view_projection, float skybox_angle);
        void Render();
};

#endif
