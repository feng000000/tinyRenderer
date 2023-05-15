#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_()
{
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail())
        return;
    std::string line;
    while (!in.eof())
    {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        int itrash;
        std::string strash;
        if (!line.compare(0, 2, "v "))
        {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++)
                iss >> v.raw[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 2, "f "))
        {
            std::vector<std::pair<int, int>> f;
            iss >> trash;
            int idx, vtidx;
            while (iss >> idx >> trash >> vtidx >> trash >> itrash)
            {
                idx--; // in wavefront obj all indices start at 1, not zero
                vtidx--;
                f.push_back({idx, vtidx});
            }
            faces_.push_back(f);
        }
        else if (!line.compare(0, 4, "vt  "))
        {
            iss >> strash;
            Vec2f t;
            for (int i = 0; i < 2; i++)
                iss >> t.raw[i];
            textures_.push_back(t);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
    std::cerr << "# vt# " << textures_.size() << std::endl;
}

Model::~Model()
{
}

int Model::nverts()
{
    return (int)verts_.size();
}

int Model::nfaces()
{
    return (int)faces_.size();
}

int Model::ntextures()
{
    return (int)textures_.size();
}

std::vector<std::pair<int, int>> Model::face(int idx)
{
    return faces_[idx];
}

Vec3f Model::vert(int idx)
{
    return verts_[idx];
}

Vec2f Model::texture(int idx)
{
    return textures_[idx];
}

void Model::load_texture(std::string filename)
{
    this->textureMap.read_tga_file(filename.c_str());
    this->textureMap.flip_vertically();
}

// 计算纹理坐标
// vts为三角形三个点的纹理坐标
// bc_screen为当前点相对于三角形的重心坐标
// 返回纹理图上的对应的具体坐标
Vec2i Model::vtexture(std::vector<Vec2f> &vts, Vec3f &bc_screen)
{
    Vec2f res(0.0f, 0.0f);

    // 计算插值 (相对于三角形的重心坐标) 得出P点对应的纹理坐标
    for (int i = 0; i < 3; i++)
    {
        res[0] += vts[i][0] * bc_screen[i];
        res[1] += vts[i][1] * bc_screen[i];
    }

    // 乘上宽/高才是具体的坐标
    return Vec2i(
        res[0] * this->textureMap.get_width(),
        res[1] * this->textureMap.get_height());
}

TGAColor Model::getTexture(std::vector<Vec2f> &vts, Vec3f &bc_screen)
{
    Vec2i pos = this->vtexture(vts, bc_screen);
    return this->textureMap.get(pos.x, pos.y);
}