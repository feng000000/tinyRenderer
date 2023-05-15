#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"


const int width = 800;
const int height = 800;
const int depth = 255;

std::unique_ptr<Model> model;
float zbuffer[width * height];
Vec3f cameraPos(0, 0, 3);


Matrix vertex2homo(Vec3f v)
{
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.0f;
    return m;
}

Vec3f home2vertex(Matrix m)
{
    return Vec3f(
        m[0][0] / m[3][0],
        m[1][0] / m[3][0],
        m[2][0] / m[3][0]);
}

Matrix viewport(int x, int y, int w, int h)
{
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = depth / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = depth / 2.f;

    return m;
}

Matrix projectionMatrix()
{
    Matrix p = Matrix::identity(4);

    p[3][2] = -1.f / cameraPos.z;

    return p;
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P)
{
    // https://www.notion.so/8b3a4d1ac81f4beaac8c1a77621f7fd5?pvs=4
    // P点相对于三角形的重心坐标
    // (AC_x, AB_x, PA_x) 叉乘 (AC_y, AB_y, PA_y)
    Vec3f u = Vec3f(
                  C[0] - A[0],
                  B[0] - A[0],
                  A[0] - P[0]) ^
              Vec3f(
                  C[1] - A[1],
                  B[1] - A[1],
                  A[1] - P[1]);

    if (std::abs(u[2]) > 1e-2) // u[2] 是整数. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec3f *pts, std::vector<Vec2f> &vts, TGAImage &image, float intensity)
{
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }

    TGAColor color;
    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);

            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;

            P.z = 0;
            for (int i = 0; i < 3; i++)
                P.z += pts[i][2] * bc_screen[i];


            if (zbuffer[int(P.x + P.y * width)] <= P.z)
            {
                color = model->getTexture(vts, bc_screen);
                color = color *  intensity;

                zbuffer[int(P.x + P.y * width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

Vec3f normal(Vec3f p)
{
    Vec3i u(
        int(p.x + 0.5f),
        int(p.y + 0.5f),
        int(p.z + 0.5f)
    );
    return Vec3f(u);
}

int main(int argc, char **argv)
{
    model = std::make_unique<Model>("../data/african_head.obj");
    model->load_texture("../data/african_head_diffuse.tga");

    for (int i = 0; i < width * height; i ++)
        zbuffer[i] = -std::numeric_limits<float>::max();

    TGAImage image(width, height, TGAImage::RGB);
    Vec3f light_dir(0, 0, -1);
    Matrix projection = projectionMatrix();
    Matrix viewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

    for (int i = 0; i < model->nfaces(); i ++)
    {
        std::vector<std::pair<int, int>> face = model->face(i);
        Vec3f pts[3];
        Vec3f world_coords[3];
        std::vector<Vec2f> vts;

        for (int j = 0; j < 3; j ++)
        {
            Vec3f v = model->vert(face[j].first);
            Vec2f vt = model->texture(face[j].second);

            world_coords[j] = v;

            pts[j] = normal(home2vertex(viewPort * projection * vertex2homo(v)));

            // TODO: 浮点数要四舍五入, 不然有的三角形渲染不出来?
            // pts[j] =  Vec3f(Vec3i(home2vertex(viewPort * projection * vertex2homo(v))));

            vts.push_back(vt);
        }

        Vec3f n = (world_coords[2] - world_coords[0]) ^
                  (world_coords[1] - world_coords[0]);
        n.normalize();

        float intensity = n * light_dir;

        if (intensity > 0)
            triangle(pts, vts, image, intensity);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}
