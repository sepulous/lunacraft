#ifndef SHADER_H
#define SHADER_H

#include <glm/glm.hpp>

class Shader
{
    private:
        unsigned int _id;

    public:
        Shader();
        Shader(const char *vertexShaderPath, const char *fragmentShaderPath);
        void MakeProgram(const char *vertexShaderPath, const char *fragmentShaderPath);
        void Use();
        unsigned int GetID();
        void SetInt(const char *name, int value);
        void SetFloat(const char *name, float value);
        void SetVec3(const char *name, glm::vec3 vec);
        void SetVec4(const char *name, glm::vec4 vec);
        void SetMat3(const char *name, glm::mat3 mat);
        void SetMat4(const char *name, glm::mat4 mat);
};

#endif
