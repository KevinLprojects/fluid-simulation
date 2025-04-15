#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define CIRCLE_SEGMENTS 32
#define NUM_CIRCLES 10
#define PIXEL_RADIUS 40.0f // Constant pixel radius

float offsets[NUM_CIRCLES * 2];

void fillOffsets() {
    for (int i = 0; i < NUM_CIRCLES; i++) {
        float x = (i % 5) * 100.0f; // 100px spacing
        float y = (i / 5) * 100.0f;
        offsets[i * 2 + 0] = x;
        offsets[i * 2 + 1] = y;
    }
}

const char* vertexShaderSource =
    "#version 120\n"
    "uniform vec2 windowSize;\n"
    "uniform float pixelRadius;\n"
    "attribute vec2 position;\n"
    "attribute vec2 offset;\n"
    "void main() {\n"
    "   vec2 scale = vec2(pixelRadius * 2.0 / windowSize.x,\n"
    "                     pixelRadius * 2.0 / windowSize.y);\n"
    "   vec2 screenOffset = vec2(-1.0, -1.0) + offset;\n"
    "   gl_Position = vec4(position * scale + screenOffset, 0.0, 1.0);\n"
    "}\n";

const char* fragmentShaderSource =
    "#version 120\n"
    "void main() {\n"
    "   gl_FragColor = vec4(0.2, 0.8, 0.6, 1.0);\n"
    "}\n";

// These need to be global so the refresh callback can access them
GLFWwindow* window;
GLuint program, instanceVBO;
GLint windowSizeLoc, pixelRadiusLoc;

void render() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glUniform2f(windowSizeLoc, (float)width, (float)height);
    glUniform1f(pixelRadiusLoc, PIXEL_RADIUS);

    // Convert pixel offsets to NDC
    float screenOffsets[NUM_CIRCLES * 2];
    for (int i = 0; i < NUM_CIRCLES; i++) {
        float x = (float)(rand() % width);
        float y = (float)(rand() % height);
        screenOffsets[i * 2 + 0] = x / (float)width * 2.0f;
        screenOffsets[i * 2 + 1] = y / (float)height * 2.0f;
    }


    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(screenOffsets), screenOffsets);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2, NUM_CIRCLES);
    glfwSwapBuffers(window);
}

void refreshCallback(GLFWwindow* win) {
    render();
}

int main() {
    srand((unsigned int)time(NULL));

    if (!glfwInit()) return -1;
    window = glfwCreateWindow(800, 600, "balls", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Disable VSync for responsiveness

    glewExperimental = GL_TRUE;
    glewInit();

    glfwSetWindowRefreshCallback(window, refreshCallback);
    fillOffsets();

    // Generate unit circle as triangle fan
    float circle[(CIRCLE_SEGMENTS + 2) * 2];
    circle[0] = 0.0f; circle[1] = 0.0f;
    for (int i = 0; i <= CIRCLE_SEGMENTS; i++) {
        float angle = (float)i / CIRCLE_SEGMENTS * 2.0f * 3.1415926f;
        circle[(i + 1) * 2 + 0] = cosf(angle);
        circle[(i + 1) * 2 + 1] = sinf(angle);
    }

    // Buffers
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &instanceVBO);
    glBindVertexArray(vao);

    // Circle vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Instance offset buffer (NDC, updated each frame)
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(offsets), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(1, 1);

    // Shader setup
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(vs);
    glCompileShader(fs);

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "offset");
    glLinkProgram(program);
    glUseProgram(program);

    // Uniform locations
    windowSizeLoc = glGetUniformLocation(program, "windowSize");
    pixelRadiusLoc = glGetUniformLocation(program, "pixelRadius");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        render();
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
