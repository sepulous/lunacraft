#pragma once

#include <filesystem>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

using VertexAttribs = std::vector<std::pair<size_t, int>>;

class Shader
{
    private:
        GLuint id_;
        std::filesystem::path vertex_shader_path_;
        std::filesystem::path fragment_shader_path_;
        VertexAttribs vertex_attribs_;

    public:
        Shader();
        Shader(const std::filesystem::path &fragment_shader_path, const std::filesystem::path &vertex_shader_path, const VertexAttribs &vertex_attribs);

        Shader(const Shader&) = delete;
        Shader &operator=(const Shader&) = delete;

        Shader(Shader&&) = delete;
        Shader &operator=(Shader&&) = delete;

        void SetFragmentShader(const std::filesystem::path &fragment_shader_path);
        void SetVertexShader(const std::filesystem::path &vertex_shader_path, const VertexAttribs &vertex_attribs);
        void Compile();
        void Use();
        GLuint GetID();
        VertexAttribs GetVertexAttribs();
        void SetInt(const char *name, int value);
        void SetFloat(const char *name, float value);
        void SetVec2(const char *name, const glm::vec2 &vec);
        void SetVec3(const char *name, const glm::vec3 &vec);
        void SetVec4(const char *name, const glm::vec4 &vec);
        void SetMat3(const char *name, const glm::mat3 &mat);
        void SetMat4(const char *name, const glm::mat4 &mat);
};

class ShaderManager
{
    public:
        static Shader UI_IMAGE_SHADER;
        static Shader UI_TEXT_SHADER;
        static Shader BLOCK_SHADER;
        static Shader SIMPLE_UNLIT_SHADER;
        static Shader MOB_SHADER;
        static Shader SKYBOX_SHADER;
        static Shader FXAA_SHADER;

    public:
        ShaderManager() = delete;
        static void CompileAllShaders();
};
