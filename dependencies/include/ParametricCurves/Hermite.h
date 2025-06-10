#include "Curve.h"

class Hermite : public Curve
{
public:
    Hermite();
    GLuint generateCurve(int pointsPerSegment);
    GLuint generateBuffers();
};
