#include "geometry.h"
#include "camera.h"


Camera::Camera(Vec3f cameraPos, Vec3f lookDir, Vec3f upDir)
{
    pos_        = cameraPos.normalize();
    lookDir_    = lookDir.normalize();
    upDir_      = upDir.normalize();
    rightDir_   = (lookDir ^ upDir).normalize();
}

Matrix4f Camera::viewMatrix()
{
    Matrix4f Rview = Matrix4f::identity();
    Rview[0][0] = OX().x;
    Rview[0][1] = OX().y;
    Rview[0][2] = OX().z;

    Rview[1][0] = OY().x;
    Rview[1][1] = OY().y;
    Rview[1][2] = OY().z;

    Rview[2][0] = OZ().x;
    Rview[2][1] = OZ().y;
    Rview[2][2] = OZ().z;

    Rview[0][3] = -pos_.x;
    Rview[1][3] = -pos_.y;
    Rview[2][3] = -pos_.z;

    return Rview;
}


// ------------------- private ------------------- //

Vec3f Camera::OX() { return rightDir_; }


Vec3f Camera::OY() { return upDir_; }


Vec3f Camera::OZ() { return lookDir_ * -1.f; }
