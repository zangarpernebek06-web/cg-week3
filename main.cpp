#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

// 1. Vertex Shader (uOffset және uScale)
const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

uniform vec2 uOffset;
uniform float uScale;

void main() {
    gl_Position = vec4(aPos.xy * uScale + uOffset, aPos.z, 1.0);
    ourColor = aColor;
}
)";

// Fragment Shader
const char* fragmentSrc = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(ourColor, 1.0);
}
)";

// Глобальды айнымалылар
float speed = 1.5f;       // рад/сек (Айналу жылдамдығы)
float orbitRadius = 0.4f;  // Радиус (Үшбұрыштардың арақашықтығы)

// Басқару функциясы (W, S, Q, E, R пернелері)
void processInput(GLFWwindow* window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // W / S — Айналу жылдамдығын өзгерту
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) speed += 2.0f * dt;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) speed -= 2.0f * dt;

    // Q / E — Орталықтан арақашықтықты (радиусты) өзгерту
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) orbitRadius -= 0.5f * dt;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) orbitRadius += 0.5f * dt;

    // Жылдамдықты шектеу (clamp)
    if (speed < 0.1f) speed = 0.1f;
    if (speed > 10.0f) speed = 10.0f;

    // Радиусты шектеу (экраннан шығып кетпеуі үшін)
    if (orbitRadius < 0.05f) orbitRadius = 0.05f;
    if (orbitRadius > 0.60f) orbitRadius = 0.60f;

    // R — Бастапқы мәндерді қайтару (Reset)
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        speed = 1.5f;
        orbitRadius = 0.4f;
    }
}

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Week 04 - Delta Time & Uniforms", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Шейдерлерді компиляциялау
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Үшбұрыш төбелері (координаталар мен түстер)
    float vertices[] = {
         0.0f,  0.1f, 0.0f,  1.0f, 0.0f, 0.0f,
        -0.1f, -0.1f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.1f, -0.1f, 0.0f,  0.0f, 0.0f, 1.0f
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Uniform орындарын іздеу
    int locOffset = glGetUniformLocation(shaderProgram, "uOffset");
    int locScale  = glGetUniformLocation(shaderProgram, "uScale");

    float lastFrame = (float)glfwGetTime();
    float angle = 0.0f;

    // Рендеринг циклі
    while (!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime();
        float dt = now - lastFrame;
        lastFrame = now;

        processInput(window, dt);

        angle += speed * dt;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // --- 1-ҮШБҰРЫШ: Сағат тіліне ҚАРСЫ айналады ---
        glUniform2f(locOffset, std::cos(angle) * orbitRadius, std::sin(angle) * orbitRadius);
        float scale1 = 0.75f + 0.25f * std::sin(now * 3.0f);
        glUniform1f(locScale, scale1);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // --- 2-ҮШБҰРЫШ: Сағат тіліМЕН (-angle) айналады ---
        glUniform2f(locOffset, std::cos(-angle) * orbitRadius, std::sin(-angle) * orbitRadius);
        float scale2 = 0.75f + 0.25f * std::cos(now * 3.0f);
        glUniform1f(locScale, scale2);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
