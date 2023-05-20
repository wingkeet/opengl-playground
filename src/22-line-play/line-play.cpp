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

static GLuint create_program()
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
                program = create_program();
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

    GLint max_uniform_block_size{};
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &max_uniform_block_size);
    fmt::print("GL_MAX_UNIFORM_BLOCK_SIZE: {}\n", max_uniform_block_size);

    GLint max_uniform_buffer_bindings{};
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_uniform_buffer_bindings);
    fmt::print("GL_MAX_UNIFORM_BUFFER_BINDINGS: {}\n", max_uniform_buffer_bindings);

    // https://www.geeks3d.com/20140704/tutorial-introduction-to-opengl-4-3-shader-storage-buffers-objects-ssbo-demo/

    GLint max_shader_storage_block_size{};
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_shader_storage_block_size);
    fmt::print("GL_MAX_SHADER_STORAGE_BLOCK_SIZE: {}\n", max_shader_storage_block_size);

    GLint max_shader_storage_buffer_bindings{};
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &max_shader_storage_buffer_bindings);
    fmt::print("GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS: {}\n", max_shader_storage_buffer_bindings);

    GLint max_vertex_shader_storage_blocks{};
    glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &max_vertex_shader_storage_blocks);
    fmt::print("GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS: {}\n", max_vertex_shader_storage_blocks);

    GLint max_fragment_shader_storage_blocks{};
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &max_fragment_shader_storage_blocks);
    fmt::print("GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS: {}\n", max_fragment_shader_storage_blocks);

    GLint max_geometry_shader_storage_blocks{};
    glGetIntegerv(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS, &max_geometry_shader_storage_blocks);
    fmt::print("GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS: {}\n", max_geometry_shader_storage_blocks);

    GLint max_tess_control_shader_storage_blocks{};
    glGetIntegerv(GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS, &max_tess_control_shader_storage_blocks);
    fmt::print("GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS: {}\n", max_tess_control_shader_storage_blocks);

    GLint max_tess_evaluation_shader_storage_blocks{};
    glGetIntegerv(GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS, &max_tess_evaluation_shader_storage_blocks);
    fmt::print("GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS: {}\n", max_tess_evaluation_shader_storage_blocks);

    GLint max_compute_shader_storage_blocks{};
    glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &max_compute_shader_storage_blocks);
    fmt::print("GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS: {}\n", max_compute_shader_storage_blocks);

    GLint max_combined_shader_storage_blocks{};
    glGetIntegerv(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, &max_combined_shader_storage_blocks);
    fmt::print("GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS: {}\n", max_combined_shader_storage_blocks);
}

// https://stackoverflow.com/questions/27810542/what-is-the-difference-between-glbufferstorage-and-glbufferdata
static GLuint create_ssbo(const std::vector<glm::vec4>& varray)
{
    GLuint ssbo{};
    glCreateBuffers(1, &ssbo);
    glNamedBufferStorage(ssbo, varray.size()*sizeof(*varray.data()), varray.data(), 0);
    const GLuint binding_point_index{0}; // [0..GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point_index, ssbo);
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "22-line-play", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1); // vsync on

    print_info();
    set_callbacks(window);

    program = create_program();
    glUseProgram(program);

    // https://stackoverflow.com/questions/60440682/drawing-a-line-in-modern-opengl

    const GLint loc_mvp = glGetUniformLocation(program, "u_mvp");
    const GLint loc_thi = glGetUniformLocation(program, "u_thickness");

    glUniform1f(loc_thi, 20.0f);

    // Minimum 4 vertices
    std::vector<glm::vec4> varray{
        {-0.5f, 0.0f, 0.0f, 1.0f},
        {-0.5f, 0.0f, 0.0f, 1.0f},
        {+0.5f, 0.0f, 0.0f, 1.0f},
        {+1.0f, 0.0f, 0.0f, 1.0f},
    };
    const GLuint ssbo = create_ssbo(varray);

    GLuint vao{};
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
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
