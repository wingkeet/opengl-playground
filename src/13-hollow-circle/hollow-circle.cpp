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
        fs::canonical(dirname() / ".." / "shader" / "mvp-color.vert").c_str(),
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
    // Build model matrix
    const glm::mat4 identity_matrix{1.0f};
    const float tf = static_cast<float>(current_time);
    const glm::mat4 model_matrix{identity_matrix};

    // Build view matrix
    const glm::vec3 camera{0.0f, 0.0f, 5.0f};
    const glm::vec3 center{0.0f, 0.0f, 0.0f};
    const glm::vec3 up{0.0f, 1.0f, 0.0f};
    const glm::mat4 view_matrix = glm::lookAt(camera, center, up);

    // Build model-view matrix
    const glm::mat4 mv_matrix = view_matrix * model_matrix;

    // Build orthographic projection matrix
    int width{}, height{};
    glfwGetFramebufferSize(window, &width, &height);
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    const glm::mat4 proj_matrix = glm::ortho(
        -1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -1000.0f, 1000.0f);

    // Copy model-view and projection matrices to uniform variables
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mv_matrix));
    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(proj_matrix));

    // Set the background color
    const GLfloat background[]{0.2f, 0.2f, 0.2f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, background);

    // Set the color of our circle
    glUniform3f(2, 0.58f, 0.29f, 0.0f);

    // Draw hollow circle
    glDrawArrays(GL_TRIANGLE_STRIP, 0, num_vertices);
}

/**
 * Generates a hollow circle that is meant to be drawn using a triangle strip.
 * `radius` specifies the radius of the outer edge of the circle.
 * `width` specifies the width or thickness of the filled portion of the circle.
 *     The inner edge of the circle is given by `radius` - `width`.
 * `triangles` specifies the number of triangles in the triangle strip.
 *     Should be an even number, or a gap would appear.
 * Returns a vector of 2d vertices. The number of vertices returned is always `triangles` + 2.
 */
std::vector<glm::vec2> gen_hollow_circle(float radius, float width, int triangles)
{
    const float w{width / 2}; // half width
    const float angle{glm::two_pi<float>() / triangles};
    std::vector<glm::vec2> vertices;
    vertices.reserve(triangles + 2);

    // Radius of the center of the filled portion of the circle
    radius -= w;

    // Generate alternating vertices on the inner edge and the outer edge.
    // `edge` toggles between -1 (inner edge) and +1 (outer edge).
    for (int i{}, edge{-1}; i < triangles+2; i++, edge*=-1) {
        vertices.emplace_back(glm::vec2{
            (radius + w * edge) * std::cos(angle * i),
            (radius + w * edge) * std::sin(angle * i)
        });
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

    GLFWwindow* window = glfwCreateWindow(600, 600, "13-hollow-circle", nullptr, nullptr);
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
    const std::vector<glm::vec2> vertices = gen_hollow_circle(1.0f, 0.5f, 90);

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

    // Uncomment this call to draw in wireframe polygons
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window)) {
        process_gamepad(window);
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
