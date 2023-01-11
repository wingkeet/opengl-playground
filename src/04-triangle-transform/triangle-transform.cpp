#include "glad.h"
#include <filesystem>
#include <fmt/core.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "utils.h"

// Global variables
static GLuint program{};
static GLFWcursor* hand_cursor{};
static float scale{1.0f};
static float rotate_x{0.0f};
static float translate_x{0.0f}, translate_y{0.0f};

static std::string window_title()
{
    return fmt::format(
        "04-triangle-transform (s={:3.2f}) (rx={:1.0f}) (tx={:3.2f},ty={:3.2f})",
        scale, rotate_x, translate_x, translate_y);
}

static GLuint compile_shaders()
{
    namespace fs = std::filesystem;
    return compile_shaders({
        fs::canonical(dirname() / ".." / "shader" / "transform.vert").c_str(),
        fs::canonical(dirname() / ".." / "shader" / "basic.frag").c_str(),
    });
}

static bool hit_test(
    GLFWwindow* window,
    double xcursor, double ycursor,
    double xndc, double yndc)
{
    int width{}, height{};
    glfwGetFramebufferSize(window, &width, &height);
    const double xw = (xndc + 1) * (width / 2.0); // window x [0..width]
    const double yw = (-yndc + 1) * (height / 2.0); // window y [0..height]
    const double xd = (xcursor - xw) * (xcursor - xw);
    const double yd = (ycursor - yw) * (ycursor - yw);
    return (xd + yd) < (5 * 5); // check is done in squared space to avoid square root
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
    glfwSetCursorPosCallback(
        window,
        [](GLFWwindow* window, double xpos, double ypos) {
            const bool hit = hit_test(window, xpos, ypos, 0.5, -0.5);
            glfwSetCursor(window, hit ? hand_cursor : nullptr);
            const int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if (state == GLFW_PRESS) {
                fmt::print("{}, {}\n", xpos, ypos);
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
    fmt::print("Use left trigger to scale, left stick to rotate, and right stick to translate.\n");
}

static void process_gamepad(GLFWwindow* window)
{
    GLFWgamepadstate state{};

    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
        if (state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        scale = state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] + 2.0f;
        rotate_x = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] * 70.0f;
        translate_x = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        translate_y = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
        glfwSetWindowTitle(window, window_title().c_str());
    }
}

static void render(GLFWwindow* window, double currentTime)
{
    const GLfloat background[]{0.2f, 0.2f, 0.2f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, background);

    // Build model-view matrix
    const auto identity_matrix = glm::mat4(1.0f);
    const auto scale_matrix = glm::scale(
        identity_matrix, glm::vec3(scale, scale, 0.0f));
    const auto rotate_matrix = glm::rotate(
        identity_matrix, glm::radians(rotate_x), glm::vec3(1.0f, 0.0f, 0.0f));
    const auto translate_matrix = glm::translate(
        identity_matrix, glm::vec3(translate_x, translate_y, 0.0f));
    const auto model_matrix = translate_matrix * rotate_matrix * scale_matrix;
    const auto view_matrix = glm::translate(
        identity_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
    const auto mv_matrix = view_matrix * model_matrix;

    // Copy model-view matrix to uniform variable
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mv_matrix));

    // Draw our first triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);
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

    GLFWwindow* window = glfwCreateWindow(800, 600, window_title().c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1); // vsync on

    print_info();
    hand_cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    set_callbacks(window);

    program = compile_shaders();
    glUseProgram(program);

    // Define the vertices of our triangle
    // Note that the winding order is counter-clockwise
    const GLfloat vertices[]{
        // position         color
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
         0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    // Create and populate interleaved vertex buffer
    // using DSA (Direct State Access) API in OpenGL 4.5.
    GLuint vbo{};
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);

    // Create VAO
    GLuint vao{};
    glCreateVertexArrays(1, &vao);

    // Bind the vertex buffer to the VAO's vertex buffer binding point
    constexpr GLuint binding_index{0}; // [0..GL_MAX_VERTEX_ATTRIB_BINDINGS)
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

    // Uncomment this call to draw in wireframe polygons
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
