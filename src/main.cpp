#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
std::unique_ptr<Model> model;
std::unique_ptr<TGAImage> TextureImage(new TGAImage);
const int width = 800;
const int height = 800;


// 世界坐标转屏幕坐标, 但保留了z轴的值
Vec3f world2screen(Vec3f v)
{
    return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}


// 返回点P相对于三角形ABC的重心坐标
// P = (1 - u - v)*A + u*B + v*C
// return {1 - u - v, u, v}
// 传入的是三维的点, 但是忽略z轴
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P)
{
    // https://www.notion.so/8b3a4d1ac81f4beaac8c1a77621f7fd5?pvs=4
    // P点相对于三角形的重心坐标
    // (AC_x, AB_x, PA_x) 叉乘 (AC_y, AB_y, PA_y)
    Vec3f u = Vec3f(
        C[0] - A[0],
        B[0] - A[0],
        A[0] - P[0]
    ) ^ Vec3f(
        C[1] - A[1],
        B[1] - A[1],
        A[1] - P[1]
    );

    if (std::abs(u[2]) > 1e-2) // u[2] 是整数. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}


void triangle(Vec3f *pts, std::vector<Vec2f>& vts,float *zbuffer,
    TGAImage &image, std::unique_ptr<TGAImage> &textureImage, float intensity)
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

            // 判断点P是否在三角形pts中
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;

            // 通过 "三角形三个顶点坐标" 和 "P相对于三角形的重心坐标" 计算 "P的z坐标"
            // P = (1 - u - v)*A + u*B + v*C
            // Pz = (1 - u - v)*Az + u*Bz + v*Cz
            P.z = 0;
            for (int i = 0; i < 3; i++)
                P.z += pts[i][2] * bc_screen[i];

            // 通过zbuffer判断是否更新点
            if (zbuffer[int(P.x + P.y * width)] < P.z)
            {
                // 计算纹理坐标
                float tx = 0.0, ty = 0.0;

                // 计算插值 (相对于三角形的重心坐标) 得出P点对应的纹理坐标
                for (int i = 0; i < 3; i ++)
                {
                    tx += vts[i][0] * bc_screen[i];
                    ty += vts[i][1] * bc_screen[i];
                }

                // 乘上宽/高才是具体的坐标
                tx *= textureImage->get_width();
                ty *= textureImage->get_height();


                color = textureImage->get(tx, ty);
                color = color * intensity;

                zbuffer[int(P.x + P.y * width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}


int main(int argc, char **argv)
{
    model = std::make_unique<Model>("../data/african_head.obj");

    TextureImage->read_tga_file("../data/african_head_diffuse.tga");
    TextureImage->flip_vertically();

    float *zbuffer = new float[width * height];
    TGAImage image(width, height, TGAImage::RGB);
    Vec3f light_dir(0,0,-1);

    for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

    for (int i = 0; i < model->nfaces(); i ++)
    {
        std::vector<std::pair<int, int>> face = model->face(i);
        Vec3f pts[3];
        Vec3f world_coords[3];
        std::vector<Vec2f> vts;

        // obj文件中 f开头的行(f xx/xx/xx xx/xx/xx xx/xx/xx )
        // 代表了面片的每个点的坐标(第一个数是坐标, 第二个数是对应的纹理的编号)
        // vt 开头的行(vt  0.00001 0.000002 0.000000) 这里是二维的所以第三个数都为0.0
        // 代表了对应纹理图的位置, 数据范围是 (0, 1), 所以要乘上纹理图的宽/高.
        // 所以在渲染每个点时, 要根据该点对应的纹理坐标来获取该点的颜色
        for (int j = 0; j < 3; j++)
        {
            Vec3f v = model->vert(face[j].first);
            Vec2f vt = model->texture(face[j].second);

            world_coords[j] = v;
            pts[j] = world2screen(v);
            vts.push_back(vt);
        }

        //三角形法线
        Vec3f n = (world_coords[2] - world_coords[0]) ^
                  (world_coords[1] - world_coords[0]);

        // 设置为单位向量
        n.normalize();

        // 光的强度为光的角度和三角形法线的点乘结果, 夹角越大, 强度越弱
        float intensity = n * light_dir;

        // 强度为负数时, 光照不到三角形, 这样的三角形我们直接忽略.
        if (intensity > 0)
        {
            triangle(
                pts,
                vts,
                zbuffer,
                image,
                TextureImage,
                intensity
            );
        }
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output1.tga");
    return 0;
}
