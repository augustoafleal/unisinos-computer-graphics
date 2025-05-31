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
string loadMTL(const string& path);


const GLuint WIDTH = 800, HEIGHT = 600;
bool rotateX = false, rotateY = false, rotateZ = false;
glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float scaleFactor = 0.1f;
vector<glm::vec3> positions = {
    glm::vec3(-0.6f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.6f, 0.0f, 0.0f)
};
string mtlFilePath = "";
glm::vec3 ambientColor(0.0f), diffuseColor(0.0f), specularColor(0.0f), emissiveColor(0.0f);
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 2.0f);
float shininess = 32.0f;

int main()
{
    glfwInit();


    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Lighting - Augusto Leal", nullptr, nullptr);
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

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    Shader shader("../20250531_atividadeVivencial/shaders/vertex_shader_v2.glsl",
                  "../20250531_atividadeVivencial/shaders/fragment_shader_v2.glsl");
    Geometry geom = setupGeometry("../20250531_atividadeVivencial/models/suzanne_painted/suzanne_painted.obj");

    glUseProgram(shader.ID);

    glm::mat4 model = glm::mat4(1);
    GLint modelLoc = glGetUniformLocation(shader.ID, "model");

    shader.setVec3("ka", ambientColor.r, ambientColor.g, ambientColor.b);
    shader.setVec3("kd", diffuseColor.r, diffuseColor.g, diffuseColor.b);
    shader.setVec3("ks", specularColor.r, specularColor.g, specularColor.b);
    shader.setVec3("ke", emissiveColor.r, emissiveColor.g, emissiveColor.b);
    shader.setFloat("q", shininess);

    // Antes de enviar para o shader:
    std::cout << "Ambient Color: " << ambientColor.r << ", " << ambientColor.g << ", " << ambientColor.b << std::endl;
    std::cout << "Diffuse Color: " << diffuseColor.r << ", " << diffuseColor.g << ", " << diffuseColor.b << std::endl;
    std::cout << "Specular Color: " << specularColor.r << ", " << specularColor.g << ", " << specularColor.b << std::endl;
    std::cout << "Emissive Color: " << emissiveColor.r << ", " << emissiveColor.g << ", " << emissiveColor.b << std::endl;
    std::cout << "Shininess: " << shininess << std::endl;
    // Pass light properties
    //shader.setVec3("lightPos", 0.0f, 2.0f, 0.0f);
    //shader.setVec3("lightColor", 1.3f, 1.3f, 1.3f);

    // glm::vec3 keyLightPos = glm::vec3( 2.0f, 2.0f, 2.0f);
    // glm::vec3 keyLightColor   = glm::vec3(1.0f, 1.0f, 1.0f);
    //
    // glm::vec3 fillLightPos   = glm::vec3(-2.0f, 1.5f, 2.0f);
    // glm::vec3 fillLightColor  = glm::vec3(0.5f, 0.5f, 0.5f);
    //
    // glm::vec3 backLightPos   = glm::vec3( 0.0f, 2.5f, -2.0f);
    // glm::vec3 backLightColor  = glm::vec3(0.3f, 0.3f, 0.3f);

    glm::vec3 lightPositions[3] = {
        glm::vec3(-2.7f, 0.0f, 0.9f),
        glm::vec3(2.0f, 1.5f, 2.0f),
        glm::vec3(0.0f, 2.5f, -2.0f)
    };

    glm::vec3 lightColors[3] = {
        glm::vec3(0.8f, 0.2f, 0.2f), // vermelho mais suave
        glm::vec3(0.2f, 0.2f, 0.8f), // azul fraco para contraste
        glm::vec3(0.3f, 0.3f, 0.3f)  // luz branca suave para fill light
    };

    for (int i = 0; i < 3; ++i) {
        shader.setVec3(("lightPos[" + std::to_string(i) + "]").c_str(),
                       lightPositions[i].x, lightPositions[i].y, lightPositions[i].z);

        shader.setVec3(("lightColor[" + std::to_string(i) + "]").c_str(),
                       lightColors[i].r, lightColors[i].g, lightColors[i].b);
    }

    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    shader.setMat4("view", glm::value_ptr(view));
    shader.setMat4("projection", glm::value_ptr(projection));

    model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/ glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, 0, glm::value_ptr(model));

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Limpa a tela
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Tempo para animações
        float time = static_cast<float>(glfwGetTime());
        deltaTime = time - lastFrame;
        lastFrame = time;

        // Animação da luz principal (key light)
        lightPositions[0].x = 3.0f * sin(time);

        // Envia luzes para o shader
        for (int i = 0; i < 3; ++i) {
            shader.setVec3(("lightPos[" + std::to_string(i) + "]").c_str(),
                           lightPositions[i].x, lightPositions[i].y, lightPositions[i].z);

            shader.setVec3(("lightColor[" + std::to_string(i) + "]").c_str(),
                           lightColors[i].r, lightColors[i].g, lightColors[i].b);
        }

        // Model matrix
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

        // Renderiza objeto
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(geom.VAO);
        glDrawArrays(GL_TRIANGLES, 0, geom.vertexCount);
        glBindVertexArray(0);
       // glBindTexture(GL_TEXTURE_2D, 0);

        glfwSwapBuffers(window);
    }

    // Cleanup
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

    if (scaleFactor < 0.1f)
    {
        scaleFactor = 0.1f;
    }
    if (scaleFactor > 0.7f)
    {
        scaleFactor = 0.7f;
    }
}

//
// Geometry setupGeometry(const char* filepath)
// {
//     std::vector<GLfloat> vertices;
//     std::vector<glm::vec3> vert;
//     std::vector<glm::vec2> uvs;
//     std::vector<glm::vec3> normals;
//
//     loadObject(filepath, vert, uvs, normals);
//
//     vertices.reserve(vert.size() * 8); // 3 for position + 3 for color + 2 for UV
//     for (size_t i = 0; i < vert.size(); ++i)
//     {
//         vertices.insert(vertices.end(), {vert[i].x, vert[i].y, vert[i].z, 1.0f, 0.0f, 1.0f, uvs[i].x, uvs[i].y});
//     }
//
//     GLuint VBO, VAO;
//
//     // Generate VBO
//     glGenBuffers(1, &VBO);
//
//     // Bind VBO
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//
//     // Upload vertex data to the GPU
//     glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
//
//     // Generate VAO
//     glGenVertexArrays(1, &VAO);
//
//     // Bind VAO
//     glBindVertexArray(VAO);
//
//     // Define position attribute (3 floats: x, y, z)
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
//     glEnableVertexAttribArray(0);
//
//     // Define color attribute (3 floats: r, g, b)
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//     glEnableVertexAttribArray(1);
//
//     // Define texture coordinate attribute (2 floats: u, v)
//     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
//     glEnableVertexAttribArray(2);
//
//     // Unbind VBO and VAO
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
//
//     // Return the struct containing VAO and the number of vertices
//     Geometry geom;
//     geom.VAO = VAO;
//     geom.vertexCount = vertices.size() / 8; // Each vertex has 6 components (3 for position + 3 for color)
//
//     //Load MTL and Texture
//     string basePath = string(filepath).substr(0, string(filepath).find_last_of("/"));
//     string mtlPath = basePath + "/" + mtlFilePath;
//     string textureFile = loadMTL(mtlPath);
//
//     //if (!textureFile.empty())
//     //{
//     //    string fullTexturePath = basePath + "/" + textureFile;
//     //    geom.textureID = loadTexture(fullTexturePath);
//     //    geom.textureFilePath = fullTexturePath;
//     //}
//
//     return geom;
// }

// SEM COR
Geometry setupGeometry(const char* filepath)
{
    std::vector<GLfloat> vertices;
    std::vector<glm::vec3> vert;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    loadObject(filepath, vert, uvs, normals);

    // 3 for position + 3 for normal + 2 for UV = 8
    vertices.reserve(vert.size() * 8);
    for (size_t i = 0; i < vert.size(); ++i)
    {
        vertices.insert(vertices.end(),
                        {
                            vert[i].x, vert[i].y, vert[i].z,           // position
                            normals[i].x, normals[i].y, normals[i].z, // normal
                            uvs[i].x, uvs[i].y                         // uv
                        });
    }

    GLuint VBO, VAO;

    // Generate and bind VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Generate and bind VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    // position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // uv attribute (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Return geometry
    Geometry geom;
    geom.VAO = VAO;
    geom.vertexCount = vertices.size() / 8;

    // Load texture (if needed)
    string basePath = string(filepath).substr(0, string(filepath).find_last_of("/"));
    string mtlPath = basePath + "/" + mtlFilePath;
    string textureFile = loadMTL(mtlPath);

    // Optional texture loading
    // if (!textureFile.empty()) {
    //     string fullTexturePath = basePath + "/" + textureFile;
    //     geom.textureID = loadTexture(fullTexturePath);
    //     geom.textureFilePath = fullTexturePath;
    // }

    return geom;
}

// // COM COR
// Geometry setupGeometry(const char* filepath)
// {
//     std::vector<GLfloat> vertices;
//     std::vector<glm::vec3> vert;
//     std::vector<glm::vec2> uvs;
//     std::vector<glm::vec3> normals;
//
//     loadObject(filepath, vert, uvs, normals);
//
//     // 3 for position + 3 for normal + 2 for UV
//     vertices.reserve(vert.size() * 11); // 3+3+2+3 = 11
//     for (size_t i = 0; i < vert.size(); ++i)
//     {
//         vertices.insert(vertices.end(),
//                         {
//                             vert[i].x, vert[i].y, vert[i].z, // position
//                             normals[i].x, normals[i].y, normals[i].z, // normal
//                             uvs[i].x, uvs[i].y, // uv
//                             1.0f, 0.0f, 0.0f // red color (example)
//                         });
//     }
//
//     GLuint VBO, VAO;
//
//     // Generate and bind VAO
//     glGenVertexArrays(1, &VAO);
//     glBindVertexArray(VAO);
//
//     // Generate and bind VBO
//     glGenBuffers(1, &VBO);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
//
//     // position attribute (location = 0)
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
//     glEnableVertexAttribArray(0);
//
//     // normal attribute (location = 3)
//     glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//     glEnableVertexAttribArray(1);
//
//     // uv attribute (location = 2)
//     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
//     glEnableVertexAttribArray(2);
//
//     // color attribute (location = 1)
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
//     glEnableVertexAttribArray(3);
//
//
//     // Unbind
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
//
//     // Return geometry
//     Geometry geom;
//     geom.VAO = VAO;
//     geom.vertexCount = vertices.size() / 11; // 3 + 3 + 2
//
//     // Load texture (if needed)
//     string basePath = string(filepath).substr(0, string(filepath).find_last_of("/"));
//     string mtlPath = basePath + "/" + mtlFilePath;
//     string textureFile = loadMTL(mtlPath);
//
//     // Optional texture loading
//     // if (!textureFile.empty()) {
//     //     string fullTexturePath = basePath + "/" + textureFile;
//     //     geom.textureID = loadTexture(fullTexturePath);
//     //     geom.textureFilePath = fullTexturePath;
//     // }
//
//     return geom;
// }


// // COM COR
// Geometry setupGeometry(const char* filepath)
// {
//     std::vector<GLfloat> vertices;
//     std::vector<glm::vec3> vert;
//     std::vector<glm::vec2> uvs;
//     std::vector<glm::vec3> normals;
//
//     loadObject(filepath, vert, uvs, normals);
//
//     // 3 for position + 3 for normal + 2 for UV
//     vertices.reserve(vert.size() * 11); // 3+3+2+3 = 11
//     for (size_t i = 0; i < vert.size(); ++i)
//     {
//         vertices.insert(vertices.end(),
//                         {
//                             vert[i].x, vert[i].y, vert[i].z, // position
//                             normals[i].x, normals[i].y, normals[i].z, // normal
//                             uvs[i].x, uvs[i].y, // uv
//                             1.0f, 0.0f, 0.0f // red color (example)
//                         });
//     }
//
//     GLuint VBO, VAO;
//
//     // Generate and bind VAO
//     glGenVertexArrays(1, &VAO);
//     glBindVertexArray(VAO);
//
//     // Generate and bind VBO
//     glGenBuffers(1, &VBO);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
//
//     // position attribute (location = 0)
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
//     glEnableVertexAttribArray(0);
//
//     // normal attribute (location = 3)
//     glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//     glEnableVertexAttribArray(1);
//
//     // uv attribute (location = 2)
//     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
//     glEnableVertexAttribArray(2);
//
//     // color attribute (location = 1)
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
//     glEnableVertexAttribArray(3);
//
//
//     // Unbind
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
//
//     // Return geometry
//     Geometry geom;
//     geom.VAO = VAO;
//     geom.vertexCount = vertices.size() / 11; // 3 + 3 + 2
//
//     // Load texture (if needed)
//     string basePath = string(filepath).substr(0, string(filepath).find_last_of("/"));
//     string mtlPath = basePath + "/" + mtlFilePath;
//     string textureFile = loadMTL(mtlPath);
//
//     // Optional texture loading
//     // if (!textureFile.empty()) {
//     //     string fullTexturePath = basePath + "/" + textureFile;
//     //     geom.textureID = loadTexture(fullTexturePath);
//     //     geom.textureFilePath = fullTexturePath;
//     // }
//
//     return geom;
// }

//
// Geometry setupGeometry(const char* filepath)
// {
//     std::vector<GLfloat> vertices;
//     std::vector<glm::vec3> vert;
//     std::vector<glm::vec2> uvs;
//     std::vector<glm::vec3> normals;
//
//     loadObject(filepath, vert, uvs, normals);
//
//     // 3 for position + 3 for normal + 2 for UV
//     vertices.reserve(vert.size() * 8);
//     for (size_t i = 0; i < vert.size(); ++i)
//     {
//         vertices.insert(vertices.end(),
//         {
//             vert[i].x, vert[i].y, vert[i].z,            // position
//             normals[i].x, normals[i].y, normals[i].z,   // normal
//             uvs[i].x, uvs[i].y                          // uv
//         });
//     }
//
//     GLuint VBO, VAO;
//
//     // Generate and bind VAO
//     glGenVertexArrays(1, &VAO);
//     glBindVertexArray(VAO);
//
//     // Generate and bind VBO
//     glGenBuffers(1, &VBO);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
//
//     // position attribute (location = 0)
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
//     glEnableVertexAttribArray(0);
//
//     // normal attribute (location = 3)
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//     glEnableVertexAttribArray(1);
//
//     // uv attribute (location = 2)
//     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
//     glEnableVertexAttribArray(2);
//
//     // Unbind
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
//
//     // Return geometry
//     Geometry geom;
//     geom.VAO = VAO;
//     geom.vertexCount = vertices.size() / 8;
//
//     // Load texture (if needed)
//     string basePath = string(filepath).substr(0, string(filepath).find_last_of("/"));
//     string mtlPath = basePath + "/" + mtlFilePath;
//     string textureFile = loadMTL(mtlPath);
//
//     // Optional texture loading
//     // if (!textureFile.empty()) {
//     //     string fullTexturePath = basePath + "/" + textureFile;
//     //     geom.textureID = loadTexture(fullTexturePath);
//     //     geom.textureFilePath = fullTexturePath;
//     // }
//
//     return geom;
// }

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
