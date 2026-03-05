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

Shader ShaderManager::UI_IMAGE_SHADER;
Shader ShaderManager::UI_TEXT_SHADER;
Shader ShaderManager::BLOCK_SHADER;
Shader ShaderManager::SIMPLE_UNLIT_SHADER;
Shader ShaderManager::SKYBOX_SHADER;

void ShaderManager::CompileAllShaders()
{
    UI_IMAGE_SHADER.SetFragmentShader(Storage::SHADERS / "ui_image.frag");
    UI_IMAGE_SHADER.SetVertexShader(Storage::SHADERS / "ui_image.vert", {
        {4, GL_FLOAT}
    });
    UI_IMAGE_SHADER.Compile();

    ///////////////////

    UI_TEXT_SHADER.SetFragmentShader(Storage::SHADERS / "ui_text.frag");
    UI_TEXT_SHADER.SetVertexShader(Storage::SHADERS / "ui_text.vert", {
        {4, GL_FLOAT}
    });
    UI_TEXT_SHADER.Compile();

    ///////////////////

    BLOCK_SHADER.SetFragmentShader(Storage::SHADERS / "block.frag");
    BLOCK_SHADER.SetVertexShader(Storage::SHADERS / "block.vert", {
        {3, GL_FLOAT},
        {4, GL_FLOAT},
        {3, GL_FLOAT},
        {3, GL_FLOAT},
    });
    BLOCK_SHADER.Compile();

    ///////////////////

    SIMPLE_UNLIT_SHADER.SetFragmentShader(Storage::SHADERS / "simple_unlit.frag");
    SIMPLE_UNLIT_SHADER.SetVertexShader(Storage::SHADERS / "simple_unlit.vert", {
        {3, GL_FLOAT},
        {2, GL_FLOAT},
    });
    SIMPLE_UNLIT_SHADER.Compile();

    ///////////////////

    SKYBOX_SHADER.SetFragmentShader(Storage::SHADERS / "skybox.frag");
    SKYBOX_SHADER.SetVertexShader(Storage::SHADERS / "skybox.vert", {
        {3, GL_FLOAT},
    });
    SKYBOX_SHADER.Compile();
}

//
// Shaders
//

Shader::Shader(const std::filesystem::path &fragment_shader_path, const std::filesystem::path &vertex_shader_path, const VertexAttribs &vertex_attribs)
{
    fragment_shader_path_ = fragment_shader_path;
    vertex_shader_path_ = vertex_shader_path;
    vertex_attribs_ = vertex_attribs;
}

Shader::Shader() {}

void Shader::SetFragmentShader(const std::filesystem::path &fragment_shader_path)
{
    fragment_shader_path_ = fragment_shader_path;
}

void Shader::SetVertexShader(const std::filesystem::path &vertex_shader_path, const VertexAttribs &vertex_attribs)
{
    vertex_shader_path_ = vertex_shader_path;
    vertex_attribs_ = vertex_attribs;
}

void Shader::Compile()
{
    int success;
    char error_log[512];

    //
    // Vertex shader
    //
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    std::ifstream vertex_shader_file(vertex_shader_path_);
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

    //
    // Fragment shader
    //
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    std::ifstream fragment_shader_file(fragment_shader_path_);
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

    //
    // Create shader program
    //
    id_ = glCreateProgram();
    glAttachShader(id_, vertex_shader_id);
    glAttachShader(id_, fragment_shader_id);
    glLinkProgram(id_);
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Check for errors
    glGetShaderiv(id_, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id_, 512, NULL, error_log);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << error_log << std::endl;
    }
}

void Shader::Use()
{
    glUseProgram(id_);
}

GLuint Shader::GetID()
{
    return id_;
}

VertexAttribs Shader::GetVertexAttribs()
{
    return vertex_attribs_;
}

void Shader::SetInt(const char *name, int value)
{
    glUniform1i(glGetUniformLocation(id_, name), value);
}

void Shader::SetFloat(const char *name, float value)
{
    glUniform1f(glGetUniformLocation(id_, name), value);
}

void Shader::SetVec2(const char *name, const glm::vec2& vec)
{
    glUniform2f(glGetUniformLocation(id_, name), vec.x, vec.y);
}

void Shader::SetVec3(const char *name, const glm::vec3& vec)
{
    glUniform3f(glGetUniformLocation(id_, name), vec.x, vec.y, vec.z);
}

void Shader::SetVec4(const char *name, const glm::vec4& vec)
{
    glUniform4f(glGetUniformLocation(id_, name), vec.x, vec.y, vec.z, vec.w);
}

void Shader::SetMat3(const char *name, const glm::mat3& mat)
{
    glUniformMatrix3fv(glGetUniformLocation(id_, name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::SetMat4(const char *name, const glm::mat4& mat)
{
    glUniformMatrix4fv(glGetUniformLocation(id_, name), 1, GL_FALSE, glm::value_ptr(mat));
}
