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

// ------------------------
// Includes and Namespace
// ------------------------

using namespace std;

// ------------------------
// Constants
// ------------------------

const GLuint WIDTH = 800, HEIGHT = 600;

// ------------------------
// Structs
// ------------------------

struct Geometry {
    GLuint VAO;
    GLuint vertexCount;
    GLuint textureID = 0;
    string textureFilePath;
};

// ------------------------
// Function Declarations
// ------------------------

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
Geometry setupGeometry(const char* filepath);
bool loadObject(
    const char* path,
    vector<glm::vec3>& out_vertices,
    vector<glm::vec2>& out_uvs,
    vector<glm::vec3>& out_normals);
int loadTexture(const string& path);
string loadMTL(const string& path);

// ------------------------
// Transformation State
// ------------------------

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float scaleFactor = 0.4f;
glm::vec3 position = glm::vec3(0.0f);

vector<glm::vec3> positions = {
    glm::vec3(-0.6f, 0.0f, 0.0f),
    glm::vec3( 0.0f, 0.0f, 0.0f),
    glm::vec3( 0.6f, 0.0f, 0.0f)
};

// ------------------------
// Rotation State
// ------------------------

bool rotateX = false;
bool rotateY = false;
bool rotateZ = false;

// ------------------------
// Lighting Control
// ------------------------

bool keyLight  = true;
bool fillLight = true;
bool backLight = true;

// ------------------------
// Camera
// ------------------------

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 2.0f);

// ------------------------
// Material Properties
// ------------------------

string mtlFilePath = "";

glm::vec3 ambientColor(0.0f);
glm::vec3 diffuseColor(0.0f);
glm::vec3 specularColor(0.0f);
glm::vec3 emissiveColor(0.0f);

float shininess = 32.0f;

int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "20250531 - Atividade Vivencial", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, keyCallback);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
    const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    Shader shader("../20250531_atividadeVivencial/shaders/vertex_shader.glsl",
                  "../20250531_atividadeVivencial/shaders/fragment_shader.glsl");
    Geometry geom = setupGeometry("../20250531_atividadeVivencial/models/suzanne_painted/suzanne_painted.obj");

    glUseProgram(shader.ID);

    glm::mat4 model = glm::mat4(1);
    GLint modelLoc = glGetUniformLocation(shader.ID, "model");
    shader.setVec3("ka", ambientColor.r, ambientColor.g, ambientColor.b);
    shader.setVec3("kd", diffuseColor.r, diffuseColor.g, diffuseColor.b);
    shader.setVec3("ks", specularColor.r, specularColor.g, specularColor.b);
    shader.setVec3("ke", emissiveColor.r, emissiveColor.g, emissiveColor.b);
    shader.setFloat("q", shininess);

    std::cout << "Ambient Color: " << ambientColor.r << ", " << ambientColor.g << ", " << ambientColor.b << std::endl;
    std::cout << "Diffuse Color: " << diffuseColor.r << ", " << diffuseColor.g << ", " << diffuseColor.b << std::endl;
    std::cout << "Specular Color: " << specularColor.r << ", " << specularColor.g << ", " << specularColor.b <<
        std::endl;
    std::cout << "Emissive Color: " << emissiveColor.r << ", " << emissiveColor.g << ", " << emissiveColor.b <<
        std::endl;
    std::cout << "Shininess: " << shininess << std::endl;

    glm::vec3 lightPos[3] = {
        glm::vec3(-2.0f, 4.0f, 3.0f),
        glm::vec3(2.0f, -3.0f, 3.0f),
        glm::vec3(-2.0f, 0.5f, -3.0f)
    };

    glm::vec3 lightColor[3] = {
        glm::vec3(0.8f),
        glm::vec3(0.3f),
        glm::vec3(0.5f)
    };
    for (int i = 0; i < 3; ++i)
    {
        shader.setVec3(("lightPos[" + std::to_string(i) + "]").c_str(),
                       lightPos[i].x, lightPos[i].y, lightPos[i].z);

        shader.setVec3(("lightColor[" + std::to_string(i) + "]").c_str(),
                       lightColor[i].r, lightColor[i].g, lightColor[i].b);
    }

    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    shader.setMat4("view", glm::value_ptr(view));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    shader.setMat4("projection", glm::value_ptr(projection));

    model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/ glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, 0, glm::value_ptr(model));

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLineWidth(10);
        glPointSize(20);

        float time = static_cast<float>(glfwGetTime());
        deltaTime = time - lastFrame;
        lastFrame = time;

        // Uncomment the lines below to see the key light move around the object
        // lightPos[0].x = sin(time) * 5.0f;
        // lightPos[0].z = cos(time) * 5.0f;

        for (int i = 0; i < 3; ++i)
        {
            bool enabled = true;
            if (i == 0) enabled = keyLight;
            else if (i == 1) enabled = fillLight;
            else if (i == 2) enabled = backLight;

            glm::vec3 colorToSend = enabled ? lightColor[i] : glm::vec3(0.0f);

            shader.setVec3(("lightPos[" + std::to_string(i) + "]").c_str(),
                           lightPos[i].x, lightPos[i].y, lightPos[i].z);

            shader.setVec3(("lightColor[" + std::to_string(i) + "]").c_str(),
                           colorToSend.r, colorToSend.g, colorToSend.b);
        }

        model = glm::mat4(1.0f);

        for (const auto& pos : positions)
        {
            model = glm::translate(model, pos);
        }

        model = glm::scale(model, glm::vec3(scaleFactor));

        if (rotateX)
        {
            model = glm::rotate(model, time, glm::vec3(1.0f, 0.0f, 0.0f));
        }
        else if (rotateY)
        {
            model = glm::rotate(model, time, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (rotateZ)
        {
            model = glm::rotate(model, time, glm::vec3(0.0f, 0.0f, 1.0f));
        }

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(geom.VAO);
        glDrawArrays(GL_TRIANGLES, 0, geom.vertexCount);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &geom.VAO);
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

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        keyLight = !keyLight;
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        keyLight = !keyLight;
    }

    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        fillLight = !fillLight;
    }

    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        backLight = !backLight;
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

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

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
    geom.vertexCount = vertices.size() / 8;

    string basePath = string(filepath).substr(0, string(filepath).find_last_of("/"));
    string mtlPath = basePath + "/" + mtlFilePath;
    string textureFile = loadMTL(mtlPath);

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

        if (type == "v") // vertex data
        {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        }
        else if (type == "vt") // texture coordinate (UV)
        {
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (type == "vn") // normal vector
        {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (type == "f") // face data (indices for vertices, uvs, and normals)
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

string loadMTL(const string& path)
{
    ifstream mtlFile(path);
    if (!mtlFile)
    {
        cerr << "Failed to open MTL file: " << path << endl;
        return "";
    }

    string line, texturePath;
    while (getline(mtlFile, line))
    {
        istringstream iss(line);
        string keyword;
        iss >> keyword;

        if (keyword == "map_Kd")
        {
            iss >> texturePath;
        }
        else if (keyword == "Ka")
        {
            iss >> ambientColor.r >> ambientColor.g >> ambientColor.b;
        }
        else if (keyword == "Kd")
        {
            iss >> diffuseColor.r >> diffuseColor.g >> diffuseColor.b;
        }
        else if (keyword == "Ks")
        {
            iss >> specularColor.r >> specularColor.g >> specularColor.b;
        }
        else if (keyword == "Ns")
        {
            iss >> shininess;
        }
        else if (keyword == "Ke")
        {
            iss >> emissiveColor.r >> emissiveColor.g >> emissiveColor.b;
        }
    }
    mtlFile.close();

    if (texturePath.empty())
    {
        cerr << "No diffuse texture found in MTL file: " << path << endl;
    }
    return texturePath;
}
