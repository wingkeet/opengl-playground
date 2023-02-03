#include "glad.h"
#include <cmath>
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
static float camera_y{2.0f};

static std::string window_title()
{
    return fmt::format("06-cube (camera={:2.1f})", camera_y);
}

static GLuint compile_shaders()
{
    namespace fs = std::filesystem;
    return compile_shaders({
        fs::canonical(dirname() / ".." / "shader" / "mvp.vert").c_str(),
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
    glfwSetScrollCallback(
        window,
        [](GLFWwindow* window, double xoffset, double yoffset) {
            camera_y += yoffset * 0.5;
            camera_y = glm::clamp(camera_y, -3.0f, 3.0f);
            glfwSetWindowTitle(window, window_title().c_str());
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

    fmt::print("Use mouse wheel to move the camera up and down.\n");
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
    const glm::mat4 identity_matrix{1.0f};
    const float timef = static_cast<float>(current_time);
    const glm::mat4 model_matrix = glm::rotate(identity_matrix,
        std::sin(timef) * 2.0f, glm::vec3{0.0f, 1.0f, 0.0f});

    // Build view matrix
    const glm::vec3 camera{0.0f, camera_y, 5.0f};
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

    // Draw our first cube
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
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
    set_callbacks(window);

    program = compile_shaders();
    glUseProgram(program);

    // Define the vertices of our cube.
    // Note that the winding order is counter-clockwise.
    const GLfloat vertices[]{
        // front face, red
        -1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, // 0
         1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, // 1
         1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, // 2
        -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, // 3

        // back face, green
        -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 4
         1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 5
         1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 6
        -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 7

        // left face, blue
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // 8
        -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 9
        -1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 10
        -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // 11

        // right face, yellow
         1.0f, -1.0f,  1.0f, 1.0f, 1.0f, 0.0f, // 12
         1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, // 13
         1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f, // 14
         1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 0.0f, // 15

        // top face, magenta
        -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f, // 16
         1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f, // 17
         1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 1.0f, // 18
        -1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 1.0f, // 19

        // bottom face, cyan
        -1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // 20
         1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // 21
         1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, // 22
        -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, // 23
    };
    const GLuint indices[]{
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4,
        8, 9, 10,
        10, 11, 8,
        12, 13, 14,
        14, 15, 12,
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20,
    };

    // Create and populate interleaved vertex buffer using
    // DSA (Direct State Access) API in OpenGL 4.5.
    GLuint vbo{};
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);

    // Create and populate element buffer using DSA API in OpenGL 4.5
    GLuint ebo{};
    glCreateBuffers(1, &ebo);
    glNamedBufferStorage(ebo, sizeof(indices), indices, 0);

    // Create VAO
    GLuint vao{};
    glCreateVertexArrays(1, &vao);

    // Bind the element buffer to the VAO
    glVertexArrayElementBuffer(vao, ebo);

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
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();

    fmt::print("Bye.\n");
    return 0;
}
