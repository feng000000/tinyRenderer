#include <algorithm>
#include "mygl.h"

Vec3f mygl::barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P)
{
    Vec3f u = Vec3f(C[0] - A[0], B[0] - A[0], A[0] - P[0]) ^
              Vec3f(C[1] - A[1], B[1] - A[1], A[1] - P[1]);

    if (std::abs(u[2]) > 1e-2) // u[2] 是整数. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1);
}

Matrix4f mygl::viewportMatrix(int x, int y, int w, int h)
{
    Matrix4f m = Matrix4f::identity();
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = 255.f / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = 255.f / 2.f;

    return m;
}

Matrix4f mygl::projectionMatrix(float coeff)
{
    Matrix4f p = Matrix4f::identity();

    p[3][2] = coeff;

    return p;
}

Matrix4f mygl::viewMatrix(Vec3f cameraPos, Vec3f lookDir, Vec3f upDir)
{
    cameraPos.normalize();
    lookDir.normalize();
    upDir.normalize();
    Vec3f rightDir = (lookDir ^ upDir).normalize();

    Matrix4f Rview = Matrix4f::identity();
    Rview[0][0] = rightDir.x;
    Rview[0][1] = rightDir.y;
    Rview[0][2] = rightDir.z;

    Rview[1][0] = upDir.x;
    Rview[1][1] = upDir.y;
    Rview[1][2] = upDir.z;

    Rview[2][0] = -lookDir.x;
    Rview[2][1] = -lookDir.y;
    Rview[2][2] = -lookDir.z;

    Rview[0][3] = -cameraPos.x;
    Rview[1][3] = -cameraPos.y;
    Rview[2][3] = -cameraPos.z;

    return Rview;
}

void mygl::triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer)
{

}

mygl::IShader::~IShader() {}