#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <fstream>
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <filesystem>
#include <shader/Shader.h>
#include "stb_image/stb_image.h"

using namespace std;

// ------------------------
// Structs
// ------------------------

struct Geometry
{
    GLuint VAO;
    GLuint vertexCount;
    GLuint textureID = 0;
    string textureFilePath;
    glm::vec3 position;
    float scaleFactor = 0.4;

    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    glm::vec3 ke;
    float shininess = 32.0f;
};

struct Material
{
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    glm::vec3 ke;
    float shininess;
    string texturePath;
};

struct Camera
{
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float Speed;
    float Sensitivity;

    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), Speed(1.0f), Sensitivity(0.1f)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(const string& direction, float deltaTime)
    {
        float velocity = Speed * deltaTime;

        if (direction == "FORWARD")
        {
            Position += Front * velocity;
        }
        if (direction == "BACKWARD")
        {
            Position -= Front * velocity;
        }
        if (direction == "LEFT")
        {
            Position -= Right * velocity;
        }
        if (direction == "RIGHT")
        {
            Position += Right * velocity;
        }
    }

    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

// ------------------------
// Functions
// ------------------------

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void continousKeyPress(GLFWwindow* window, Camera& camera, float currentTime);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
Geometry setupGeometry(const char* filepath);
bool loadObject(const char* path,
                std::vector<glm::vec3>& out_vertices,
                std::vector<glm::vec2>& out_uvs,
                std::vector<glm::vec3>& out_normals);
GLuint setupBackgroundQuad(GLuint& quadVAO, GLuint& quadVBO, const char* texturePath);
int loadTexture(const std::string& path);
Material loadMTL(const std::string& path);

// ------------------------
// Variables
// ------------------------

const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;

// ------------------------
// Scene states
// ------------------------

bool rotateX = false;
bool rotateY = false;
bool rotateZ = false;

int indexObject = 0;
std::vector<Geometry> sceneObjects;

// ------------------------
// Time transformations
// ------------------------

glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float speed = 0.1f;

// ------------------------
// Light and materials
// ------------------------

std::string mtlFilePath = "";
glm::vec3 ambientColor(0.0f);
glm::vec3 diffuseColor(0.0f);
glm::vec3 specularColor(0.0f);
glm::vec3 emissiveColor(0.0f);

// ------------------------
// Camera
// ------------------------

Camera camera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    -90.0f,
    0.0f
);

// ------------------------
// Mouse control
// ------------------------

bool firstMouse = true;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;

int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Controle de câmera FPS - Augusto Leal", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Callback functions
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    Shader shader("../module5/shaders/vertex_shader.glsl", "../module5/shaders/fragment_shader.glsl");
    Geometry mars = setupGeometry("../module5/models/mars/mars.obj");
    mars.position = glm::vec3(-2.0f, 0.0f, 0.0f);

    Geometry moon = setupGeometry("../module5/models/moon/moon.obj");
    moon.position = glm::vec3(2.0f, 0.0f, 0.0f);
    sceneObjects = {mars, moon};
    glUseProgram(shader.ID);

    glm::mat4 model = glm::mat4(1);
    GLint modelLoc = glGetUniformLocation(shader.ID, "model");

    shader.setVec3("lightPos", 4.0f, 0.0f, 0.0f);
    shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    shader.setMat4("view", glm::value_ptr(view));
    shader.setMat4("projection", glm::value_ptr(projection));

    model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/ glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, 0, glm::value_ptr(model));

    // STARS
    GLuint backgroundVAO, backgroundVBO;
    GLuint backgroundTexture = setupBackgroundQuad(backgroundVAO, backgroundVBO, "../module5/textures/stars.jpg");
    Shader backgroundShader("../module5/shaders/vertex_background.glsl", "../module5/shaders/fragment_background.glsl");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = camera.GetViewMatrix();
        shader.setMat4("view", glm::value_ptr(view));

        // STARS
        glDisable(GL_DEPTH_TEST);
        glUseProgram(backgroundShader.ID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        //backgroundShader.setInt("background", 0);

        glBindVertexArray(backgroundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
        glUseProgram(shader.ID);

        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        for (size_t i = 0; i < sceneObjects.size(); ++i)
        {
            Geometry& obj = sceneObjects[i];

            model = glm::mat4(1.0f);
            model = glm::translate(model, obj.position);
            model = glm::scale(model, glm::vec3(obj.scaleFactor));

            if (indexObject == i)
            {
                if (rotateX)
                {
                    model = glm::rotate(model, currentFrame, glm::vec3(1.0f, 0.0f, 0.0f));
                }
                else if (rotateY)
                {
                    model = glm::rotate(model, currentFrame, glm::vec3(0.0f, 1.0f, 0.0f));
                }
                else if (rotateZ)
                {
                    model = glm::rotate(model, currentFrame, glm::vec3(0.0f, 0.0f, 1.0f));
                }
            }

            shader.setVec3("ka", obj.ka.r, obj.ka.g, obj.ka.b);
            shader.setVec3("kd", obj.kd.r, obj.kd.g, obj.kd.b);
            shader.setVec3("ks", obj.ks.r, obj.ks.g, obj.ks.b);
            shader.setVec3("ke", obj.ke.r, obj.ke.g, obj.ke.b);
            shader.setFloat("q", obj.shininess);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glBindTexture(GL_TEXTURE_2D, obj.textureID);

            glBindVertexArray(obj.VAO);
            glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);

            continousKeyPress(window, camera, deltaTime);
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glfwSwapBuffers(window);
    }

    for (const Geometry& obj : sceneObjects)
    {
        glDeleteVertexArrays(1, &obj.VAO);
    }

    glfwTerminate();
    return 0;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    float speedControl = 2.5f;
    speed = speedControl * deltaTime;

    if (key == GLFW_KEY_0 && action == GLFW_PRESS)
    {
        indexObject = 0;
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        indexObject = 1;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

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

    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        glm::vec3 right = glm::normalize(glm::cross(camera.Front, camera.Up));
        sceneObjects[indexObject].position -= right * speed;
    }

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        sceneObjects[indexObject].position += glm::normalize(camera.Front) * speed;
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        sceneObjects[indexObject].position -= glm::normalize(camera.Front) * speed;
    }

    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        glm::vec3 right = glm::normalize(glm::cross(camera.Front, camera.Up));
        sceneObjects[indexObject].position += right * speed;
    }

    if (key == GLFW_KEY_J && action == GLFW_PRESS)
    {
        sceneObjects[indexObject].position.y -= speed;
    }

    if (key == GLFW_KEY_I && action == GLFW_PRESS)
    {
        sceneObjects[indexObject].position.y += speed;
    }

    if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
    {
        sceneObjects[indexObject].scaleFactor -= speed;
    }

    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
    {
        sceneObjects[indexObject].scaleFactor += speed;
    }

    if (sceneObjects[indexObject].scaleFactor < 0.1f)
    {
        sceneObjects[indexObject].scaleFactor = 0.1f;
    }

    if (sceneObjects[indexObject].scaleFactor > 0.7f)
    {
        sceneObjects[indexObject].scaleFactor = 0.7f;
    }
}

void continousKeyPress(GLFWwindow* window, Camera& camera, float currentTime)
{
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        camera.ProcessKeyboard("FORWARD", currentTime);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        camera.ProcessKeyboard("BACKWARD", currentTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        camera.ProcessKeyboard("LEFT", currentTime);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        camera.ProcessKeyboard("RIGHT", currentTime);
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    static float sensitivity = 0.1f;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.Yaw += xoffset;
    camera.Pitch += yoffset;

    if (camera.Pitch > 89.0f)
    {
        camera.Pitch = 89.0f;
    }
    if (camera.Pitch < -89.0f)
    {
        camera.Pitch = -89.0f;
    }

    camera.updateCameraVectors();
}

Geometry setupGeometry(const char* filepath)
{
    std::vector<GLfloat> vertices;
    std::vector<glm::vec3> vert;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    loadObject(filepath, vert, uvs, normals);

    vertices.reserve(vert.size() * 8);
    for (size_t i = 0; i < vert.size(); ++i)
    {
        vertices.insert(vertices.end(),
                        {
                            vert[i].x, vert[i].y, vert[i].z,
                            uvs[i].x, uvs[i].y,
                            normals[i].x, normals[i].y, normals[i].z
                        });
    }

    GLuint VBO, VAO;

    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Geometry geom;
    geom.VAO = VAO;
    geom.vertexCount = vertices.size() / 6; // 3 for position + 3 for color

    string basePath = string(filepath).substr(0, string(filepath).find_last_of("/"));
    string mtlPath = basePath + "/" + mtlFilePath;
    Material mat = loadMTL(mtlPath);
    geom.ka = mat.ka;
    geom.kd = mat.kd;
    geom.ks = mat.ks;
    geom.ke = mat.ke;
    geom.shininess = mat.shininess;
    geom.textureFilePath = mat.texturePath;

    if (!mat.texturePath.empty())
    {
        string fullTexturePath = basePath + "/" + mat.texturePath;
        geom.textureID = loadTexture(fullTexturePath);
    }

    return geom;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

bool loadObject(
    const char* path,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec2>& out_uvs,
    std::vector<glm::vec3>& out_normals)
{
    std::ifstream file(path);

    if (!file)
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        return false;
    }

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v")
        {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        }
        else if (type == "vt")
        {
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (type == "vn")
        {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (type == "f")
        {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            char slash;

            for (int i = 0; i < 3; ++i)
            {
                iss >> vertexIndex[i] >> slash >> uvIndex[i] >> slash >> normalIndex[i];
                vertexIndices.push_back(vertexIndex[i]);
                uvIndices.push_back(uvIndex[i]);
                normalIndices.push_back(normalIndex[i]);
            }
        }
        else if (type == "mtllib")
        {
            iss >> mtlFilePath;
        }
    }

    // Populate output vectors
    for (unsigned int i = 0; i < vertexIndices.size(); ++i)
    {
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        glm::vec2 uv = temp_uvs[uvIndex - 1];
        glm::vec3 normal = temp_normals[normalIndex - 1];

        out_vertices.push_back(vertex);
        out_uvs.push_back(uv);
        out_normals.push_back(normal);
    }

    file.close();

    return true;
}

GLuint setupBackgroundQuad(GLuint& quadVAO, GLuint& quadVBO, const char* texturePath)
{
    float quadVertices[] = {
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    GLuint textureID = loadTexture(texturePath);

    return textureID;
}

int loadTexture(const string& path)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cerr << "Failed to load texture: " << path << endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

Material loadMTL(const string& path)
{
    Material mat;
    ifstream mtlFile(path);
    if (!mtlFile)
    {
        cerr << "Failed to open MTL file: " << path << endl;
        return mat;
    }

    string line;
    while (getline(mtlFile, line))
    {
        istringstream iss(line);
        string keyword;
        iss >> keyword;

        if (keyword == "map_Kd")
        {
            iss >> mat.texturePath;
        }
        else if (keyword == "Ka")
        {
            iss >> mat.ka.r >> mat.ka.g >> mat.ka.b;
        }
        else if (keyword == "Kd")
        {
            iss >> mat.kd.r >> mat.kd.g >> mat.kd.b;
        }
        else if (keyword == "Ks")
        {
            iss >> mat.ks.r >> mat.ks.g >> mat.ks.b;
        }
        else if (keyword == "Ke")
        {
            iss >> mat.ke.r >> mat.ke.g >> mat.ke.b;
        }
        else if (keyword == "Ns")
        {
            iss >> mat.shininess;
        }
    }
    return mat;
}
