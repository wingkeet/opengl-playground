#include "glad.h"
#include <fmt/core.h>
#include <GLFW/glfw3.h>
#include "shader.h"

// Global variables
static GLuint program{};
static GLFWcursor* hand_cursor{};

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
    fmt::print("OpenGL vendor: {}\n", glGetString(GL_VENDOR));
    fmt::print("OpenGL renderer: {}\n", glGetString(GL_RENDERER));
    fmt::print("OpenGL version: {}\n", glGetString(GL_VERSION));
    fmt::print("GLSL version: {}\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    GLint max_attributes{};
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attributes);
    fmt::print("Max vertex attributes: {}\n", max_attributes);

    GLint max_uniforms{};
    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &max_uniforms);
    fmt::print("Max uniform locations: {}\n", max_uniforms);

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

static void render(GLFWwindow* window, double currentTime)
{
    const GLfloat background[]{0.2f, 0.2f, 0.2f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, background);

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

    GLFWwindow* window = glfwCreateWindow(800, 600, "02-cube", nullptr, nullptr);
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
    const GLfloat positions[]{
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
    };
    const GLfloat colors[]{
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };

    GLuint vao{};
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and populate the positions buffer.
    // Vertex attribute arrays are disabled by default, so we call
    // glEnableVertexAttribArray() to enable them.
    GLuint positions_vbo{};
    glGenBuffers(1, &positions_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Create and populate the colors buffer
    GLuint colors_vbo{};
    glGenBuffers(1, &colors_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    // Uncomment this call to draw in wireframe polygons
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window)) {
        process_gamepad(window);
        render(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Shutting down from here onwards
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &positions_vbo);
    glDeleteBuffers(1, &colors_vbo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();

    fmt::print("Bye\n");
    return 0;
}