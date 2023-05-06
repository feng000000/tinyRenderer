// #include <iostream>
#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const int width = 800;
const int height = 800;
Model *model = NULL;

// pts 为存储三角形三个点坐标的 vec2<int> 类型的数组
// p 为点的坐标
Vec3f barycentric(Vec2i *pts, Vec2i P)
{
    // https://www.notion.so/8b3a4d1ac81f4beaac8c1a77621f7fd5?pvs=4
    // P点相对于三角形的重心坐标
    // (AC_x, AB_x, PA_x) 叉乘 (AC_y, AB_y, PA, y)
    Vec3f u = Vec3f(
        pts[2][0] - pts[0][0],
        pts[1][0] - pts[0][0],
        pts[0][0] - P[0]
    ) ^ Vec3f(
        pts[2][1] - pts[0][1],
        pts[1][1] - pts[0][1],
        pts[0][1] - P[1]
    );

    // `pts` and `P` has integer value as coordinates,
    // so `abs(u[2])` < 1 means `u[2]` is 0, that means
    // triangle is degenerate, in this case return
    // something with negative coordinates.
    if (std::abs(u.z) < 1)
        return Vec3f(-1, 1, 1);

    return Vec3f(
        1.f - (u.x + u.y) / u.z,
        u.y / u.z,
        u.x / u.z
    );
}

void triangle(Vec2i *pts, TGAImage &image, TGAColor color)
{
    Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
    Vec2i bboxmax(0, 0);
    Vec2i clamp(image.get_width() - 1, image.get_height() - 1);

    // 计算碰撞盒
    for (int i = 0; i < 3; i++)
    {
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }

    Vec2i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = barycentric(pts, P);


            // 我们渲染出来的模型上好像有一些黑色的小洞,
            // 这些小洞是黑色的，而且大都是位于一个三角形的边附近，
            // 也就是说我们在光栅化的时候这些点并没有被赋予颜色，
            // 而是直接被丢弃了（被我们的算法认为在三角形外).
            // 这里的问题就是浮点数比较精度的问题，我们可以将比较的值改为一个比较小的负数，
            // 比如-0.1, 就可以得到比较好的结果了

            // if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
            //     continue;
            if (bc_screen.x < -0.1 || bc_screen.y < -0.1 || bc_screen.z < -0.1)
                continue;
            image.set(P.x, P.y, color);
        }
    }
}

int main(int argc, char **argv)
{
    if (2 == argc)
        model = new Model(argv[1]);
    else
        model = new Model("../obj/african_head.obj");

    TGAImage image(width, height, TGAImage::RGB);
    Vec3f light_dir(0,0,-1);

    for (int i = 0; i < model->nfaces(); i++)
    {
        // printf("traverse faces %d\n", i);
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];

        for (int j = 0; j < 3; j++)
        {
            Vec3f v = model->vert(face[j]);

            world_coords[j] = v;

            // 将三维的点沿z轴映射到 xoy 平面, 即 令 x' = x, y' = y.
            // 然后向右移动1个像素, 向上移动一个像素.
            // 然后横向放大 width / 2 倍, 纵向放大 height / 2 倍.
            screen_coords[j] = Vec2i(
                (v.x + 1.) * width / 2.,
                (v.y + 1.) * height / 2.
            );
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
            // 我们将对颜色执行线性计算.
            // 但是 (128,128,128) 颜色的亮度不及 (255, 255, 255) 的一半.
            // 我们将忽略伽马校正并容忍颜色亮度的不正确.
            triangle(
                screen_coords,
                image,
                TGAColor(intensity*255, intensity*255, intensity*255, 255)
            );
        }
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}
