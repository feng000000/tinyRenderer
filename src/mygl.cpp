#include <algorithm>
#include "mygl.h"

Matrix4f mygl::modelView;
Matrix4f mygl::viewport;
Matrix4f mygl::projection;

// 求点P相对于三角形ABC的重心坐标
static Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P)
{
    Vec3f u = Vec3f(C[0] - A[0], B[0] - A[0], A[0] - P[0]) ^
              Vec3f(C[1] - A[1], B[1] - A[1], A[1] - P[1]);

    if (std::abs(u[2]) > 1e-2) // u[2] 是整数. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1);
}

// 视口变换矩阵, 将点变换到二维屏幕上
void mygl::viewportMatrix(int x, int y, int w, int h)
{
    viewport = Matrix4f::identity();
    viewport[0][3] = x + w / 2.f;
    viewport[1][3] = y + h / 2.f;
    viewport[2][3] = 255.f / 2.f;

    viewport[0][0] = w / 2.f;
    viewport[1][1] = h / 2.f;
    viewport[2][2] = 255.f / 2.f;
}

// 透视投影矩阵
void mygl::projectionMatrix(float coeff)
{
    projection = Matrix4f::identity();
    projection[3][2] = coeff;
}

// 视图变换矩阵, 将世界坐标系的点转换成相机坐标系的点
void mygl::viewMatrix(Vec3f cameraPos, Vec3f lookPos, Vec3f upDir)
{
    cameraPos.normalize();
    // 右手系
    Vec3f z = (cameraPos - lookPos).normalize();
    Vec3f x = (upDir ^ z).normalize();
    Vec3f y = (z ^ x).normalize();

    modelView = Matrix4f::identity();
    for (int i = 0; i < 3; i ++)
    {
        modelView[0][i] = x[i];
        modelView[1][i] = y[i];
        modelView[2][i] = z[i];
        modelView[i][3] = -lookPos[i];
    }
}


void mygl::triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer)
{
    Vec2f bboxmin(std::numeric_limits<float>::max(),
                  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(),
                  -std::numeric_limits<float>::max());

    for (int i = 0; i < 3; i ++)
    {
        for (int j = 0; j < 2; j ++)
        {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
        }
    }

    Vec2i P;
    TGAColor color;
    int cnt = 10;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x ++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y ++)
        {
            Vec3f c = barycentric(proj<2>(pts[0] / pts[0][3]),
                                  proj<2>(pts[1] / pts[1][3]),
                                  proj<2>(pts[2] / pts[2][3]),
                                  proj<2>(P));

            if (c.x < 0 || c.y < 0 || c.z < 0) continue;

            // 插值计算z坐标: z坐标插值 / 齐次坐标插值
            float z = 0.f;
            float w = 0.f;
            for (int i = 0; i < 3; i ++)
            {
                z += pts[i][2] * c[i];
                w += pts[i][3] * c[i];
            }

            int frag_depth = std::max(0, std::min(255, int(z / w + 0.5f)));
            if (zbuffer.get(P.x, P.y)[0] > frag_depth) continue;

            bool discard = shader.fragment(c, color);
            if (!discard)
            {
                zbuffer.set(P.x, P.y, TGAColor(frag_depth));
                image.set(P.x, P.y, color);
            }
        }
    }
}

mygl::IShader::~IShader() {}