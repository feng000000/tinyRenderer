#pragma once
#include "tgaimage.h"
#include "geometry.h"

namespace mygl
{
extern Matrix4f modelView;
extern Matrix4f viewport;
extern Matrix4f projection;

void viewportMatrix(int x, int y, int w, int h);

void projectionMatrix(float coeff);

void viewMatrix(Vec3f cameraPos, Vec3f lookPos, Vec3f upDir);


class IShader
{
public:
    virtual ~IShader();

    // 顶点着色器
    virtual Vec4f vertex(int iface, int nthvert) = 0;

    // 片段着色器
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

void triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer);

} // namespace mygl
