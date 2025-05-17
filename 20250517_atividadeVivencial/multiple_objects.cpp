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

struct Geometry
{
    GLuint VAO;
    GLuint vertexCount;
    GLuint textureID = 0;
    string textureFilePath;
    glm::vec3 position;
    float scaleFactor = 0.1;
};

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
Geometry setupGeometry(const char* filepath);
bool loadObject(
    const char* path,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec2>& out_uvs,
    std::vector<glm::vec3>& out_normals);
int loadTexture(const string& path);

const GLuint WIDTH = 800, HEIGHT = 600;
bool rotateX = false, rotateY = false, rotateZ = false, posD = false;
float lastFrame = 0.0f;
float scaleFactor = 0.25f;
string mtlFilePath = "";
glm::vec3 ambientColor(0.0f), diffuseColor(0.0f), specularColor(0.0f), emissiveColor(0.0f);
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 2.0f);
int indexObject = 0;
int indexColor = 0;
std::vector<Geometry> sceneObjects;

int main()
{

    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "20250517 - Atividade Vivencial", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, keyCallback);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

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

    Shader shader("../20250517_atividadeVivencial/shaders/vertex_shader_simple.glsl", "../20250517_atividadeVivencial/shaders/fragment_shader_simple.glsl");
    Geometry suz1 = setupGeometry("../20250517_atividadeVivencial/models/suzanne/suzanne.obj");
    suz1.position = glm::vec3(-0.4f, 0.0f, 0.0f);

    Geometry suz2 = setupGeometry("../20250517_atividadeVivencial/models/suzanne/suzanne.obj");
    suz2.position = glm::vec3(0.4f, 0.0f, 0.0f);

    sceneObjects= { suz1, suz2 };

    glUseProgram(shader.ID);

    glm::mat4 model = glm::mat4(1);
    GLint modelLoc = glGetUniformLocation(shader.ID, "model");

    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    shader.setMat4("view", glm::value_ptr(view));
    shader.setMat4("projection", glm::value_ptr(projection));

    model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/ glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, 0, glm::value_ptr(model));

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float angle = (float)glfwGetTime();

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
            }

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glBindTexture(GL_TEXTURE_2D, 0);

            glBindVertexArray(obj.VAO);
            glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &suz1.VAO);
    glDeleteVertexArrays(1, &suz2.VAO);

    glfwTerminate();
    return 0;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    float speed = 0.1f;

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

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        sceneObjects[indexObject].position.z -= speed;
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        sceneObjects[indexObject].position.z += speed;
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        sceneObjects[indexObject].position.x -= speed;
    }

    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        sceneObjects[indexObject].position.x += speed;
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


Geometry setupGeometry(const char* filepath)
{
    std::vector<GLfloat> vertices;
    std::vector<glm::vec3> vert;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    loadObject(filepath, vert, uvs, normals);

    vertices.reserve(vert.size() * 8); // 3 for position + 3 for color + 2 for UV

    if (indexColor == 0)
    {
        for (size_t i = 0; i < vert.size(); ++i)
        {
            vertices.insert(vertices.end(), {vert[i].x, vert[i].y, vert[i].z, 1.0f, 0.0f, 0.0f, uvs[i].x, uvs[i].y});
        }
    } else if (indexColor == 1)
    {
        for (size_t i = 0; i < vert.size(); ++i)
        {
            vertices.insert(vertices.end(), {vert[i].x, vert[i].y, vert[i].z, 1.0f, 1.0f, 0.0f, uvs[i].x, uvs[i].y});
        }
    }

    indexColor += 1;

    GLuint VBO, VAO;

    // Generate VBO
    glGenBuffers(1, &VBO);

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Upload vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    // Generate VAO
    glGenVertexArrays(1, &VAO);

    // Bind VAO
    glBindVertexArray(VAO);

    // Define position attribute (3 floats: x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Define color attribute (3 floats: r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Define texture coordinate attribute (2 floats: u, v)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // Unbind VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Geometry geom;
    geom.VAO = VAO;
    geom.vertexCount = vertices.size() / 6;

    string basePath = string(filepath).substr(0, string(filepath).find_last_of("/"));
    string mtlPath = basePath + "/" + mtlFilePath;

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
