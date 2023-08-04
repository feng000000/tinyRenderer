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
Vec3f cameraPos = Vec3f(1, 1, 3);
Vec3f lookPos   = Vec3f(0, 0, 0);
Vec3f upPos     = Vec3f(0, 1, 0);
// 光线的反方向(光线从该点射向原点), 这样方便判断光线是否照到平面. (light_dir点乘法向量 > 0)
Vec3f light_dir = Vec3f(1, 1, 1);


// 采用Gourand着色模型的着色器
class GouraudShader : public mygl::IShader
{
public:
    // written by vertex shader, read by fragment shader
    Vec3f varying_intensity;

    // 用矩阵存三个点的纹理坐标, written by vertex shader, read by fragment shader
    Matrix<2, 3, float> varying_uv;

    Matrix<4, 4, float> uniform_M;   //  Projection*ModelView
    Matrix<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()

    // iface为三角形编号, nthvert为顶点编号, 返回屏幕坐标
    virtual Vec4f vertex(int iface, int nthvert) override
    {
        using namespace mygl;

        varying_uv.set_col(nthvert, model->texture(iface, nthvert));
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir); // get diffuse lighting intensity

        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert), 1.0f); // read the vertex from .obj file
        return viewport *  projection *  modelView * gl_Vertex; // transform it to screen coordinates
    }

    // bar为当前像素相对于三角形的重心坐标, color为当前像素的颜色, 返回是否丢弃该像素
    virtual bool fragment(Vec3f bar, TGAColor &color) override
    {
        // 通过重心坐标插值计算当前点的纹理坐标
        Vec2f uv = varying_uv * bar;
        // 法线
        Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
        // 光的方向
        Vec3f l = proj<3>(uniform_M   * embed<4>(light_dir        )).normalize();
        // 反射光方向
        Vec3f r = (n * (n * l * 2.f) - l).normalize();

        // specular镜面反射
        float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
        // 漫反射, 即intensity
        float diff = std::max(0.f, n * l);

        // color = TGAColor(255, 255, 255) * intensity;
        color = model->getTexture(uv) * diff;
        for (int i = 0; i < 3; i ++)
            // 环境分量系数取5, 漫反射分量系数取1, 镜面反射分量取0.6, 但是通常系数之和要等于1
            color[i] = std::min<float>(5 + color[i] * (1 * diff + 0.6f * spec), 255);

        // 是否丢弃该像素
        return false;
    }
};

int main(int argc, char **argv)
{
    model = std::make_unique<Model>("../data/african_head.obj");

    mygl::viewMatrix(cameraPos, lookPos, upPos);
    mygl::viewportMatrix(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    mygl::projectionMatrix( -1.f / (cameraPos - lookPos).norm());
    light_dir.normalize();

    TGAImage image  (width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    GouraudShader shader;
    shader.uniform_M   = mygl::projection * mygl::modelView;
    // shader.uniform_MIT = (mygl::projection * mygl::modelView).invert_transpose();
    shader.uniform_MIT = Matrix4f();
    shader.uniform_MIT[0][0] = 1;
    shader.uniform_MIT[0][1] = 0;
    shader.uniform_MIT[0][2] = 0;
    shader.uniform_MIT[0][3] = 0;

    shader.uniform_MIT[1][0] = 0;
    shader.uniform_MIT[1][1] = 1;
    shader.uniform_MIT[1][2] = 0;
    shader.uniform_MIT[1][3] = 0;

    shader.uniform_MIT[2][0] = 0;
    shader.uniform_MIT[2][1] = 0;
    shader.uniform_MIT[2][2] = 1;
    shader.uniform_MIT[2][3] = 0;

    shader.uniform_MIT[3][0] = 0;
    shader.uniform_MIT[3][1] = 0;
    shader.uniform_MIT[3][2] = 0.333333;
    shader.uniform_MIT[3][3] = 1;

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
