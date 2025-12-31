#include <iostream>
#include <filesystem>
#include <vector>

#include "skybox.h"
#include "shader.h"
#include "storage.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image/stb_image.h>

Skybox::Skybox()
{
    float skybox_vertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);
    
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int skybox;
    glGenTextures(1, &skybox);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);

    std::vector<std::filesystem::path> faces = {
        Storage::IMAGE_DIR / "skybox" / "skybox_right.png",
        Storage::IMAGE_DIR / "skybox" / "skybox_left.png",
        Storage::IMAGE_DIR / "skybox" / "skybox_top.png",
        Storage::IMAGE_DIR / "skybox" / "skybox_bottom.png",
        Storage::IMAGE_DIR / "skybox" / "skybox_front.png",
        Storage::IMAGE_DIR / "skybox" / "skybox_back.png"
    };
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        int width, height, nrChannels;
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Skybox::Update(const glm::mat4& view_projection, float skybox_angle)
{
    Shader skybox_shader = ShaderManager::SKYBOX_SHADER;
    skybox_shader.Use();
    glUniformMatrix4fv(glGetUniformLocation(skybox_shader.GetID(), "view_projection"), 1, GL_FALSE, glm::value_ptr(view_projection));
    glm::mat3 skybox_rotation(
        glm::vec3(1, 0, 0),
        glm::vec3(0, glm::cos(skybox_angle), glm::sin(skybox_angle)),
        glm::vec3(0, -glm::sin(skybox_angle), glm::cos(skybox_angle))
    );
    glUniformMatrix3fv(glGetUniformLocation(skybox_shader.GetID(), "rotation"), 1, GL_FALSE, glm::value_ptr(skybox_rotation));
}

void Skybox::Render()
{
    ShaderManager::SKYBOX_SHADER.Use();
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
