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
#include <stb_image/stb_image.h>
#include <random>
#include <algorithm>
#include <ParametricCurves/Bezier.h>
#include <ParametricCurves/Hermite.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
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
    float scaleFactor = 0.07;
    string name = "";

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
    glm::vec3 InitialPosition;

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
        InitialPosition = position;
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(const string& direction, float deltaTime)
    {
        float velocity = Speed * deltaTime;
        glm::vec3 newPosition = Position;

        if (direction == "FORWARD")
        {
            newPosition += Front * velocity;
        }
        if (direction == "BACKWARD")
        {
            newPosition -= Front * velocity;
        }
        if (direction == "LEFT")
        {
            newPosition -= Right * velocity;
        }
        if (direction == "RIGHT")
        {
            newPosition += Right * velocity;
        }

        float minX = InitialPosition.x - 10.0f;
        float maxX = InitialPosition.x + 10.0f;

        float minY = InitialPosition.y - 2.0f;
        float maxY = InitialPosition.y + 5.0f;

        float minZ = InitialPosition.z - 10.0f;
        float maxZ = InitialPosition.z + 10.0f;

        newPosition.x = glm::clamp(newPosition.x, minX, maxX);
        newPosition.y = glm::clamp(newPosition.y, minY, maxY);
        newPosition.z = glm::clamp(newPosition.z, minZ, maxZ);

        Position = newPosition;
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
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
Geometry setupGeometry(const char* filepath);
bool loadObject(const char* path,
                std::vector<glm::vec3>& out_vertices,
                std::vector<glm::vec2>& out_uvs,
                std::vector<glm::vec3>& out_normals);
GLuint setupBackgroundQuad(GLuint& quadVAO, GLuint& quadVBO, const char* texturePath, const char* type);
int loadTexture(const std::string& path);
Material loadMTL(const std::string& path);
vector<glm::vec3> generateControlPointsSet(int nPoints);
vector<glm::vec3> generateControlPointsSet();
std::vector<glm::vec3> generateUnisinosPointsSet();
GLuint generateControlPointsBuffer(vector<glm::vec3> controlPoints);
std::vector<glm::vec3> generateCircleControlPointsSet(int nPoints, float radius = 0.5f, string orientation = "");
std::vector<glm::vec3> generateAsteroidControlPoints();
bool checkSphereAABB(glm::vec3 sphereCenter, float radius, glm::vec3 boxMin, glm::vec3 boxMax);
bool changeToGameplayScene = false;
json jsonReader(string jsonpath);

// ------------------------
// Variables
// ------------------------

const GLuint WIDTH = 1024;
const GLuint HEIGHT = 768;

// ------------------------
// Scene states
// ------------------------

bool rotateX = false;
bool rotateY = false;
bool rotateZ = false;
bool throwBall = false;
int indexObject = 0;
std::vector<Geometry> sceneObjects;
std::vector<Geometry> numberObjects;

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
    glm::vec3(0.0f, 5.0f, 10.0f),
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

// ------------------------
// Parametric Curves
// ------------------------

int bezierNbCurvePoints = 0;
int bezierPointOnCurveIterReference = 0;
int hermiteNbCurvePoints = 0;
int hermitePointOnCurveIterReference = 0;
int bezierAsteroidsNbCurvePoints = 0;
int bezierAsteroidsPointOnCurveIterReference = 0;
vector<glm::vec3> pointsOnCurveVector;

// ------------------------
// Enum
// ------------------------

enum SceneState
{
    SELECTION_SCENE,
    GAMEPLAY_SCENE
};

SceneState currentScene = SELECTION_SCENE;

// ------------------------
// Ball Physics
// ------------------------

glm::vec3 ballPosition;
glm::vec3 ballVelocity;
bool initialized = false;
glm::vec3 forward = glm::normalize(camera.Front);
float launchSpeed = 20.0f;
float launchBallSpeed = 15.0f;
float ballGroundY = -10.0f;
float ballRadius = 0.5f;
bool alreadyScored = false;
int score = 0;

// ------------------------
// Ball Softening
// ------------------------

bool ballSoftened = false;
float softenTimer = 0.0f;
float softenDuration = 0.5f;

// ------------------------
// Bezier Logic
// ------------------------

static bool justLaunched = true;
static float bezierT = 0.0f;
static float bezierSpeed = 0.5f;
static std::vector<glm::vec3> bezierCurvePoints;
static bool bezierPhase = false;
static glm::vec3 finalBezierDirection;
bool throwBallBezier = false;

// ------------------------
// Flash Effect
// ------------------------

bool flashScreen = false;
float flashTimer = 0.0f;
const float flashDuration = 0.7f;

// ------------------------
// JSON Configuration
// ------------------------

std::string jsonpath = "../finalProject/basketball_config.json";

int main()
{
    glfwInit();

    json jsonData = jsonReader(jsonpath);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, jsonData["windowName"].get<string>().c_str(), nullptr,
                                          nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);


    Shader shader(jsonData["vertexShaderObject"].get<string>().c_str(),
                  jsonData["fragmentShaderObject"].get<string>().c_str());

    Geometry basketBall = setupGeometry(jsonData["basketBall"].get<string>().c_str());
    basketBall.position = glm::vec3(jsonData["basketBallPosition"][0], jsonData["basketBallPosition"][1],
                                    jsonData["basketBallPosition"][2]);
    basketBall.scaleFactor = jsonData["basketBallScaleFactor"];


    basketBall.name = "basketBall";

    Geometry orangeBall = setupGeometry(jsonData["orangeBall"].get<string>().c_str());
    orangeBall.position = glm::vec3(jsonData["orangeBallPosition"][0], jsonData["orangeBallPosition"][1],
                                    jsonData["orangeBallPosition"][2]);
    orangeBall.scaleFactor = jsonData["orangeBallScaleFactor"];
    orangeBall.name = "orangeBall";

    Geometry pumpkinBall = setupGeometry(jsonData["pumpkinBall"].get<string>().c_str());
    pumpkinBall.position = glm::vec3(jsonData["pumpkinBallPosition"][0], jsonData["pumpkinBallPosition"][1],
                                     jsonData["pumpkinBallPosition"][2]);
    pumpkinBall.scaleFactor = jsonData["pumpkinBallScaleFactor"];
    pumpkinBall.name = "pumpkinBall";

    Geometry basketHoop = setupGeometry(jsonData["basketHoop"].get<string>().c_str());
    basketHoop.position = glm::vec3(jsonData["basketHoopPosition"][0], jsonData["basketHoopPosition"][1],
                                    jsonData["basketHoopPosition"][2]);
    basketHoop.scaleFactor = jsonData["basketHoopScaleFactor"];

    glm::vec3 hoopBase = basketHoop.position;

    // --- Backboard ---ll
    glm::vec3 backboardMin = hoopBase + glm::vec3(-4.2f, 13.0f, 4.0f);
    glm::vec3 backboardMax = hoopBase + glm::vec3(3.0f, 18.2f, 4.0);

    // --- Rim ---
    glm::vec3 rimMin = hoopBase + glm::vec3(-0.2f, 13.5f, 7.0f);
    glm::vec3 rimMax = hoopBase + glm::vec3(0.2f, 13.5f, 7.0f);
    glm::vec3 rimCenter = hoopBase + glm::vec3(0.0f, 13.5f, 7.0f);
    glm::vec3 scoreZoneMin = hoopBase + glm::vec3(-1.0f, 15.0f, 3.0f);
    glm::vec3 scoreZoneMax = hoopBase + glm::vec3(-0.9f, 15.0f, 3.2f);
    glm::vec3 hoopCenter = (scoreZoneMin + scoreZoneMax) * 0.5f;

    // --- Pole ---
    glm::vec3 poleMin = hoopBase + glm::vec3(-1.0f, 0.0f, 3.0f);
    glm::vec3 poleMax = hoopBase + glm::vec3(1.0f, 13.0f, 3.1);

    sceneObjects = {basketBall, orangeBall, pumpkinBall};
    glUseProgram(shader.ID);

    glm::mat4 model = glm::mat4(1);
    GLint modelLoc = glGetUniformLocation(shader.ID, "model");

    shader.setVec3("lightPos", jsonData["lightPos"][0], jsonData["lightPos"][1], jsonData["lightPos"][2]);
    shader.setVec3("lightColor", jsonData["lightColor"][0], jsonData["lightColor"][1], jsonData["lightColor"][2]);

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    shader.setMat4("view", glm::value_ptr(view));
    shader.setMat4("projection", glm::value_ptr(projection));

    Shader shaderNumber(jsonData["vertexShaderNumber"].get<string>().c_str(),
                        jsonData["fragmentShaderNumber"].get<string>().c_str());

    for (int i = 0; i <= 3; ++i)
    {
        string numberObjectPath = jsonData["numberObject"].get<string>() + std::to_string(i) + "/number_" +
            std::to_string(i) +
            ".obj";
        Geometry number = setupGeometry(numberObjectPath.c_str());
        number.position = glm::vec3(jsonData["numberPosition"][0], jsonData["numberPosition"][1],
                                    jsonData["numberPosition"][2]);
        number.scaleFactor = jsonData["numberScaleFactor"];

        numberObjects.push_back(number);
    }

    glUseProgram(shaderNumber.ID);

    model = glm::mat4(1);
    modelLoc = glGetUniformLocation(shaderNumber.ID, "model");
    shaderNumber.setVec3("lightPos", jsonData["lightPos"][0], jsonData["lightPos"][1], jsonData["lightPos"][2]);
    shaderNumber.setVec3("lightColor", jsonData["lightColor"][0], jsonData["lightColor"][1], jsonData["lightColor"][2]);

    shaderNumber.setMat4("view", glm::value_ptr(view));
    shaderNumber.setMat4("projection", glm::value_ptr(projection));

    shaderNumber.setMat4("model", glm::value_ptr(model));

    // --- Background Floor ---
    GLuint backgroundVAO, backgroundVBO;
    GLuint backgroundTexture =
        setupBackgroundQuad(backgroundVAO, backgroundVBO, jsonData["floorTexture"].get<string>().c_str(), "floor");
    Shader backgroundShader(jsonData["vertexShaderBackground"].get<string>().c_str(),
                            jsonData["fragmentShaderBackground"].get<string>().c_str());
    glUseProgram(backgroundShader.ID);
    glUniform1i(glGetUniformLocation(backgroundShader.ID, "backgroundTexture"), 0);

    glm::mat4 modelFloor = glm::mat4(1);
    GLint modelLocFloor = glGetUniformLocation(backgroundShader.ID, "modelFloor");

    glUniformMatrix4fv(modelLocFloor, 1, 0, glm::value_ptr(modelFloor));

    // --- Background Stars ---
    GLuint backgroundStarsVAO, backgroundStarsVBO;
    GLuint backgroundStarsTexture = setupBackgroundQuad(backgroundStarsVAO, backgroundStarsVBO,
                                                        jsonData["starsTexture"].get<string>().c_str(),
                                                        "");
    Shader backgroundStarsShader(jsonData["vertexShaderBackgroundStars"].get<string>().c_str(),
                                 jsonData["fragmentShaderBackgroundStars"].get<string>().c_str());

    glUseProgram(backgroundStarsShader.ID);
    glUniform1i(glGetUniformLocation(backgroundStarsShader.ID, "backgroundStarsTexture"), 0);

    // --- Shader Curves  ---
    Shader curvesShader(jsonData["vertexShaderCurves"].get<string>().c_str(),
                        jsonData["fragmentShaderCurves"].get<string>().c_str());
    glUseProgram(curvesShader.ID);
    curvesShader.setMat4("view", glm::value_ptr(view));
    curvesShader.setMat4("projection", glm::value_ptr(projection));
    curvesShader.setVec4("finalColor", 1, 0, 0, 1);

    // --- Bezier ---
    Bezier bezier;
    bezier.setShader(&curvesShader);
    bezier.generateQuadraticCurve(30);
    bezierNbCurvePoints = bezier.getNbCurvePoints();

    // --- Hermite ---
    vector<glm::vec3> hermiteControlPoints = generateCircleControlPointsSet(100, 60.0f, "horizontal");
    GLuint VAOhermiteControlPoints = generateControlPointsBuffer(hermiteControlPoints);
    Hermite hermite;
    hermite.setControlPoints(hermiteControlPoints);
    hermite.setShader(&curvesShader);
    hermite.generateCurve(60);
    hermiteNbCurvePoints = hermite.getNbCurvePoints();

    while (!glfwWindowShouldClose(window))
    {
        // --- Events and buffers ---
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        view = camera.GetViewMatrix();
        shader.setMat4("view", glm::value_ptr(view));

        if (score >= 3)
        {
            score = 0;
            flashScreen = true;
            flashTimer = 0.0f;
        }

        if (flashScreen)
        {
            flashTimer += deltaTime;

            if (fmod(flashTimer * 10.0f, 2.0f) < 1.0f)
            {
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            }
            else
            {
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            }

            if (flashTimer > flashDuration)
            {
                flashScreen = false;
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            }
        }
        glDisable(GL_DEPTH_TEST);

        // --- Background Stars ---
        if (!flashScreen)
        {
            glUseProgram(backgroundStarsShader.ID);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, backgroundStarsTexture);
            glBindVertexArray(backgroundStarsVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }

        // --- Background Floor ---
        glUseProgram(backgroundShader.ID);
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        backgroundShader.setMat4("view", glm::value_ptr(view));
        backgroundShader.setMat4("projection", glm::value_ptr(projection));

        modelFloor = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        glUniformMatrix4fv(modelLocFloor, 1, GL_FALSE, glm::value_ptr(modelFloor));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        glBindVertexArray(backgroundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // --- Drawing Bezier Curve ---
        glUseProgram(curvesShader.ID);
        curvesShader.setMat4("view", glm::value_ptr(view));
        curvesShader.setMat4("projection", glm::value_ptr(projection));
        curvesShader.setVec4("finalColor", 1, 0, 0, 1);

        glm::vec3 p0 = ballPosition;
        glm::vec3 p2 = hoopCenter;

        glm::vec3 p1 = (p0 + p2) * 0.5f + glm::vec3(0.0f, 3.0f, 0.0f);
        std::vector<glm::vec3> controlPoints;
        controlPoints.push_back(p0);
        controlPoints.push_back(p1);
        controlPoints.push_back(p2);

        bezier.setControlPoints(controlPoints);
        bezier.generateQuadraticCurve(30);
        bezierNbCurvePoints = bezier.getNbCurvePoints();
        GLuint VAObezierControlPoints = generateControlPointsBuffer(controlPoints);

        glBindVertexArray(VAObezierControlPoints);
        glBindVertexArray(0);
        if (jsonData["showParametricCurves"].get<bool>())
        {
            bezier.drawCurve(glm::vec4(1, 0, 0, 1));
        }

        // --- Hermite ---
        glUseProgram(curvesShader.ID);
        curvesShader.setMat4("view", glm::value_ptr(view));
        curvesShader.setMat4("projection", glm::value_ptr(projection));
        curvesShader.setVec4("finalColor", 1, 1, 0, 1);

        glBindVertexArray(VAOhermiteControlPoints);
        glBindVertexArray(0);
        if (jsonData["showParametricCurves"].get<bool>())
        {
            hermite.drawCurve(glm::vec4(1, 1, 0, 1));
        }

        glm::vec3 hermitePointOnCurve = hermite.getPointOnCurve(hermitePointOnCurveIterReference);
        pointsOnCurveVector.push_back(hermitePointOnCurve);
        hermitePointOnCurveIterReference = (hermitePointOnCurveIterReference + 1) % hermiteNbCurvePoints;

        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        switch (currentScene)
        {
        case SELECTION_SCENE:
            {
                glEnable(GL_DEPTH_TEST);
                glUseProgram(shader.ID);
                shader.setMat4("view", glm::value_ptr(view));
                shader.setMat4("projection", glm::value_ptr(projection));
                shader.setVec4("finalColor", 1, 0, 0, 1);

                for (size_t i = 0; i < sceneObjects.size(); ++i)
                {
                    Geometry& obj = sceneObjects[i];

                    model = glm::mat4(1.0f);
                    model = glm::translate(model, obj.position);
                    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
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

                if (changeToGameplayScene)
                {
                    Geometry selectedBall = sceneObjects[indexObject];
                    sceneObjects.clear();
                    sceneObjects.resize(2);
                    sceneObjects[0] = selectedBall;
                    sceneObjects.push_back(basketHoop);
                    currentScene = GAMEPLAY_SCENE;
                }

                break;
            }
        case GAMEPLAY_SCENE:
            {
                // --- Scene objects ---
                glEnable(GL_DEPTH_TEST);
                glUseProgram(shader.ID);
                shader.setMat4("view", glm::value_ptr(view));
                shader.setMat4("projection", glm::value_ptr(projection));
                shader.setVec4("finalColor", 1, 0, 0, 1);

                for (size_t i = 0; i < sceneObjects.size(); ++i)
                {
                    Geometry& obj = sceneObjects[i];

                    if (i == 0) // Ball
                    {
                        if (!throwBall)
                        {
                            glm::vec3 forward = glm::normalize(camera.Front);
                            glm::vec3 up = glm::normalize(camera.Up);
                            glm::vec3 right = glm::normalize(glm::cross(forward, up));

                            glm::vec3 offset = forward * 2.0f + up * -0.5f; // + right * 0.0f;
                            glm::vec3 ballPosition = camera.Position + offset;

                            model = glm::mat4(1.0f);
                            model = glm::translate(model, ballPosition);

                            glm::mat4 rotationOnly = glm::mat4(glm::mat3(camera.GetViewMatrix()));
                            model *= glm::inverse(rotationOnly);
                        }
                        else
                        {
                            if (throwBallBezier)
                            {
                                if (justLaunched)
                                {
                                    glm::vec3 p0 = camera.Position + camera.Front * 2.0f + camera.Up * -0.5f;
                                    glm::vec3 p2 = hoopCenter;
                                    glm::vec3 p1 = (p0 + p2) * 0.5f + glm::vec3(0.0f, 8.0f, 0.0f);

                                    bezierCurvePoints.clear();
                                    for (int i = 0; i <= 30; ++i)
                                    {
                                        float t = i / 30.0f;
                                        glm::vec3 point =
                                            (1 - t) * (1 - t) * p0 +
                                            2 * (1 - t) * t * p1 +
                                            t * t * p2;
                                        bezierCurvePoints.push_back(point);
                                    }

                                    bezierT = 0.0f;
                                    ballPosition = p0;
                                    bezierPhase = true;
                                    justLaunched = false;
                                }

                                if (bezierPhase)
                                {
                                    bezierT += deltaTime * bezierSpeed;

                                    if (bezierT >= 1.0f)
                                    {
                                        bezierPhase = false;

                                        glm::vec3 pA = bezierCurvePoints[bezierCurvePoints.size() - 2];
                                        glm::vec3 pB = bezierCurvePoints.back();
                                        finalBezierDirection = glm::normalize(pB - pA);

                                        ballVelocity = finalBezierDirection * launchBallSpeed * 0.6f;
                                    }
                                    else
                                    {
                                        int totalPoints = bezierCurvePoints.size();
                                        float indexF = bezierT * (totalPoints - 1);
                                        int idx = (int)indexF;
                                        float frac = indexF - idx;

                                        if (idx < totalPoints - 1)
                                        {
                                            ballPosition = glm::mix(bezierCurvePoints[idx], bezierCurvePoints[idx + 1],
                                                                    frac);
                                        }
                                    }
                                }
                                else
                                {
                                    ballVelocity += glm::vec3(0.0f, -4.81f, 0.0f) * deltaTime;
                                    ballPosition += ballVelocity * deltaTime;
                                }
                            }
                            else
                            {
                                glm::vec3 launchDir = glm::normalize(camera.Front + camera.Up * 0.25f);

                                if (justLaunched)
                                {
                                    ballVelocity = launchDir * launchBallSpeed;
                                    ballPosition = camera.Position + camera.Front * 2.0f + camera.Up * -0.5f;
                                    justLaunched = false;
                                }

                                ballVelocity += glm::vec3(0.0f, -4.81f, 0.0f) * deltaTime;
                                ballPosition += ballVelocity * deltaTime;
                            }

                            if (!alreadyScored &&
                                checkSphereAABB(ballPosition, ballRadius, scoreZoneMin, scoreZoneMax) &&
                                ballVelocity.y < 0.0f)
                            {
                                score++;
                                alreadyScored = true;
                                ballSoftened = true;
                                softenTimer = 0.0f;
                                std::cout << "Hoop! Score: " << score << std::endl;
                            }

                            if (alreadyScored &&
                                !checkSphereAABB(ballPosition, ballRadius, scoreZoneMin, scoreZoneMax))
                            {
                                alreadyScored = false;
                            }

                            if (ballSoftened)
                            {
                                softenTimer += deltaTime;
                                ballVelocity *= 0.5f;

                                if (softenTimer >= softenDuration)
                                {
                                    ballSoftened = false;
                                }
                            }

                            model = glm::mat4(1.0f);
                            model = glm::translate(model, ballPosition);
                            model = glm::rotate(model, currentFrame * 5, glm::vec3(-1.0f, 0.0f, 0.0f));

                            if (ballPosition.y < ballGroundY)
                            {
                                ballPosition.y = ballGroundY;
                                ballVelocity.y *= -0.6f;
                                ballVelocity.x *= 0.95f;
                                ballVelocity.z *= 0.95f;

                                if (glm::length(ballVelocity) < 10.5f)
                                {
                                    throwBall = false;
                                    throwBallBezier = false;

                                    ballVelocity = glm::vec3(0.0f);
                                    ballPosition = camera.Position + camera.Front * 2.0f + camera.Up * -0.5f;
                                    justLaunched = true;
                                }
                            }

                            if (checkSphereAABB(ballPosition, ballRadius, backboardMin, backboardMax) ||
                                checkSphereAABB(ballPosition, ballRadius, rimMin, rimMax) ||
                                checkSphereAABB(ballPosition, ballRadius, poleMin, poleMax))
                            {
                                glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);
                                ballVelocity = ballVelocity - 2.0f * glm::dot(ballVelocity, normal) * normal;
                                ballVelocity *= 0.7f;
                                ballPosition += normal * 0.03f;
                                std::cout << "Colision\n";
                            }
                        }
                        if (obj.name != "basketBall")
                        {
                            model = glm::scale(model, glm::vec3(obj.scaleFactor));
                        }
                    }
                    else
                    {
                        model = glm::mat4(1.0f);
                        model = glm::translate(model, obj.position);
                        model = glm::rotate(model, glm::radians(-90.0f),
                                            glm::vec3(jsonData["objectRotation"][0], jsonData["objectRotation"][1],
                                                      jsonData["objectRotation"][2]));
                        model = glm::scale(model, glm::vec3(obj.scaleFactor));
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

                Geometry& numberObject = numberObjects[score];
                numberObject.position = pointsOnCurveVector[0];

                glUseProgram(shaderNumber.ID);

                shaderNumber.setMat4("view", glm::value_ptr(view));
                shaderNumber.setMat4("projection", glm::value_ptr(projection));
                shaderNumber.setVec4("finalColor", 1.0, 0, 0, 1);

                glm::vec3 center = glm::vec3(0.0f, numberObject.position.y, 0.0f);
                glm::vec3 dirToCenter = glm::normalize(center - numberObject.position);
                float angle = atan2(dirToCenter.x, dirToCenter.z);

                model = glm::mat4(1.0f);
                model = glm::translate(model, numberObject.position);
                model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(-90.0f),
                                    glm::vec3(jsonData["numberRotation"][0], jsonData["numberRotation"][1],
                                              jsonData["numberRotation"][2]));

                model = glm::scale(model, glm::vec3(numberObject.scaleFactor));

                shaderNumber.setVec3("ka", numberObject.ka.r, numberObject.ka.g, numberObject.ka.b);
                shaderNumber.setVec3("kd", numberObject.kd.r, numberObject.kd.g, numberObject.kd.b);
                shaderNumber.setVec3("ks", numberObject.ks.r, numberObject.ks.g, numberObject.ks.b);
                shaderNumber.setVec3("ke", numberObject.ke.r, numberObject.ke.g, numberObject.ke.b);
                shaderNumber.setFloat("q", numberObject.shininess);
                shaderNumber.setVec3("color", 1.0f, 0.0f, 0.0f);

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                glBindTexture(GL_TEXTURE_2D, numberObject.textureID);
                glBindVertexArray(numberObject.VAO);
                glDrawArrays(GL_TRIANGLES, 0, numberObject.vertexCount);
                break;
            }
        }

        // --- Finalizing Frame ---
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glfwSwapBuffers(window);
        pointsOnCurveVector.clear();

        continousKeyPress(window, camera, deltaTime);
    }

    // --- Cleanup ---
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

    if (currentScene == SELECTION_SCENE)
    {
        if (key == GLFW_KEY_0 && action == GLFW_PRESS)
        {
            indexObject = 0;
        }

        if (key == GLFW_KEY_1 && action == GLFW_PRESS)
        {
            indexObject = 1;
        }

        if (key == GLFW_KEY_2 && action == GLFW_PRESS)
        {
            indexObject = 2;
        }

        if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
        {
            changeToGameplayScene = true;
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
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }


    if (sceneObjects[indexObject].scaleFactor < 0.001f)
    {
        sceneObjects[indexObject].scaleFactor = 0.001f;
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

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (currentScene == GAMEPLAY_SCENE)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            throwBall = true;
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            throwBallBezier = true;
            throwBall = true;
        }
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

GLuint setupBackgroundQuad(GLuint& quadVAO, GLuint& quadVBO, const char* texturePath, const char* type)
{
    std::vector<float> quadVertices;
    GLuint textureID;

    if (type == "floor")
    {
        float floorSize = 60.0f;
        float halfSize = floorSize / 2;
        float repeatFactor = 50.0f;
        float floorHeight = 2.5f;

        quadVertices = {
            -halfSize, floorHeight, -halfSize, 0.0f, 0.0f,
            halfSize, floorHeight, -halfSize, repeatFactor, 0.0f,
            halfSize, floorHeight, halfSize, repeatFactor, repeatFactor,

            -halfSize, floorHeight, -halfSize, 0.0f, 0.0f,
            halfSize, floorHeight, halfSize, repeatFactor, repeatFactor,
            -halfSize, floorHeight, halfSize, 0.0f, repeatFactor
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(float), quadVertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        textureID = loadTexture(texturePath);
    }
    else
    {
        quadVertices = {
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
        glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(float), quadVertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        textureID = loadTexture(texturePath);
    }


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

std::vector<glm::vec3> generateCircleControlPointsSet(int nPoints, float radius, string orientation)
{
    std::vector<glm::vec3> controlPoints;

    for (int i = 0; i < nPoints; i++)
    {
        float angle = (2.0f * M_PI * i) / nPoints;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        float z = radius * sin(angle);
        if (orientation == "horizontal")
        {
            controlPoints.push_back(glm::vec3(x, 0.0f, z));
        }
        else
        {
            controlPoints.push_back(glm::vec3(x, y, 0.0f));
        }
    }

    return controlPoints;
}

std::vector<glm::vec3> generateControlPointsSet(int nPoints)
{
    std::vector<glm::vec3> controlPoints;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution(-0.9f, 0.9f);

    for (int i = 0; i < nPoints; i++)
    {
        glm::vec3 point;
        do
        {
            point.x = distribution(gen);
            point.y = distribution(gen);
        }
        while (find(controlPoints.begin(), controlPoints.end(), point) != controlPoints.end());

        point.z = 0.0f;

        controlPoints.push_back(point);
    }

    return controlPoints;
}

vector<glm::vec3> generateControlPointsSet()
{
    vector<glm::vec3> controlPoints;

    controlPoints.push_back(glm::vec3(-0.6, -0.4, 0.0));
    controlPoints.push_back(glm::vec3(-0.4, -0.6, 0.0));
    controlPoints.push_back(glm::vec3(-0.2, -0.2, 0.0));
    controlPoints.push_back(glm::vec3(0.0, 0.0, 0.0));
    controlPoints.push_back(glm::vec3(0.2, 0.2, 0.0));
    controlPoints.push_back(glm::vec3(0.4, 0.6, 0.0));
    controlPoints.push_back(glm::vec3(0.6, 0.4, 0.0));

    return controlPoints;
}

GLuint generateControlPointsBuffer(vector<glm::vec3> controlPoints)
{
    GLuint VBO, VAO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(GLfloat) * 3, controlPoints.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

std::vector<glm::vec3> generateAsteroidControlPoints()
{
    std::vector<glm::vec3> pts;
    glm::vec3 start(-50, 20, -8);
    glm::vec3 end(50, -20, -4);
    int numPoints = 100;

    for (int i = 1; i <= numPoints; ++i)
    {
        float t = (float)i / (numPoints + 1);

        glm::vec3 point = (1 - t) * start + t * end;

        point.y += sin(t * 3.14f) * 2.0f;
        point.z += cos(t * 3.14f) * 1.0f;

        pts.push_back(point);
    }

    return pts;
}

bool checkSphereAABB(glm::vec3 sphereCenter, float radius, glm::vec3 boxMin, glm::vec3 boxMax)
{
    float x = std::max(boxMin.x, std::min(sphereCenter.x, boxMax.x));
    float y = std::max(boxMin.y, std::min(sphereCenter.y, boxMax.y));
    float z = std::max(boxMin.z, std::min(sphereCenter.z, boxMax.z));

    float distance = glm::length(glm::vec3(x, y, z) - sphereCenter);
    return distance < radius;
}

json jsonReader(string jsonpath)
{
    ifstream f(jsonpath);
    json jsonData = json::parse(f);
    string json_string = jsonData.dump();

    return jsonData;
}
