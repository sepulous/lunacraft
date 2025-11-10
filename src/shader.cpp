#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "storage.h"

//
// Shader manager
//

Shader ShaderManager::_screen_image_shader;

void ShaderManager::CompileAllShaders()
{
    _screen_image_shader.MakeProgram(Storage::SHADER_DIR / "screen_image.vert", Storage::SHADER_DIR / "screen_image.frag");
}

Shader ShaderManager::GetShader(int shader)
{
    switch (shader)
    {
        case SHADER_SCREEN_IMAGE: return _screen_image_shader;
        default: return _screen_image_shader; // TODO: Special error shader if invalid shader is requested
    }
}

//
// Shaders
//

Shader::Shader(std::filesystem::path vertex_shader_path, std::filesystem::path fragment_shader_path)
{
    MakeProgram(vertex_shader_path, fragment_shader_path);
}

Shader::Shader() {}

void Shader::MakeProgram(std::filesystem::path vertex_shader_path, std::filesystem::path fragment_shader_path)
{
    int success;
    char error_log[512];

    // Vertex shader
    unsigned int vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    std::ifstream vertex_shader_file(vertex_shader_path);
    std::string vertex_shader_source((std::istreambuf_iterator<char>(vertex_shader_file)), std::istreambuf_iterator<char>());
    const char *vertex_shader_source_cstr = vertex_shader_source.c_str();
    glShaderSource(vertex_shader_id, 1, &vertex_shader_source_cstr, NULL);
    glCompileShader(vertex_shader_id);

    // Check for errors
    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader_id, 512, NULL, error_log);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << error_log << std::endl;
    }

    // Fragment shader
    unsigned int fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    std::ifstream fragment_shader_file(fragment_shader_path);
    std::string fragment_shader_source((std::istreambuf_iterator<char>(fragment_shader_file)), std::istreambuf_iterator<char>());
    const char *fragment_shader_source_cstr = fragment_shader_source.c_str();
    glShaderSource(fragment_shader_id, 1, &fragment_shader_source_cstr, NULL);
    glCompileShader(fragment_shader_id);

    // Check for errors
    glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader_id, 512, NULL, error_log);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << error_log << std::endl;
    }

    // Create shader program
    _id = glCreateProgram();
    glAttachShader(_id, vertex_shader_id);
    glAttachShader(_id, fragment_shader_id);
    glLinkProgram(_id);
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Check for errors
    glGetShaderiv(_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(_id, 512, NULL, error_log);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << error_log << std::endl;
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
