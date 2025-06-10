#include "CatmullRom.h"

CatmullRom::CatmullRom()
{
    M = glm::mat4(-1, 3, -3, 1,
                  2, -5, 4, -1,
                  -1, 0, 1, 0,
                  0, 2, 0, 0
    );
}

GLuint CatmullRom::generateCurve(int pointsPerSegment)
{
    float step = 1.0 / (float)pointsPerSegment;

    float t = 0;

    int nControlPoints = controlPoints.size();

    for (int i = 0; i < nControlPoints - 3; i += 3)
    {
        for (float t = 0.0; t <= 1.0; t += step)
        {
            glm::vec3 p;

            glm::vec4 T(t * t * t, t * t, t, 1);

            glm::vec3 P0 = controlPoints[i];
            glm::vec3 P1 = controlPoints[i + 1];
            glm::vec3 P2 = controlPoints[i + 2];
            glm::vec3 P3 = controlPoints[i + 3];

            glm::mat4x3 G(P0, P1, P2, P3);

            p = G * M * T; //---------
            p = p * glm::vec3(0.5, 0.5, 0.5);

            curvePoints.push_back(p);
        }
    }

    GLuint VAO = generateBuffers();

    return VAO;
}

GLuint CatmullRom::generateBuffers()
{
    GLuint VBO, VAO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, curvePoints.size() * sizeof(glm::vec3), curvePoints.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    this->VAO = VAO;
    return VAO;
}
