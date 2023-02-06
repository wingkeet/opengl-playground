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

static GLuint compile_shaders()
{
    namespace fs = std::filesystem;
    return compile_shaders({
        fs::canonical(dirname() / ".." / "shader" / "color.vert").c_str(),
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
                program = compile_shaders();
                glUseProgram(program);
            }
        }
    );
    glfwSetMouseButtonCallback(
        window,
        [](GLFWwindow* window, int button, int action, int mods) {
            double xpos{}, ypos{};
            glfwGetCursorPos(window, &xpos, &ypos);
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                fmt::print("mouse down {}, {}\n", xpos, ypos);
            }
            else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                fmt::print("mouse up {}, {}\n", xpos, ypos);
            }
        }
    );
    glfwSetWindowFocusCallback(
        window,
        [](GLFWwindow* window, int focused) {
            //fmt::print("focused {}\n", focused);
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

    if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
        fmt::print("Gamepad: {}\n", glfwGetGamepadName(GLFW_JOYSTICK_1));
    }
    else {
        fmt::print("Gamepad: none\n");
    }
}

static void process_gamepad(GLFWwindow* window)
{
    GLFWgamepadstate state{};

    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
        if (state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

static void render(GLFWwindow* window, double current_time, int num_vertices)
{
    // Set the background color
    const GLfloat background[]{0.2f, 0.2f, 0.2f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, background);

    // Set the color of our circle
    glUniform3f(0, 1.0f, 0.0f, 0.65f);

    // Draw our first circle
    glDrawArrays(GL_TRIANGLE_FAN, 0, num_vertices);
}

// https://stackoverflow.com/questions/59468388/how-to-use-gl-triangle-fan-to-draw-a-circle-in-opengl
std::vector<float> gen_circle(int num_vertices)
{
    const float angle = 2 * M_PI / num_vertices;
    std::vector<float> vertices;
    vertices.reserve(num_vertices * 3);

    for (int i{}; i < num_vertices; i++) {
        vertices.push_back(std::cos(angle * i));
        vertices.push_back(std::sin(angle * i));
        vertices.push_back(0.0f);
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

    GLFWwindow* window = glfwCreateWindow(600, 600, "09-circle", nullptr, nullptr);
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

    // Generate the vertices of our circle
    const std::vector<float> vertices = gen_circle(50);

    // Create and populate interleaved vertex buffer using
    // DSA (Direct State Access) API in OpenGL 4.5.
    GLuint vbo{};
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(GLfloat) * vertices.size(), vertices.data(), 0);

    // Create VAO
    GLuint vao{};
    glCreateVertexArrays(1, &vao);

    // Bind the vertex buffer to the VAO's vertex buffer binding point
    const GLuint binding_index{0}; // [0..GL_MAX_VERTEX_ATTRIB_BINDINGS)
    glVertexArrayVertexBuffer(vao, binding_index, vbo, 0, sizeof(GLfloat)*3);

    // Enable vertex attribute location 0
    glEnableVertexArrayAttrib(vao, 0);

    // Specify the data format for each vertex attribute location
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

    // Tell OpenGL to read the data for vertex attribute location 0
    // from the buffer, which is attached to vertex buffer binding point 0.
    glVertexArrayAttribBinding(vao, 0, binding_index);

    // This shows that we do not have to bind the VAO before
    // calling the above functions.
    glBindVertexArray(vao);

    // Uncomment this call to draw in wireframe polygons
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window)) {
        process_gamepad(window);
        render(window, glfwGetTime(), vertices.size() / 3);
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
