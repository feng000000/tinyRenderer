#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

// ------------------- Model Class ------------------- //

Model::Model(const char *filename) : verts_(), faces_(), normals_()
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
                iss >> v[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 2, "f "))
        {
            std::vector<Vec3i> f;
            iss >> trash;
            int idx, vtidx, nidx;
            vector<Vec3f> nVerts;
            vector<Vec3f> nNorms;
            vector<Vec2f> nTextures;
            while (iss >> idx >> trash >> vtidx >> trash >> nidx)
            {
                idx --; // in wavefront obj all indices start at 1, not zero
                vtidx --;
                nidx --;
                nVerts.push_back(verts_[idx]);
                nTextures.push_back(textures_[vtidx]);
                nNorms.push_back(normals_[nidx]);
            }

            faces_.push_back(Trangle(nVerts, nNorms, nTextures));
        }
        else if (!line.compare(0, 4, "vt  "))
        {
            iss >> strash;
            Vec2f t;
            for (int i = 0; i < 2; i++)
                iss >> t[i];
            textures_.push_back(t);
        }
        else if (!line.compare(0, 4, "vn  "))
        {
            iss >> strash;
            Vec3f t;
            for (int i = 0; i < 3; i ++)
                iss >> t[i];
            normals_.push_back(t);
        }
    }
    std::cerr << "# v# " << verts_.size() << std::endl;
    std::cerr << "# f# " << faces_.size() << std::endl;
    std::cerr << "# vt# " << textures_.size() << std::endl;
    std::cerr << "# vn# " << normals_.size() << std::endl;

    load_texture("/home/feng/Code/tinyRenderer/data/african_head_diffuse.tga", this->textureMap);
    load_texture("/home/feng/Code/tinyRenderer/data/african_head_spec.tga", this->specularMap);
    load_texture("/home/feng/Code/tinyRenderer/data/african_head_nm.tga", this->normalMap);
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

Trangle Model::face(int idx)
{
    return faces_[idx];
}

// 获取顶点坐标, 参数为三角形编号和顶点编号
Vec3f Model::vert(int iface, int ivert)
{
    return faces_[iface].nVert(ivert);
}

// 获取法线向量, 参数为三角形编号和顶点编号(从obj文件中读到的法线坐标)
Vec3f Model::normal(int iface, int ivert)
{
    return faces_[iface].nNorm(ivert);
}

// 获取法线向量(从法线贴图获取)
Vec3f Model::normal(Vec2f& uvf)
{
    TGAColor c = normalMap.get(uvf[0] * normalMap.getWidth(), uvf[1] * normalMap.getHeight());
    // 切线方向范围为 (-1, 1)映射到了(0, 255), 要将它映射回来
    return Vec3f{(float)c[2], (float)c[1], (float)c[0]} * 2.f / 255.f - Vec3f{1, 1, 1};
}

// 获取纹理坐标, 参数为三角形编号和顶点编号
Vec2f Model::texture(int iface, int ivert)
{
    return faces_[iface].nTexture(ivert);
}


void Model::load_texture(std::string filename, TGAImage& img)
{
    bool status = img.read_tga_file(filename.c_str());
    std::cout << "load " << filename << " status: "
              << (status ? "ok" : "failed") << std::endl;
    img.flip_vertically();
}


// 获取纹理, 参数为纹理坐标
TGAColor Model::getTexture(Vec2f uv)
{
    return this->textureMap.get(
        uv[0] * this->textureMap.get_width(),
        uv[1] * this->textureMap.get_height());
}


int Model::nnormals()
{
    return int(normals_.size());
}

// --------------------  Trangle Class -------------------- //

Trangle::Trangle(vector<Vec3f> verts, vector<Vec3f> norms, vector<Vec2f> textures)
{
    for (int i = 0; i < 3; i ++)
    {
        verts_[i]    = (i >= verts.size())    ? Vec3f() : (verts[i]);
        norms_[i]    = (i >= norms.size())    ? Vec3f() : (norms[i]);
        textures_[i] = (i >= textures.size()) ? Vec2f() : (textures[i]);
    }
}


void Trangle::setVerts(vector<Vec3f> verts)
{
    for (int i = 0; i < 3; i ++)
        verts_[i] = (i >= verts.size()) ? Vec3f() : (verts[i]);
}


void Trangle::setNorms(vector<Vec3f> norms)
{
    for (int i = 0; i < 3; i ++)
        norms_[i] = (i >= norms.size()) ? Vec3f() : (norms[i]);
}


void Trangle::setTextures(vector<Vec2f> textures)
{
    for (int i = 0; i < 3; i ++)
        textures_[i] = (i >= textures.size()) ? Vec2f() : (textures[i]);
}


Vec3f Trangle::nVert(int idx)
{
    return verts_[idx];
}


Vec3f Trangle::nNorm(int idx)
{
    return norms_[idx];
}


Vec2f Trangle::nTexture(int idx)
{
    return textures_[idx];
}

// 从镜面反射贴图中获取镜面反射分;量
float Model::specular(Vec2f uvf)
{
    Vec2i uv(uvf[0]*specularMap.get_width(), uvf[1]*specularMap.get_height());
    return specularMap.get(uv[0], uv[1])[0]/1.f;
}