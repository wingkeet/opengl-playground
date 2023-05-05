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
        fs::canonical(dirname() / ".." / "shader" / "dashed-line.vert").c_str(),
        fs::canonical(dirname() / ".." / "shader" / "dashed-line.frag").c_str(),
    });
}

static void set_viewport(GLFWwindow* window)
{
    int width{}, height{};
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    const float w = width, h = height;
    proj_matrix = glm::perspective(glm::radians(90.0f), w/h, 0.1f, 10.0f);
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

                int width{}, height{};
                glfwGetFramebufferSize(window, &width, &height);

                const GLint loc_dash = glGetUniformLocation(program, "u_dashSize");
                const GLint loc_gap  = glGetUniformLocation(program, "u_gapSize");
                const GLint loc_res  = glGetUniformLocation(program, "u_resolution");

                glUniform1f(loc_dash, 10.0f);
                glUniform1f(loc_gap, 10.0f);
                glUniform2f(loc_res, width, height);
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "19-dashed-line", nullptr, nullptr);
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

    // https://stackoverflow.com/questions/52928678/dashed-line-in-opengl3

    const GLint loc_mvp  = glGetUniformLocation(program, "u_mvp");
    const GLint loc_dash = glGetUniformLocation(program, "u_dashSize");
    const GLint loc_gap  = glGetUniformLocation(program, "u_gapSize");

    glUniform1f(loc_dash, 10.0f);
    glUniform1f(loc_gap, 10.0f);

    const std::vector<GLfloat> varray{
        -1, -1, -1,   1, -1, -1,   1, 1, -1,   -1, 1, -1,
        -1, -1,  1,   1, -1,  1,   1, 1,  1,   -1, 1,  1
    };
    const std::vector<GLuint> iarray{
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    GLuint bo[2]{}, vao{};
    glGenBuffers(2, bo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, bo[0]);
    glBufferData(GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iarray.size()*sizeof(*iarray.data()), iarray.data(), GL_STATIC_DRAW);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    set_viewport(window);
    while (!glfwWindowShouldClose(window)) {
        static float angle{1.0f};
        glm::mat4 mv_matrix{1.0f};
        mv_matrix = glm::translate(mv_matrix, glm::vec3{0.0f, 0.0f, -3.0f});
        mv_matrix = glm::rotate(mv_matrix, glm::radians(angle), glm::vec3{1.0f, 0.0f, 0.0f});
        mv_matrix = glm::rotate(mv_matrix, glm::radians(angle/2), glm::vec3{0.0f, 1.0f, 0.0f});
        angle += 0.5f;

        const glm::mat4 mvp_matrix = proj_matrix * mv_matrix;
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp_matrix));

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_LINES, (GLsizei)iarray.size(), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    fmt::print("Bye.\n");
    return 0;
}
