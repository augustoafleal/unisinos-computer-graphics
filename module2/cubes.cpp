#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <fstream>

// GLAD
#include <GLAD/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

using namespace std;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
int setupShader();
int setupGeometry();
const GLchar* LoadShader(const char* file_name, int max_len);

// Window dimension
const GLuint WIDTH = 800, HEIGHT = 600;

const GLchar*  vertexShaderSource = LoadShader("../module2/shaders/vertex_shader.glsl", 1024 * 256);
const GLchar*  fragmentShaderSource = LoadShader("../module2/shaders/fragment_shader.glsl", 1024 * 256);

bool rotateX = false, rotateY = false, rotateZ = false;
glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float scaleFactor = 0.4f;
std::vector<glm::vec3> positions = {
    glm::vec3(-0.6f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.6f, 0.0f, 0.0f)
};
#include <filesystem>

int main()
{

    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Cubes - Augusto Leal", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Callback function
    glfwSetKeyCallback(window, keyCallback);

    // Callback Window Resize
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    // Get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
    const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    // Definindo as dimens�es da viewport com as mesmas dimens�es da janela da aplica��o
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);


    // Compile and build shader programCompilando e buildando o programa de shader
    GLuint shaderID = setupShader();

    // Generate simple buffer
    GLuint VAO = setupGeometry();

    glUseProgram(shaderID);

    glm::mat4 model = glm::mat4(1);
    GLint modelLoc = glGetUniformLocation(shaderID, "model");

    glEnable(GL_DEPTH_TEST);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Background
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        float angle = (GLfloat)glfwGetTime();
        deltaTime = angle - lastFrame;
        lastFrame = angle;

        // Draw cubes
        for (size_t i = 0; i < positions.size(); ++i)
        {
            model = glm::mat4(1);
            model = glm::translate(model, positions[i]);
            model = glm::scale(model, glm::vec3(scaleFactor, scaleFactor, scaleFactor)); // Escala uniformemente
            if (rotateX)
            {
                model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
            }
            else if (rotateY)
            {
                model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
            }
            else if (rotateZ)
            {
                model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
            }

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);

    glfwTerminate();
    return 0;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    float speed = 2.5f * deltaTime;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        rotateX = true;
        rotateY = false;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = true;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = false;
        rotateZ = true;
    }

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        for (auto& pos : positions)
        {
            pos.z -= speed;
        }
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        for (auto& pos : positions)
        {
            pos.z += speed;
        }
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        for (auto& pos : positions)
        {
            pos.x -= speed;
        }
    }

    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        for (auto& pos : positions)
        {
            pos.x += speed;
        }
    }

    if (key == GLFW_KEY_J && action == GLFW_PRESS)
    {
        for (auto& pos : positions)
        {
            pos.y -= speed;
        }
    }

    if (key == GLFW_KEY_I && action == GLFW_PRESS)
    {
        for (auto& pos : positions)
        {
            pos.y += speed;
        }
    }

    if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
    {
        scaleFactor -= speed;
    }

    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
    {
        scaleFactor += speed;
    }

    if (scaleFactor < 0.1f)
    {
        scaleFactor = 0.1f;
    }
    if (scaleFactor > 0.7f)
    {
        scaleFactor = 0.7f;
    }
}

int setupShader()
{
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // Bind shaders and create identifier of shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Checking links
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int setupGeometry()
{
    GLfloat vertices[] = {

        // Down - Red
        -0.5, -0.5, -0.5, 1.0, 0.0, 0.0,
        0.5, -0.5, -0.5, 1.0, 0.0, 0.0,
        0.5, -0.5, 0.5, 1.0, 0.0, 0.0,

        -0.5, -0.5, -0.5, 1.0, 0.0, 0.0,
        0.5, -0.5, 0.5, 1.0, 0.0, 0.0,
        -0.5, -0.5, 0.5, 1.0, 0.0, 0.0,

        // Upper - Green
        -0.5, 0.5, -0.5, 0.0, 1.0, 0.0,
        0.5, 0.5, -0.5, 0.0, 1.0, 0.0,
        0.5, 0.5, 0.5, 0.0, 1.0, 0.0,

        -0.5, 0.5, -0.5, 0.0, 1.0, 0.0,
        0.5, 0.5, 0.5, 0.0, 1.0, 0.0,
        -0.5, 0.5, 0.5, 0.0, 1.0, 0.0,

        // Front - Blue
        -0.5, -0.5, 0.5, 0.0, 0.0, 1.0,
        0.5, -0.5, 0.5, 0.0, 0.0, 1.0,
        0.5, 0.5, 0.5, 0.0, 0.0, 1.0,

        -0.5, -0.5, 0.5, 0.0, 0.0, 1.0,
        0.5, 0.5, 0.5, 0.0, 0.0, 1.0,
        -0.5, 0.5, 0.5, 0.0, 0.0, 1.0,

        // Back - Yellow
        -0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
        0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
        0.5, 0.5, -0.5, 1.0, 1.0, 0.0,

        -0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
        0.5, 0.5, -0.5, 1.0, 1.0, 0.0,
        -0.5, 0.5, -0.5, 1.0, 1.0, 0.0,

        // Left - Cyan
        -0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
        -0.5, -0.5, 0.5, 0.0, 1.0, 1.0,
        -0.5, 0.5, 0.5, 0.0, 1.0, 1.0,

        -0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
        -0.5, 0.5, 0.5, 0.0, 1.0, 1.0,
        -0.5, 0.5, -0.5, 0.0, 1.0, 1.0,

        // Right - Magenta
        0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
        0.5, -0.5, 0.5, 1.0, 0.0, 1.0,
        0.5, 0.5, 0.5, 1.0, 0.0, 1.0,

        0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
        0.5, 0.5, 0.5, 1.0, 0.0, 1.0,
        0.5, 0.5, -0.5, 1.0, 0.0, 1.0
    };
    GLuint VBO, VAO;

    // VBO identifier
    glGenBuffers(1, &VBO);

    // Bind buffer as array buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Send floats array to OpenGL buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // generate VAO identifier (Vertex Array Object)
    glGenVertexArrays(1, &VAO);

    // Bind VAO and connect vertices buffers
    glBindVertexArray(VAO);


    // Position attributes (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // colors attribute (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Unbind VAO
    glBindVertexArray(0);

    return VAO;
}

const GLchar* LoadShader(const char* file_name, int max_len) {
    char* shader_str = new char[max_len];
    shader_str[0] = '\0';

    FILE* file = fopen(file_name, "r");
    if (!file) {
        std::cerr << "ERROR: opening file for reading: " << file_name << std::endl;
        delete[] shader_str;
        return nullptr;
    }

    int current_len = 0;
    char line[2048];

    while (fgets(line, sizeof(line), file)) {
        int line_len = strlen(line);
        current_len += line_len;

        if (current_len >= max_len) {
            std::cerr << "ERROR: shader length is longer than string buffer length "
                      << max_len << std::endl;
            fclose(file);
            delete[] shader_str;
            return nullptr;
        }

        strcat(shader_str, line);
    }

    // Close the file
    if (fclose(file) != 0) {
        std::cerr << "ERROR: closing file from reading: " << file_name << std::endl;
        delete[] shader_str;
        return nullptr;
    }

    return shader_str;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}