#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <cmath>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "camera.h"

template <class t>
using vector = std::vector<t>;

const int width = 800;
const int height = 800;
const int depth = 255;

std::unique_ptr<Model> model;
float zbuffer[width * height];
Vec3f cameraPos = Vec3f(0, 1, 3);
Vec3f lookPos   = Vec3f(0, 0, 0);
Vec3f upDir     = Vec3f(0, 1, 0);
Vec3f light_dir = Vec3f(1, -1, 1).normalize();
Camera camera(cameraPos, lookPos - cameraPos, upDir);


Matrix vertex2homo(Vec3f v)
{
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.0f;
    return m;
}

Vec3f homo2vertex(Matrix m)
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
    Vec3f u = Vec3f(C[0] - A[0], B[0] - A[0], A[0] - P[0]) ^
              Vec3f(C[1] - A[1], B[1] - A[1], A[1] - P[1]);

    if (std::abs(u[2]) > 1e-2) // u[2] 是整数. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1);
}


void triangle(Vec3f Intensitys, Vec3f *pts, vector<Vec2f> &vts, TGAImage &image)
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
                color = TGAColor(255, 255, 255);
                color = model->getTexture(vts, bc_screen);
                color = color * (Intensitys * bc_screen);

                zbuffer[int(P.x + P.y * width)] = P.z;

                image.set(P.x, P.y, color);
            }
        }
    }
}


Vec3f round(Vec3f p)
{
    return Vec3f(
        Vec3i(
            int(p.x + 0.5f),
            int(p.y + 0.5f),
            int(p.z + 0.5f)
    ));
}


int main(int argc, char **argv)
{
    model = std::make_unique<Model>("../data/african_head.obj");
    model->load_texture("../data/african_head_diffuse.tga");

    for (int i = 0; i < width * height; i ++)
        zbuffer[i] = -std::numeric_limits<float>::max();

    TGAImage image(width, height, TGAImage::RGB);
    Matrix projection   = projectionMatrix();
    Matrix viewPort     = viewport(width / 8, height / 8,
                                   width * 3 / 4, height * 3 / 4);

    for (int i = 0; i < model->nfaces(); i ++)
    {
        Trangle face = model->face(i);
        Vec3f pts[3];
        Vec3f world_coords[3];
        Vec3f Intensitys;
        vector<Vec2f> vts;

        for (int j = 0; j < 3; j ++)
        {
            Vec3f v          = model->vert(i, j);
            Vec2f vt         = model->texture(i, j);
            float intensity  = model->normal(i, j).normalize() * light_dir;

            world_coords[j]  = v;
            Intensitys[j]    = std::max(0.f, intensity);
            pts[j]           = round(homo2vertex(viewPort * projection * camera.viewMatrix() * vertex2homo(v)));
            // pts[j]          = round(homo2vertex(viewPort * projection * lookat(cameraPos, look, up) * vertex2homo(v)));

            vts.push_back(vt);
        }

        triangle(Intensitys, pts, vts, image);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}
