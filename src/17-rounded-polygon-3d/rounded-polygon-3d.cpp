#include "glad.h"
#include <array>
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
static bool wireframe{true};
static glm::vec2 rotate{20.0f, -30.0f};

static std::string window_title()
{
    return fmt::format("17-rounded-polygon-3d (rx={:2.1f}, ry={:2.1f})", rotate.x, rotate.y);
}

static GLuint compile_shaders()
{
    namespace fs = std::filesystem;
    return compile_shaders({
        fs::canonical(dirname() / ".." / "shader" / "mvp3d-color.vert").c_str(),
        fs::canonical(dirname() / ".." / "shader" / "basic.frag").c_str(),
    });
}

static void set_callbacks(GLFWwindow* window)
{
    static double x{}, y{};

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
            else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
                wireframe = !wireframe;
                glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
            }
            else if (key == GLFW_KEY_HOME && action == GLFW_PRESS) {
                rotate.x = rotate.y = 0;
                glfwSetWindowTitle(window, window_title().c_str());
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
                x = xpos;
                y = ypos;
            }
            else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                fmt::print("mouse up {}, {}\n", xpos, ypos);
                rotate.x += (ypos - y) / 3;
                rotate.y += (xpos - x) / 3;
                rotate.x = std::fmod(rotate.x, 360.0f);
                rotate.y = std::fmod(rotate.y, 360.0f);
                glfwSetWindowTitle(window, window_title().c_str());
                x = xpos;
                y = ypos;
            }
        }
    );
    glfwSetCursorPosCallback(
        window,
        [](GLFWwindow* window, double xpos, double ypos) {
            const int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if (state == GLFW_PRESS) {
                rotate.x += (ypos - y) / 3;
                rotate.y += (xpos - x) / 3;
                rotate.x = std::fmod(rotate.x, 360.0f);
                rotate.y = std::fmod(rotate.y, 360.0f);
                glfwSetWindowTitle(window, window_title().c_str());
                x = xpos;
                y = ypos;
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

    fmt::print("Press <spacebar> to toggle filled and wireframe mode.\n");
    fmt::print("Press and hold left mouse button and then move mouse to rotate the cube.\n");
    fmt::print("Press <home> to rotate the cube to the home position.\n");
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
    // Build model matrix. Order of rotation must be Y followed by X.
    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    glm::mat4 model_matrix{1.0f};
    model_matrix = glm::rotate(model_matrix, glm::radians(rotate.x), glm::vec3{1.0f, 0.0f, 0.0f});
    model_matrix = glm::rotate(model_matrix, glm::radians(rotate.y), glm::vec3{0.0f, 1.0f, 0.0f});

    // Build view matrix
    const glm::vec3 camera{0.0f, 0.0f, 3.0f};
    const glm::vec3 center{0.0f, 0.0f, 0.0f};
    const glm::vec3 up{0.0f, 1.0f, 0.0f};
    const glm::mat4 view_matrix = glm::lookAt(camera, center, up);

    // Build model-view matrix
    const glm::mat4 mv_matrix = view_matrix * model_matrix;

    // Build projection matrix
    constexpr float fovy = glm::radians(60.0f);
    int width{}, height{};
    glfwGetFramebufferSize(window, &width, &height);
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    const glm::mat4 proj_matrix = glm::perspective(fovy, aspect, 0.1f, 1000.0f);

    // Copy model-view and projection matrices to uniform variables
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mv_matrix));
    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(proj_matrix));

    // Clear color buffer and depth buffer
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Set the color of our polygon to gold
    glUniform3f(2, 0.82f, 0.65f, 0.17f);

    // Draw rounded polygon
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    // Draw black point
    glUniform3f(2, 0.0f, 0.0f, 0.0f);
    glPointSize(8);
    glDrawArrays(GL_POINTS, 1, 1);
}

/**
 * Generates a pie.
 * `x` specifies the x coordinate of the center of the pie.
 * `y` specifies the y coordinate of the center of the pie.
 * `radius` specifies the radius of the pie.
 * `start` specifies the starting angle in radians.
 * `end` specifies the ending angle in radians.
 * `triangles` specifies the number of triangles that make up the pie. Must be >= 1.
 * Returns a vector of vertices. The number of vertices returned is `triangles` * 3.
 */
static std::vector<glm::vec3> gen_pie(
    float x, float y, float radius, float start, float end, int triangles)
{
    const float angle = (end - start) / triangles;

    std::vector<glm::vec3> vertices;
    vertices.reserve(triangles * 3);

    for (int i{}; i < triangles; i++) {
        vertices.emplace_back(glm::vec3{x, y, 0.0f}); // center vertex
        vertices.emplace_back(glm::vec3{
            x + radius * std::cos(i * angle + start),
            y + radius * std::sin(i * angle + start),
            0.0f
        });
        vertices.emplace_back(glm::vec3{
            x + radius * std::cos((i+1) * angle + start),
            y + radius * std::sin((i+1) * angle + start),
            0.0f
        });
    }

    return vertices;
}

/**
 * Generates a rectangle that lies on the external side of a regular polygon.
 * `n` specifies the number of sides of the regular polygon. Must be >=3.
 * `ri` specifies the circumradius of the regular polygon.
 * `rc` specifies the radius of the corners.
 * `angle` specifies the rotation angle in radians.
 *     For the bottom rectangle, the angle is zero.
 * Returns a vector of 6 vertices.
 */
static std::vector<glm::vec3> gen_rect(int n, float ri, float rc, float angle)
{
    // Find the side length and apothem of a regular polygon.
    // https://en.wikipedia.org/wiki/Regular_polygon#Circumradius
    const float side = ri * 2 * std::sin(glm::pi<float>() / n);
    const float apothem = ri * std::cos(glm::pi<float>() / n);

    // Calculate dimensions of rectangle
    const float w = side / 2; // half width
    const float h = rc / 2;   // half height

    // Build transformation matrix
    glm::mat4 tm{1.0f};
    tm = glm::rotate(tm, angle, glm::vec3{0.0, 0.0f, 1.0f});
    tm = glm::translate(tm, glm::vec3{0.0f, -(apothem + rc / 2), 0.0f});

    // Return vertices of rectangle
    return {
        tm * glm::vec4{-w, +h, 0.0f, 1.0f}, // top left vertex
        tm * glm::vec4{-w, -h, 0.0f, 1.0f}, // bottom left vertex
        tm * glm::vec4{+w, -h, 0.0f, 1.0f}, // bottom right vertex
        tm * glm::vec4{-w, +h, 0.0f, 1.0f}, // top left vertex
        tm * glm::vec4{+w, -h, 0.0f, 1.0f}, // bottom right vertex
        tm * glm::vec4{+w, +h, 0.0f, 1.0f}, // top right vertex
    };
}

/**
 * Generates a rounded polygon centered at the origin.
 * `n` specifies the number of sides of the regular polygon. Must be >=3.
 * `ri` specifies the circumradius of the regular polygon.
 * `rc` specifies the radius of the corners.
 */
static std::vector<glm::vec3> gen_polygon(int n, float ri, float rc)
{
    const float first = glm::radians(n % 2 ? 90.0f : 90.0f - 180.0f / n);
    const float angle = glm::two_pi<float>() / n;

    std::vector<glm::vec3> vertices;
    vertices.reserve(3*n + 6*n + 8*3*n);

    // Regular polygon
    for (int i = 0; i < n; i++) {
        vertices.emplace_back(glm::vec3{}); // origin

        float x{}, y{};

        x = ri * std::cos(i * angle + first);
        y = ri * std::sin(i * angle + first);
        vertices.emplace_back(glm::vec3{x, y, 0.0f});

        x = ri * std::cos((i+1) * angle + first);
        y = ri * std::sin((i+1) * angle + first);
        vertices.emplace_back(glm::vec3{x, y, 0.0f});
    }

    // Rectangles
    for (int i = 0; i < n; i++) {
        const auto v = gen_rect(n, ri, rc, i * angle);
        vertices.insert(vertices.end(), v.begin(), v.end());
    }

    // Pies (rounded corners)
    for (int i = 0; i < n; i++) {
        const float a = i * angle + first;
        const float x = ri * std::cos(a);
        const float y = ri * std::sin(a);
        const auto v = gen_pie(x, y, rc, a - angle/2, a + angle/2, 8);
        vertices.insert(vertices.end(), v.begin(), v.end());
    }

    return vertices;
}

/**
 * Generates a rounded polygon centered at the origin.
 * `n` specifies the number of sides of the regular polygon. Must be >=3.
 * `ri` specifies the circumradius of the regular polygon.
 * `rc` specifies the radius of the corners.
 */
static std::vector<glm::vec3> gen_polygon_3d(int n, float ri, float rc)
{
    glm::mat4 tm;

    // Front face
    tm = glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, 0.0f, 1.0f});
    std::vector<glm::vec3> front = gen_polygon(n, ri, rc);
    for (auto& v : front) {
        v = tm * glm::vec4{v, 1.0f};
    }

    // Back face
    tm = glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, 0.0f, -1.0f});
    //tm = glm::rotate(tm, glm::pi<float>(), glm::vec3{0.0f, 1.0f, 0.0f});
    std::vector<glm::vec3> back = gen_polygon(n, ri, rc);
    for (auto& v : back) {
        v = tm * glm::vec4{v, 1.0f};
    }

    std::vector<glm::vec3> vertices{front};
    vertices.insert(vertices.end(), back.begin(), back.end());

    // Rectangular faces
    for (int i = 0; i < n; i++) {
        vertices.emplace_back(front[3*n + 6*i + 1]);
        vertices.emplace_back(front[3*n + 6*i + 2]);
        vertices.emplace_back(back[3*n + 6*i + 2]);

        vertices.emplace_back(front[3*n + 6*i + 1]);
        vertices.emplace_back(back[3*n + 6*i + 2]);
        vertices.emplace_back(back[3*n + 6*i + 1]);
    }

    // Rounded faces
    // for (int i = 0; i < n; i++) {
    //     for (int j = 0; j < 8; j++) {
    //         vertices.emplace_back(front[3*n + 6*n + 3*j*i + 2]);
    //         vertices.emplace_back(front [3*n + 6*n + 3*j*i + 1]);
    //         vertices.emplace_back(back [3*n + 6*n + 3*j*i + 1]);

    //         vertices.emplace_back(front[3*n + 6*n + 3*j*i + 2]);
    //         vertices.emplace_back(back [3*n + 6*n + 3*j*i + 1]);
    //         vertices.emplace_back(back [3*n + 6*n + 3*j*i + 2]);
    //     }
    // }

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
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(800, 600, window_title().c_str(), nullptr, nullptr);
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

    // Generate the vertices of our rounded polygon
    const std::vector<glm::vec3> vertices = gen_polygon_3d(4, 0.8f, 0.2f);

    // Create and populate interleaved vertex buffer using
    // DSA (Direct State Access) API in OpenGL 4.5.
    GLuint vbo{};
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(glm::vec3) * vertices.size(), vertices.data(), 0);

    // Create VAO
    GLuint vao{};
    glCreateVertexArrays(1, &vao);

    // Bind the vertex buffer to the VAO's vertex buffer binding point
    const GLuint binding_index{0}; // [0..GL_MAX_VERTEX_ATTRIB_BINDINGS)
    glVertexArrayVertexBuffer(vao, binding_index, vbo, 0, sizeof(glm::vec3));

    // Enable vertex attribute location 0
    glEnableVertexArrayAttrib(vao, 0);

    // Specify the data format for each vertex attribute location
    glVertexArrayAttribFormat(vao, 0, glm::vec3::length(), GL_FLOAT, GL_FALSE, 0);

    // Tell OpenGL to read the data for vertex attribute location 0
    // from the buffer, which is attached to vertex buffer binding point 0.
    glVertexArrayAttribBinding(vao, 0, binding_index);

    // This shows that we do not have to bind the VAO before
    // calling the above functions.
    glBindVertexArray(vao);

    // Draw filled or wireframe polygons
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

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
