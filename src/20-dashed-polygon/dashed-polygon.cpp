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
static glm::mat4 wndmat{};

static GLuint create_program()
{
    namespace fs = std::filesystem;
    return compile_shaders({
        fs::canonical(dirname() / ".." / "shader" / "dashed-polygon.vert").c_str(),
        fs::canonical(dirname() / ".." / "shader" / "dashed-polygon.frag").c_str(),
    });
}

static void reload_program(GLFWwindow* window)
{
    glDeleteProgram(program);
    program = create_program();
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

static void set_viewport(GLFWwindow* window)
{
    int width{}, height{};
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    const float w = width, h = height;
    proj_matrix = glm::perspective(glm::radians(90.0f), w/h, 0.1f, 10.0f);
    const GLint loc_res = glGetUniformLocation(program, "u_resolution");
    glUniform2f(loc_res, w, h);
    wndmat = glm::scale(glm::mat4{1.0f}, glm::vec3{w/2, h/2, 1.0f});
    wndmat = glm::translate(wndmat, glm::vec3{1.0f, 1.0f, 0.0f});
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
                reload_program(window);
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "20-dashed-polygon", nullptr, nullptr);
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

    // https://stackoverflow.com/questions/52928678/dashed-line-in-opengl3

    const GLint loc_mvp  = glGetUniformLocation(program, "u_mvp");
    const GLint loc_dash = glGetUniformLocation(program, "u_dashSize");
    const GLint loc_gap  = glGetUniformLocation(program, "u_gapSize");

    glUniform1f(loc_dash, 10.0f);
    glUniform1f(loc_gap, 10.0f);

    std::vector<glm::vec3> varray;
    for (int u{}; u <= 360; ++u) {
        const float a = glm::radians(static_cast<float>(u));
        const float c = std::cos(a), s = std::sin(a);
        varray.emplace_back(glm::vec3{c, s, 0.0f});
    }
    std::vector<float> darray(varray.size(), 0.0f);

    GLuint bo[2]{}, vao{};
    glGenBuffers(2, bo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, bo[0]);
    glBufferData(GL_ARRAY_BUFFER, varray.size()*sizeof(*varray.data()), varray.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, bo[1] );
    glBufferData(GL_ARRAY_BUFFER, darray.size()*sizeof(*darray.data()), darray.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    set_viewport(window);
    while (!glfwWindowShouldClose(window)) {
        static float angle{1.0f};
        glm::mat4 mv_matrix{1.0f};
        mv_matrix = glm::translate(mv_matrix, glm::vec3{0.0f, 0.0f, -2.0f});
        mv_matrix = glm::rotate(mv_matrix, glm::radians(angle), glm::vec3{1.0f, 0.0f, 0.0f});
        mv_matrix = glm::rotate(mv_matrix, glm::radians(angle/2), glm::vec3{0.0f, 1.0f, 0.0f});
        angle += 0.5f;

        const glm::mat4 mvp_matrix = proj_matrix * mv_matrix;
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp_matrix));

        glm::vec2 vpPt{0.0f, 0.0f};
        float dist{0.0f};
        for (size_t i{}; i < varray.size(); ++i) {
            darray[i] = dist;
            const glm::vec4 clip = mvp_matrix * glm::vec4{varray[i], 1.0f};
            const glm::vec4 ndc  = clip / clip.w;
            const glm::vec4 vpC  = wndmat * ndc;
            const float len = i==0 ? 0.0f : glm::length(vpPt - glm::vec2{vpC});
            vpPt = glm::vec2(vpC);
            dist += len;
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, darray.size()*sizeof(*darray.data()), darray.data());

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)varray.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    fmt::print("Bye.\n");
    return 0;
}
