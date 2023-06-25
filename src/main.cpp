#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "mygl.h"

template <class t>
using vector = std::vector<t>;

const int width = 800;
const int height = 800;
const int depth = 255;

std::unique_ptr<Model> model;
Vec3f cameraPos = Vec3f(0, -1, 3);
Vec3f lookPos   = Vec3f(0, 0, 0);
Vec3f upPos     = Vec3f(0, 1, 0);
// 光线的反方向(光线从该点射向原点), 这样方便判断光线是否照到平面. (light_dir点乘法向量 > 0)
Vec3f light_dir = Vec3f(1, 1, 1);


// 采用Gourand着色模型的着色器
class GouraudShader : public mygl::IShader
{
public:
    Vec3f varying_intensity; // written by vertex shader, read by fragment shader

    // iface为三角形编号
    // nthvert为顶点编号
    // 返回屏幕坐标
    virtual Vec4f vertex(int iface, int nthvert) override
    {
        using namespace mygl;

        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert), 1.0f); // read the vertex from .obj file

        gl_Vertex = viewport *  projection *  modelView * gl_Vertex;     // transform it to screen coordinates
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir); // get diffuse lighting intensity

        return gl_Vertex;
    }

    // bar为当前像素相对于三角形的重心坐标
    // color为当前像素的颜色
    // 返回是否丢弃该像素
    virtual bool fragment(Vec3f bar, TGAColor &color) override
    {
        // 通过重心坐标插值计算强度
        float intensity = varying_intensity * bar;

        color = TGAColor(255, 255, 255) * intensity;

        // 是否丢弃该像素
        return false;
    }
};

int main(int argc, char **argv)
{
    model = std::make_unique<Model>("../data/african_head.obj");
    model->load_texture("../data/african_head_diffuse.tga");

    mygl::viewMatrix(cameraPos, lookPos, upPos);
    mygl::viewportMatrix(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    mygl::projectionMatrix( -1.f / (cameraPos - lookPos).norm());
    light_dir.normalize();

    TGAImage image  (width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    GouraudShader shader;

    for (int i = 0; i < model->nfaces(); i ++)
    {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j ++)
            screen_coords[j] = shader.vertex(i, j);
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.flip_vertically();
    zbuffer.flip_vertically();
    image.write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    return 0;
}
