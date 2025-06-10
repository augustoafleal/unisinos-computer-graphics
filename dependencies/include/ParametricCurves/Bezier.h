#pragma once
#include "Curve.h"
class Bezier :
    public Curve
{
public:
    Bezier();
    GLuint generateCurve(int pointsPerSegment);
    GLuint generateQuadraticCurve(int pointsPerSegment);
    GLuint generateBuffers();
};

