#include "glad.h"
#include <filesystem>
#include <fmt/core.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "shader.h"
#include "utils.h"

// Global variables
static GLuint program{};
static GLFWcursor* crosshair_cursor{};
static glm::mat4 view_matrix{};
static glm::mat4 proj_matrix{};
static float scaling{1.0f};
static float rotation{0.0f};
static glm::vec2 translation{0.0f, 0.0f};
static bool moving{};
static bool rotating{};
static bool selected{};

static std::string window_title()
{
    const float degrees = std::fmod(glm::degrees(rotation) + 360.0f, 360.0f);
    return fmt::format("17-triangle-test (rz={:2.1f})", degrees);
}

static GLuint create_program()
{
    namespace fs = std::filesystem;
    return compile_shaders({
        fs::canonical(dirname() / ".." / "shader" / "triangle-test.vert").c_str(),
        fs::canonical(dirname() / ".." / "shader" / "basic.frag").c_str(),
    });
}

// Unproject window coordinates to world coordinates
static glm::vec3 window_to_world(
    GLFWwindow* window,
    glm::vec2 win)
{
    // https://en.wikibooks.org/wiki/OpenGL_Programming/Object_selection#Unprojecting_window_coordinates
    int width{}, height{};
    glfwGetFramebufferSize(window, &width, &height);
    const glm::vec3 window_coords{win.x, height - win.y - 1, 0.0f};
    const glm::vec4 viewport{0, 0, width, height};
    const glm::vec3 world_coords = glm::unProject(window_coords, view_matrix, proj_matrix, viewport);
    return world_coords;
}

static float sign(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

static bool point_in_triangle(
    glm::vec2 p,
    glm::vec2 p1,
    glm::vec2 p2,
    glm::vec2 p3)
{
    glm::mat4 model_matrix{1.0f};
    model_matrix = glm::translate(model_matrix, glm::vec3{translation, 0.0f});
    model_matrix = glm::rotate(model_matrix, rotation, glm::vec3{0.0f, 0.0f, 1.0f});
    model_matrix = glm::scale(model_matrix, glm::vec3{scaling, scaling, 0.0f});

    p1 = model_matrix * glm::vec4{p1, 0.0f, 1.0f};
    p2 = model_matrix * glm::vec4{p2, 0.0f, 1.0f};
    p3 = model_matrix * glm::vec4{p3, 0.0f, 1.0f};

    const float d1 = sign(p, p1, p2);
    const float d2 = sign(p, p2, p3);
    const float d3 = sign(p, p3, p1);

    const bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    const bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

static void set_callbacks(GLFWwindow* window)
{
    static glm::vec2 trans{};
    static float rot{};

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
            else if (key == GLFW_KEY_HOME && action == GLFW_PRESS) {
                scaling = 1.0f;
                rotation = 0.0f;
                translation.x = translation.y = 0.0f;
                moving = rotating = false;
                glfwSetCursor(window, nullptr);
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
                const glm::vec2 world = window_to_world(window, glm::vec2{xpos, ypos});
                selected = point_in_triangle(world,
                    glm::vec2{-0.5f, -0.5f}, glm::vec2{0.5f, -0.5f}, glm::vec2{0.0f, 0.5f});
                if (selected) {
                    moving = true;
                    trans = world - translation;
                }
            }
            else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                moving = false;
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
                if (selected) {
                    rotating = true;
                    const glm::vec2 origin{translation};
                    const glm::vec2 world = window_to_world(window, glm::vec2{xpos, ypos});
                    const glm::vec2 a{1.0f, 0.0f};
                    const glm::vec2 b = glm::normalize(world - origin);
                    const float r = glm::orientedAngle(a, b);
                    rot = r - rotation;
                }
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
                rotating = false;
            }

            glfwSetCursor(window, moving || rotating ? crosshair_cursor : nullptr);
        }
    );
    glfwSetCursorPosCallback(
        window,
        [](GLFWwindow* window, double xpos, double ypos) {
            if (moving) {
                const glm::vec2 world = window_to_world(window, glm::vec2{xpos, ypos});
                translation = world - trans;
            }
            else if (rotating) {
                const glm::vec2 origin{translation};
                const glm::vec2 world = window_to_world(window, glm::vec2{xpos, ypos});
                const glm::vec2 a{1.0f, 0.0f};
                const glm::vec2 b = glm::normalize(world - origin);
                const float r = glm::orientedAngle(a, b);
                rotation = r - rot;
                glfwSetWindowTitle(window, window_title().c_str());
            }
        }
    );
    glfwSetScrollCallback(
        window,
        [](GLFWwindow* window, double xoffset, double yoffset) {
            if (selected) {
                scaling += -yoffset * 0.05f;
                scaling = glm::clamp(scaling, 0.3f, 3.0f);
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

    fmt::print("Press the left mouse button inside/outside the triangle to select/unselect.\n");
    fmt::print("To scale, scroll the mouse wheel.\n");
    fmt::print("To rotate, press and hold the right mouse button anywhere and then move the mouse.\n");
    fmt::print("To translate, press and hold the left mouse button inside the triangle and then move the mouse.\n");
    fmt::print("Press 'home' to return the triangle to the default size, rotation and position.\n");
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

static void render(GLFWwindow* window, double current_time)
{
    // Build model matrix
    glm::mat4 model_matrix{1.0f};
    model_matrix = glm::translate(model_matrix, glm::vec3{translation, 0.0f});
    model_matrix = glm::rotate(model_matrix, rotation, glm::vec3{0.0f, 0.0f, 1.0f});
    model_matrix = glm::scale(model_matrix, glm::vec3{scaling, scaling, 0.0f});

    // Build view matrix
    const glm::vec3 camera{0.0f, 0.0f, 5.0f};
    const glm::vec3 center{0.0f, 0.0f, 0.0f};
    const glm::vec3 up{0.0f, 1.0f, 0.0f};
    view_matrix = glm::lookAt(camera, center, up);

    // Build model-view matrix
    const glm::mat4 mv_matrix = view_matrix * model_matrix;

    // Build orthographic projection matrix
    int width{}, height{};
    glfwGetFramebufferSize(window, &width, &height);
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    proj_matrix = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -10.0f, 10.0f);

    // Copy model-view and projection matrices to uniform variables
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mv_matrix));
    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(proj_matrix));

    // Set the background color
    const GLfloat background[]{0.2f, 0.2f, 0.2f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, background);

    // Draw triangle
    glUniform1i(2, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (selected) {
        // Draw wireframe
        glUniform1i(2, 1);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, 3);
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

    GLFWwindow* window = glfwCreateWindow(600, 600, window_title().c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1); // vsync on

    print_info();
    crosshair_cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    set_callbacks(window);

    program = create_program();
    glUseProgram(program);

    // Define the vertices of our triangle.
    // Note that the winding order is counter-clockwise.
    const GLfloat vertices[]{
        // position         color
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
         0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    // Create and populate interleaved vertex buffer using
    // DSA (Direct State Access) API in OpenGL 4.5.
    GLuint vbo{};
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);

    // Create VAO
    GLuint vao{};
    glCreateVertexArrays(1, &vao);

    // Bind the vertex buffer to the VAO's vertex buffer binding point
    const GLuint binding_index{0}; // [0..GL_MAX_VERTEX_ATTRIB_BINDINGS)
    glVertexArrayVertexBuffer(vao, binding_index, vbo, 0, sizeof(GLfloat)*6);

    // Enable vertex attribute locations 0 and 1
    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);

    // Specify the data format for each vertex attribute location
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3);

    // Tell OpenGL to read the data for vertex attribute locations 0 and 1
    // from the buffer, which is attached to vertex buffer binding point 0.
    glVertexArrayAttribBinding(vao, 0, binding_index);
    glVertexArrayAttribBinding(vao, 1, binding_index);

    // This shows that we do not have to bind the VAO before
    // calling the above functions.
    glBindVertexArray(vao);

    while (!glfwWindowShouldClose(window)) {
        process_gamepad(window);
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
