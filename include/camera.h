#pragma once
#include "geometry.h"

class Camera
{
public:
    Camera(Vec3f pos, Vec3f lookDir, Vec3f upDir);
    Matrix viewMatrix();

private:
    Vec3f pos_;
    Vec3f rightDir_; // OX
    Vec3f upDir_;    // OY
    Vec3f lookDir_;  // -OZ

    Vec3f OX();
    Vec3f OY();
    Vec3f OZ();
};