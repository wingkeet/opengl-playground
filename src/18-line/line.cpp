#include "glad.h"
#include <filesystem>
#include <fmt/core.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "shader.h"
#include "utils.h"

// Global variables
static GLuint program{};
static glm::mat4 proj_matrix{};

static GLuint compile_shaders()
{
    namespace fs = std::filesystem;
    return compile_shaders({
        fs::canonical(dirname() / ".." / "shader" / "line.vert").c_str(),
        fs::canonical(dirname() / ".." / "shader" / "line.frag").c_str(),
    });
}

static void set_viewport(GLFWwindow* window)
{
    int width{}, height{};
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    const float w = width, h = height;
    const float aspect = w / h;
    proj_matrix = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -10.0f, 10.0f);
    const GLint loc_res = glGetUniformLocation(program, "u_resolution");
    glUniform2f(loc_res, w, h);
}

static void set_callbacks(GLFWwindow* window)
{
    glfwSetFramebufferSizeCallback(
        window,
        [](GLFWwindow* window, int width, int height) {
            set_viewport(window);
        }
    );
    glfwSetKeyCallback(
        window,
        [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            else if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
                // Press F5 to reload shaders
                glDeleteProgram(program);
                program = compile_shaders();
                glUseProgram(program);
            }
        }
    );
}

static void print_info()
{
    fmt::print("GLFW version: {}\n", glfwGetVersionString());
    fmt::print("GL_VENDOR: {}\n", glGetString(GL_VENDOR));
    fmt::print("GL_RENDERER: {}\n", glGetString(GL_RENDERER));
    fmt::print("GL_VERSION: {}\n", glGetString(GL_VERSION));
    fmt::print("GL_SHADING_LANGUAGE_VERSION: {}\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    GLint max_vertex_attribs{};
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);
    fmt::print("GL_MAX_VERTEX_ATTRIBS: {}\n", max_vertex_attribs);

    GLint max_vertex_attrib_bindings{};
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &max_vertex_attrib_bindings);
    fmt::print("GL_MAX_VERTEX_ATTRIB_BINDINGS: {}\n", max_vertex_attrib_bindings);

    GLint max_uniform_locations{};
    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &max_uniform_locations);
    fmt::print("GL_MAX_UNIFORM_LOCATIONS: {}\n", max_uniform_locations);

    GLint max_uniform_block_size{};
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &max_uniform_block_size);
    fmt::print("GL_MAX_UNIFORM_BLOCK_SIZE: {}\n", max_uniform_block_size);

    GLint max_uniform_buffer_bindings{};
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_uniform_buffer_bindings);
    fmt::print("GL_MAX_UNIFORM_BUFFER_BINDINGS: {}\n", max_uniform_buffer_bindings);

    GLint max_shader_storage_block_size{};
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_shader_storage_block_size);
    fmt::print("GL_MAX_SHADER_STORAGE_BLOCK_SIZE: {}\n", max_shader_storage_block_size);

    GLint max_shader_storage_buffer_bindings{};
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &max_shader_storage_buffer_bindings);
    fmt::print("GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS: {}\n", max_shader_storage_buffer_bindings);
}

static GLuint create_ssbo(const std::vector<glm::vec4>& varray)
{
    GLuint ssbo{};
    glCreateBuffers(1, &ssbo);
    glNamedBufferStorage(ssbo, varray.size()*sizeof(*varray.data()), varray.data(), 0);
    return ssbo;
}

int main()
{
    glfwSetErrorCallback(
        [](int error, const char* description) {
            fmt::print(stderr, "ERROR: {}\n",  description);
        }
    );

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "18-line", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1); // vsync on

    print_info();
    set_callbacks(window);

    program = compile_shaders();
    glUseProgram(program);

    // https://stackoverflow.com/questions/60440682/drawing-a-line-in-modern-opengl

    const GLint loc_mvp = glGetUniformLocation(program, "u_mvp");
    const GLint loc_thi = glGetUniformLocation(program, "u_thickness");

    glUniform1f(loc_thi, 20.0f);

    std::vector<glm::vec4> varray;
    varray.emplace_back(glm::vec4{0.0f, -1.0f, 0.0f, 1.0f});
    varray.emplace_back(glm::vec4{1.0f, -1.0f, 0.0f, 1.0f});
    for (int u{}; u <= 90; u += 10) {
        const float a = glm::radians(static_cast<float>(u));
        const float c = std::cos(a), s = std::sin(a);
        varray.emplace_back(glm::vec4{c, s, 0.0f, 1.0f});
    }
    varray.emplace_back(glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f));
    for (int u{90}; u >= 0; u -= 10) {
        const float a = glm::radians(static_cast<float>(u));
        const float c = std::cos(a), s = std::sin(a);
        varray.emplace_back(glm::vec4{c-1.0f, s-1.0f, 0.0f, 1.0f});
    }
    varray.emplace_back(glm::vec4{1.0f, -1.0f, 0.0f, 1.0f});
    varray.emplace_back(glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});
    const GLuint ssbo = create_ssbo(varray);

    GLuint vao{};
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    const GLsizei N = static_cast<GLsizei>(varray.size()) - 2;

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    set_viewport(window);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw filled polygons
        {
            glm::mat4 mv_matrix{1.0f};
            mv_matrix = glm::translate(mv_matrix, glm::vec3{-0.6f, 0.0f, 0.0f});
            mv_matrix = glm::scale(mv_matrix, glm::vec3{0.5f, 0.5f, 1.0f});
            const glm::mat4 mvp_matrix = proj_matrix * mv_matrix;

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp_matrix));
            glDrawArrays(GL_TRIANGLES, 0, 6*(N-1));
        }

        // Draw outlined polygons
        {
            glm::mat4 mv_matrix{1.0f};
            mv_matrix = glm::translate(mv_matrix, glm::vec3{0.6f, 0.0f, 0.0f});
            mv_matrix = glm::scale(mv_matrix, glm::vec3{0.5f, 0.5f, 1.0f});
            const glm::mat4 mvp_matrix = proj_matrix * mv_matrix;

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp_matrix));
            glDrawArrays(GL_TRIANGLES, 0, 6*(N-1));
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    fmt::print("Bye.\n");
    return 0;
}
