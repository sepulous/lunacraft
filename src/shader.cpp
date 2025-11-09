#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

Shader::Shader(const char *vertexShaderPath, const char *fragmentShaderPath)
{
    MakeProgram(vertexShaderPath, fragmentShaderPath);
}

Shader::Shader() {}

void Shader::MakeProgram(const char *vertexShaderPath, const char *fragmentShaderPath)
{
    int success;
    char errorLog[512];

    // Vertex shader
    unsigned int vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    std::ifstream vertexShaderFile(vertexShaderPath);
    std::string vertexShaderSource((std::istreambuf_iterator<char>(vertexShaderFile)), std::istreambuf_iterator<char>());
    const char *vertexShaderSource_c = vertexShaderSource.c_str();
    glShaderSource(vertexShaderID, 1, &vertexShaderSource_c, NULL);
    glCompileShader(vertexShaderID);

    // Check for errors
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderID, 512, NULL, errorLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << errorLog << std::endl;
    }

    // Fragment shader
    unsigned int fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::ifstream fragmentShaderFile(fragmentShaderPath);
    std::string fragmentShaderSource((std::istreambuf_iterator<char>(fragmentShaderFile)), std::istreambuf_iterator<char>());
    const char *fragmentShaderSource_c = fragmentShaderSource.c_str();
    glShaderSource(fragmentShaderID, 1, &fragmentShaderSource_c, NULL);
    glCompileShader(fragmentShaderID);

    // Check for errors
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderID, 512, NULL, errorLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << errorLog << std::endl;
    }

    // Create shader program
    _id = glCreateProgram();
    glAttachShader(_id, vertexShaderID);
    glAttachShader(_id, fragmentShaderID);
    glLinkProgram(_id);
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    // Check for errors
    glGetShaderiv(_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(_id, 512, NULL, errorLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << errorLog << std::endl;
    }
}

void Shader::Use()
{
    glUseProgram(_id);
}

unsigned int Shader::GetID()
{
    return _id;
}

void Shader::SetInt(const char *name, int value)
{
    glUniform1i(glGetUniformLocation(_id, name), value);
}

void Shader::SetFloat(const char *name, float value)
{
    glUniform1f(glGetUniformLocation(_id, name), value);
}

void Shader::SetVec3(const char *name, glm::vec3 vec)
{
    glUniform3f(glGetUniformLocation(_id, name), vec.x, vec.y, vec.z);
}

void Shader::SetVec4(const char *name, glm::vec4 vec)
{
    glUniform4f(glGetUniformLocation(_id, name), vec.x, vec.y, vec.z, vec.w);
}

void Shader::SetMat3(const char *name, glm::mat3 mat)
{
    glUniformMatrix3fv(glGetUniformLocation(_id, name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::SetMat4(const char *name, glm::mat4 mat)
{
    glUniformMatrix4fv(glGetUniformLocation(_id, name), 1, GL_FALSE, glm::value_ptr(mat));
}
