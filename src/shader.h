#ifndef SHADER_H
#define SHADER_H

#include <filesystem>

#include <glm/glm.hpp>

class Shader
{
    private:
        GLuint _id;

    public:
        Shader();
        Shader(std::filesystem::path vertex_shader_path, std::filesystem::path fragment_shader_path);

        Shader(const Shader&) = delete;
        Shader &operator=(const Shader&) = delete;

        Shader(Shader&&) = delete;
        Shader &operator=(Shader&&) = delete;

        void MakeProgram(std::filesystem::path vertex_shader_path, std::filesystem::path fragment_shader_path);
        void Use();
        GLuint GetID();
        void SetInt(const char *name, int value);
        void SetFloat(const char *name, float value);
        void SetVec3(const char *name, const glm::vec3& vec);
        void SetVec4(const char *name, const glm::vec4& vec);
        void SetMat3(const char *name, const glm::mat3& mat);
        void SetMat4(const char *name, const glm::mat4& mat);
};

class ShaderManager
{
    public:
        static Shader UI_IMAGE_SHADER;
        static Shader UI_TEXT_SHADER;
        static Shader BLOCK_SHADER;
        static Shader SKYBOX_SHADER;

    public:
        ShaderManager() = delete;
        static void CompileAllShaders();
};

#endif
