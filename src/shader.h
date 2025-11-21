#ifndef SHADER_H
#define SHADER_H

#include <filesystem>

#include <glm/glm.hpp>

#define SHADER_SCREEN_IMAGE 0
#define SHADER_SCREEN_TEXT 1

class Shader
{
    private:
        unsigned int _id;

    public:
        Shader();
        Shader(std::filesystem::path vertex_shader_path, std::filesystem::path fragment_shader_path);
        void MakeProgram(std::filesystem::path vertex_shader_path, std::filesystem::path fragment_shader_path);
        void Use();
        unsigned int GetID();
        void SetInt(const char *name, int value);
        void SetFloat(const char *name, float value);
        void SetVec3(const char *name, glm::vec3 vec);
        void SetVec4(const char *name, glm::vec4 vec);
        void SetMat3(const char *name, glm::mat3 mat);
        void SetMat4(const char *name, glm::mat4 mat);
};

class ShaderManager
{
    private:
        static Shader _screen_image_shader;
        static Shader _screen_text_shader;

    public:
        ShaderManager() = delete;
        static void CompileAllShaders();
        static Shader GetShader(int shader_id);
};

#endif
