#pragma once
#include "Curve.h"
class CatmullRom :
    public Curve
{
public:
    CatmullRom();
    GLuint generateCurve(int pointsPerSegment);
    GLuint generateBuffers();
};

