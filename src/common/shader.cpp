#include "glad.h"
#include <fmt/core.h>
#include <initializer_list>
#include <vector>
#include "shader.h"
#include "utils.h"

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
    const std::string str = read_file(filename);
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

GLuint compile_shaders(const std::initializer_list<std::string_view>& filenames)
{
    // Create program, attach shaders to it, and link it
    const GLuint program = glCreateProgram();
    std::vector<GLuint> shaders;
    shaders.reserve(filenames.size());
    for (auto filename : filenames) {
        const GLuint shader = create_shader(filename);
        glAttachShader(program, shader);
        shaders.emplace_back(shader);
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
