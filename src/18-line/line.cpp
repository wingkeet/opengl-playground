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

static GLuint compile_shaders()
{
    namespace fs = std::filesystem;
    return compile_shaders({
        fs::canonical(dirname() / ".." / "shader" / "line.vert").c_str(),
        fs::canonical(dirname() / ".." / "shader" / "line.frag").c_str(),
    });
}

static void set_callbacks(GLFWwindow* window)
{
    glfwSetFramebufferSizeCallback(
        window,
        [](GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
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

    GLint max_shader_storage_block_size{};
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_shader_storage_block_size);
    fmt::print("GL_MAX_SHADER_STORAGE_BLOCK_SIZE: {}\n", max_shader_storage_block_size);
}

static GLuint create_ssbo(std::vector<glm::vec4> &varray)
{
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo );
    glBufferData(GL_SHADER_STORAGE_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);
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

    GLint loc_mvp = glGetUniformLocation(program, "u_mvp");
    GLint loc_res = glGetUniformLocation(program, "u_resolution");
    GLint loc_thi = glGetUniformLocation(program, "u_thickness");

    glUniform1f(loc_thi, 20.0);

    std::vector<glm::vec4> varray;
    varray.emplace_back(glm::vec4{0.0f, -1.0f, 0.0f, 1.0f});
    varray.emplace_back(glm::vec4{1.0f, -1.0f, 0.0f, 1.0f});
    for (int u{}; u <= 90; u += 10)
    {
        double a = glm::radians((float)u);
        double c = std::cos(a), s = std::sin(a);
        varray.emplace_back(glm::vec4{(float)c, (float)s, 0.0f, 1.0f});
    }
    varray.emplace_back(glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f));
    for (int u{90}; u >= 0; u -= 10)
    {
        double a = glm::radians((float)u);
        double c = std::cos(a), s = std::sin(a);
        varray.emplace_back(glm::vec4{(float)c-1.0f, (float)s-1.0f, 0.0f, 1.0f});
    }
    varray.emplace_back(glm::vec4{1.0f, -1.0f, 0.0f, 1.0f});
    varray.emplace_back(glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});
    const GLuint ssbo = create_ssbo(varray);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    const GLsizei N = (GLsizei)varray.size() - 2;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glm::mat4 project{};
    int vpSize[2]{0, 0};
    while (!glfwWindowShouldClose(window))
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w != vpSize[0] ||  h != vpSize[1])
        {
            vpSize[0] = w; vpSize[1] = h;
            glViewport(0, 0, vpSize[0], vpSize[1]);
            float aspect = (float)w / (float)h;
            project = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -10.0f, 10.0f);
            glUniform2f(loc_res, (float)w, (float)h);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 modelview1{1.0f};
        modelview1 = glm::translate(modelview1, glm::vec3{-0.6f, 0.0f, 0.0f});
        modelview1 = glm::scale(modelview1, glm::vec3{0.5f, 0.5f, 1.0f});
        const glm::mat4 mvp1 = project * modelview1;

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp1));
        glDrawArrays(GL_TRIANGLES, 0, 6*(N-1));

        glm::mat4 modelview2{1.0f};
        modelview2 = glm::translate(modelview2, glm::vec3{0.6f, 0.0f, 0.0f});
        modelview2 = glm::scale(modelview2, glm::vec3{0.5f, 0.5f, 1.0f});
        const glm::mat4 mvp2 = project * modelview2;

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp2));
        glDrawArrays(GL_TRIANGLES, 0, 6*(N-1));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    fmt::print("Bye.\n");
    return 0;
}
