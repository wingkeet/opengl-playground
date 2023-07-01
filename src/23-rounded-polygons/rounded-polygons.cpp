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
static bool wireframe{};
static std::vector<glm::vec2> all;
static std::vector<int> count;
static std::vector<int> first;

static GLuint create_program()
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
                program = create_program();
                glUseProgram(program);
            }
            else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
                wireframe = !wireframe;
                glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
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

    fmt::print("Press spacebar to toggle filled and wireframe mode.\n");
}

static void render(GLFWwindow* window, double current_time)
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

    // Copy model-view and projection matrices to uniform variables
    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(proj_matrix));

    // Set the background color
    const GLfloat background[]{0.2f, 0.2f, 0.2f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, background);

    // Set the color of our polygon to gold
    glUniform3f(2, 0.82f, 0.65f, 0.17f);

    // Draw polygons
    for (int n{}; n < 12; n++) {
        const float tx = n % 4 * 0.6f - 0.9f;
        const float ty = -n / 4 * 0.6f + 0.6f;
        glm::mat4 model_matrix{1.0f};
        model_matrix = glm::translate(model_matrix, glm::vec3{tx, ty, 0.0f});
        if (n % 2) {
            const float rotation = glm::pi<float>() / (n+3);
            model_matrix = glm::rotate(model_matrix, rotation, glm::vec3{0.0f, 0.0f, 1.0f});
        }
        model_matrix = glm::scale(model_matrix, glm::vec3{0.25f, 0.25f, 1.0f});

        const glm::mat4 mv_matrix = view_matrix * model_matrix;
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mv_matrix));

        glDrawArrays(GL_TRIANGLES, first[n], count[n]);
    }
}

/**
 * Generates a pie.
 * `x` specifies the x coordinate of the center of the pie.
 * `y` specifies the y coordinate of the center of the pie.
 * `radius` specifies the radius of the pie.
 * `start` specifies the starting angle in radians.
 * `end` specifies the ending angle in radians.
 * `triangles` specifies the number of triangles that make up the pie. Must be >= 1.
 * Returns a vector of 2d vertices. The number of vertices returned is `triangles` * 3.
 */
static std::vector<glm::vec2> gen_pie(
    float x, float y, float radius, float start, float end, int triangles)
{
    const float angle = (end - start) / triangles;

    std::vector<glm::vec2> vertices;
    vertices.reserve(triangles * 3);

    for (int i{}; i < triangles; i++) {
        vertices.emplace_back(glm::vec2{x, y}); // center vertex
        vertices.emplace_back(glm::vec2{
            x + radius * std::cos(i * angle + start),
            y + radius * std::sin(i * angle + start)
        });
        vertices.emplace_back(glm::vec2{
            x + radius * std::cos((i+1) * angle + start),
            y + radius * std::sin((i+1) * angle + start)
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
 * Returns a vector of six 2d vertices.
 */
static std::vector<glm::vec2> gen_rect(int n, float ri, float rc, float angle)
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
static std::vector<glm::vec2> gen_polygon(int n, float ri, float rc)
{
    const float first = glm::radians(n % 2 ? 90.0f : 90.0f - 180.0f / n);
    const float angle = glm::two_pi<float>() / n;

    std::vector<glm::vec2> vertices;
    vertices.reserve(3*n + 6*n + 8*3*n);

    // Regular polygon
    for (int i = 0; i < n; i++) {
        vertices.emplace_back(glm::vec2{}); // origin

        float x{}, y{};

        x = ri * std::cos(i * angle + first);
        y = ri * std::sin(i * angle + first);
        vertices.emplace_back(glm::vec2{x, y});

        x = ri * std::cos((i+1) * angle + first);
        y = ri * std::sin((i+1) * angle + first);
        vertices.emplace_back(glm::vec2{x, y});
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

static void gen_polygons()
{
    for (int n{3}; n <= 14; n++) {
        const std::vector<glm::vec2> v = gen_polygon(n, 0.8f, 0.2f);
        all.insert(all.end(), v.begin(), v.end());
    }

    int sum{};
    for (int n{3}; n <= 14; n++) {
        const int size = 3*n + 6*n + 8*3*n;
        count.emplace_back(size);
        first.emplace_back(sum);
        sum += size;
    }
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "23-rounded-polygons", nullptr, nullptr);
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

    // Generate the vertices of our rounded polygons
    gen_polygons();

    // Create and populate interleaved vertex buffer using
    // DSA (Direct State Access) API in OpenGL 4.5.
    GLuint vbo{};
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(glm::vec2) * all.size(), all.data(), 0);

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

    // Draw filled or wireframe polygons
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

    while (!glfwWindowShouldClose(window)) {
        render(window, glfwGetTime());
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
