#define GLEW_STATIC
#define GLEW_NO_GLU
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES

#include "constants.h"
#include "mutexes.h"
#include "structures.h"

#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define CIRCLE_SEGMENTS 12
#define PIXEL_RADIUS 20.0f

extern CircleData circles[];

GLuint program;
GLuint instanceVBO_pos, instanceVBO_col;
GLint windowSizeLoc, pixelRadiusLoc;
GLFWwindow* window;

void render() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // Update instance buffers
    float instancePositions[NUM_PARTICLES * 2];
    float instanceColors[NUM_PARTICLES * 3];

    pthread_mutex_lock(&renderDataMutex);
    for (int i = 0; i < NUM_PARTICLES; i++) {

        instancePositions[i * 2 + 0] = 2.0f * circles[i].x;
        instancePositions[i * 2 + 1] = 2.0f * circles[i].y;

        // Set color
        instanceColors[i * 3 + 0] = circles[i].r;
        instanceColors[i * 3 + 1] = circles[i].g;
        instanceColors[i * 3 + 2] = circles[i].b;
    }
    pthread_mutex_unlock(&renderDataMutex);

    glUseProgram(program);

    glUniform2f(windowSizeLoc, (float)width, (float)height);
    glUniform1f(pixelRadiusLoc, PIXEL_RADIUS); // or whatever radius you want

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_pos);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(instancePositions), instancePositions);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_col);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(instanceColors), instanceColors);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2, NUM_PARTICLES);
    glfwSwapBuffers(window);
}


void refreshCallback(GLFWwindow* win) {
    render();
}

int loop() {
    if (!glfwInit()) return -1;

    window = glfwCreateWindow(800, 600, "Instanced Circles", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    glewInit();

    // Create circle geometry
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
    printf("Setting up circle geometry...\n");
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Instance position buffer
    glGenBuffers(1, &instanceVBO_pos);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_pos);
    glBufferData(GL_ARRAY_BUFFER, NUM_PARTICLES * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1); // offset
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(1, 1);

    // Instance color buffer
    glGenBuffers(1, &instanceVBO_col);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_col);
    glBufferData(GL_ARRAY_BUFFER, NUM_PARTICLES * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(2); // color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(2, 1);

    // Shaders
    const char* vertexShaderSource =
        "#version 120\n"
        "uniform vec2 windowSize;\n"
        "uniform float pixelRadius;\n"
        "attribute vec2 position;\n"
        "attribute vec2 offset;\n"
        "attribute vec3 color;\n"
        "varying vec3 vColor;\n"
        "void main() {\n"
        "   vec2 scale = vec2(pixelRadius * 2.0 / windowSize.x,\n"
        "                     pixelRadius * 2.0 / windowSize.y);\n"
        "   vec2 screenOffset = vec2(-1.0, -1.0) + offset;\n"
        "   gl_Position = vec4(position * scale + screenOffset, 0.0, 1.0);\n"
        "   vColor = color;\n"
        "}\n";

    const char* fragmentShaderSource =
        "#version 120\n"
        "varying vec3 vColor;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(vColor, 1.0);\n"
        "}\n";

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
    glBindAttribLocation(program, 2, "color");
    glLinkProgram(program);
    glUseProgram(program);

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