#include "glad.h"
#include <cassert>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <vector>
#include "shader.h"

static bool ends_with(std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

static bool starts_with(std::string_view str, std::string_view prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

static std::string read_shader_file(std::string_view filename)
{
    std::ifstream file{filename.data()};
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static GLenum shader_type(std::string_view filename)
{
    if (ends_with(filename, ".vert")) {
        return GL_VERTEX_SHADER;
    }
    if (ends_with(filename, ".frag")) {
        return GL_FRAGMENT_SHADER;
    }
    if (ends_with(filename, ".geom")) {
        return GL_GEOMETRY_SHADER;
    }
    if (ends_with(filename, ".tesc")) {
        return GL_TESS_CONTROL_SHADER;
    }
    if (ends_with(filename, ".tese")) {
        return GL_TESS_EVALUATION_SHADER;
    }
    if (ends_with(filename, ".comp")) {
        return GL_COMPUTE_SHADER;
    }

    fmt::print(stderr, "ERROR: Invalid shader filename extension {}\n", filename);
    return 0;
}

static GLuint create_shader(std::string_view filename)
{
    const GLenum type = shader_type(filename);
    const GLuint shader = glCreateShader(type);
    const std::string str = read_shader_file(filename);
    const GLchar *source = str.c_str();
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check for compile errors
    GLint success{-1};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        GLchar log[512]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        fmt::print(stderr, "ERROR: Failed to compile shader object {} -> {}\n{}\n", shader, filename, log);
    }

    return shader;
}

GLuint compile_shaders()
{
    // Create and compile shaders
    const GLuint vertex_shader = create_shader("shader/basic.vert");
    const GLuint fragment_shader = create_shader("shader/basic.frag");

    // Create program, attach shaders to it, and link it
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // Check for link errors
    GLint success{-1};
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        GLchar log[512]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        fmt::print(stderr, "ERROR: Failed to link program object {}\n{}\n", program, log);
    }

    // Delete the shaders as the program has them now
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    success = -1;
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    fmt::print("Program object {} validation status: {}\n", program, success);

    return program;
}

GLuint compile_shaders(std::initializer_list<std::filesystem::path> paths)
{
    // Create program, attach shaders to it, and link it
    const GLuint program = glCreateProgram();
    std::vector<GLuint> shaders;
    for (auto p : paths) {
        const GLuint shader = create_shader(p.c_str());
        glAttachShader(program, shader);
        shaders.push_back(shader);
    }
    glLinkProgram(program);

    // Check for link errors
    GLint success{-1};
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        GLchar log[512]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        fmt::print(stderr, "ERROR: Failed to link program object {}\n{}\n", program, log);
    }

    // Delete the shaders as the program has them now
    for (auto shader : shaders) {
        glDeleteShader(shader);
    }

    success = -1;
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    fmt::print("Program object {} validation status: {}\n", program, success);

    return program;
}
