#include <vector>
#include <iostream>
#include "geometry.h"
#include "tgaimage.h"

const int width = 200;
const int height = 200;

// 返回 p点相对于三角形的重心坐标
// 笔记: https://www.notion.so/8b3a4d1ac81f4beaac8c1a77621f7fd5
Vec3f barycentric(Vec2i *pts, Vec2i P)
    // pts 为存储三角形三个点坐标的 vec2<int> 类型的数组
    // p 为点的坐标
{
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
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;
            image.set(P.x, P.y, color);
        }
    }
}

int main(int argc, char **argv)
{
    TGAImage frame(200, 200, TGAImage::RGB);
    Vec2i pts[3] = {Vec2i(10, 10), Vec2i(100, 30), Vec2i(190, 160)};

    triangle(pts, frame, TGAColor(255, 0, 0, 255));

    frame.flip_vertically(); // to place the origin in the bottom left corner of the image
    frame.write_tga_file("framebuffer.tga");

    return 0;
}

// g++ drawTrangle.cpp tgaimage.cpp -I ../include