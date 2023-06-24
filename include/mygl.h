#pragma once
#include "tgaimage.h"
#include "geometry.h"

namespace mygl
{

extern Matrix4f modelView;
extern Matrix4f viewport;
extern Matrix4f projection;

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P);

Matrix4f viewportMatrix(int x, int y, int w, int h);

Matrix4f projectionMatrix(float coeff);

Matrix4f viewMatrix(Vec3f cameraPos, Vec3f lookDir, Vec3f upDir);


class IShader
{
public:
    virtual ~IShader();
    virtual Vec3i vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

void triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer);

} // namespace mygl
