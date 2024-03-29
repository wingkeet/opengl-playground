#include "glad.h"
#include <cmath>
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
static int first_color_index{};

static GLuint create_program()
{
    namespace fs = std::filesystem;
    return compile_shaders({
        fs::canonical(dirname() / ".." / "shader" / "dots-instancing.vert").c_str(),
        fs::canonical(dirname() / ".." / "shader" / "basic.frag").c_str(),
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
                program = create_program();
                glUseProgram(program);
            }
        }
    );
    glfwSetMouseButtonCallback(
        window,
        [](GLFWwindow* window, int button, int action, int mods) {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                first_color_index = (first_color_index + 1) % 10;
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
                first_color_index = (first_color_index + 9) % 10;
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

    fmt::print("Press left and right mouse buttons to rotate colors.\n");
}

static void render(GLFWwindow* window, double current_time, int num_vertices)
{
    // Build view matrix
    const glm::vec3 camera{0.0f, 0.0f, 5.0f};
    const glm::vec3 center{0.0f, 0.0f, 0.0f};
    const glm::vec3 up{0.0f, 1.0f, 0.0f};
    const glm::mat4 view_matrix = glm::lookAt(camera, center, up);

    // Build orthographic projection matrix
    int width{}, height{};
    glfwGetFramebufferSize(window, &width, &height);
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    const glm::mat4 proj_matrix = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -10.0f, 10.0f);

    // Build scale matrix
    const float tf = static_cast<float>(current_time);
    const float sf = 0.045f * std::sin(tf * 2) + 0.055f; // [0.01..0.1]
    const glm::mat4 scale_matrix = glm::scale(glm::mat4{1.0f}, glm::vec3{sf, sf, 1.0f});

    // Get uniform locations
    const GLint loc_view_matrix = glGetUniformLocation(program, "u_view_matrix");
    const GLint loc_proj_matrix = glGetUniformLocation(program, "u_proj_matrix");
    const GLint loc_scale_matrix = glGetUniformLocation(program, "u_scale_matrix");

    // Copy to uniform variables
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(loc_proj_matrix, 1, GL_FALSE, glm::value_ptr(proj_matrix));
    glUniformMatrix4fv(loc_scale_matrix, 1, GL_FALSE, glm::value_ptr(scale_matrix));

    // Selected CSS colors - https://www.w3schools.com/cssref/css_colors.php
    static const glm::vec3 colors[10]{
        {1.0f, 0.0f, 0.0f},                   // red
        {0.0f, 1.0f, 0.0f},                   // green
        {0.0f, 0.0f, 1.0f},                   // blue
        {1.0f, 215.0f/255, 0.0f},             // gold
        {0.5f, 0.5f, 0.5f},                   // medium gray
        {128.0f/255, 128.0f/255, 0.0f},       // olive
        {100.0f/255, 149.0f/255, 237.0f/255}, // cornflower blue
        {1.0f, 105.0f/255, 180.0f/255},       // hot pink
        {138.0f/255, 43.0f/255, 226.0f/255},  // blue violet
        {1.0f, 1.0f, 1.0f},                   // white
    };
    for (int i{}; i < 10; i++) {
        const std::string str = fmt::format("u_colors[{}]", i);
        const GLint loc = glGetUniformLocation(program, str.c_str());
        glUniform3fv(loc, 1, glm::value_ptr(colors[(first_color_index + i) % 10]));
    }

    // Draw 60 dots with instancing
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_vertices, 60); // 6 rows by 10 columns
}

// https://stackoverflow.com/questions/59468388/how-to-use-gl-triangle-fan-to-draw-a-circle-in-opengl
static std::vector<glm::vec2> gen_circle(int num_vertices)
{
    const float angle{glm::two_pi<float>() / num_vertices};
    std::vector<glm::vec2> vertices;
    vertices.reserve(num_vertices);

    // We don't need a center point. Since a circle is a convex shape,
    // we can simply use one of the points on the circle as the central
    // vertex of our triangle fan.
    for (int i{}; i < num_vertices; i++) {
        vertices.emplace_back(glm::vec2{std::cos(angle * i), std::sin(angle * i)});
    }

    return vertices;
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "21-dots-instancing", nullptr, nullptr);
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

    // Generate the vertices of our circle
    const std::vector<glm::vec2> vertices = gen_circle(30);

    // Create and populate interleaved vertex buffer using
    // DSA (Direct State Access) API in OpenGL 4.5.
    GLuint vbo{};
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(glm::vec2) * vertices.size(), vertices.data(), 0);

    // Create VAO
    GLuint vao{};
    glCreateVertexArrays(1, &vao);

    // Bind the vertex buffer to the VAO's vertex buffer binding point
    const GLuint binding_index{0}; // [0..GL_MAX_VERTEX_ATTRIB_BINDINGS)
    glVertexArrayVertexBuffer(vao, binding_index, vbo, 0, sizeof(glm::vec2));

    // Enable vertex attribute location 0
    glEnableVertexArrayAttrib(vao, 0);

    // Specify the data format for each vertex attribute location
    glVertexArrayAttribFormat(vao, 0, glm::vec2::length(), GL_FLOAT, GL_FALSE, 0);

    // Tell OpenGL to read the data for vertex attribute location 0
    // from the buffer, which is attached to vertex buffer binding point 0.
    glVertexArrayAttribBinding(vao, 0, binding_index);

    // This shows that we do not have to bind the VAO before
    // calling the above functions.
    glBindVertexArray(vao);

    while (!glfwWindowShouldClose(window)) {
        render(window, glfwGetTime(), vertices.size());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Shutting down from here onwards
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();

    fmt::print("Bye.\n");
    return 0;
}
