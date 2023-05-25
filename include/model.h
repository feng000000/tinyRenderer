#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

template <class t>
using vector = std::vector<t>;


class Trangle
{
public:
	Trangle();
	Trangle(vector<Vec3f> verts, vector<Vec3f> norms, vector<Vec2f> textures);
	void setVerts(vector<Vec3f> verts);
	void setNorms(vector<Vec3f> norms);
	void setTextures(vector<Vec2f> textures);
	Vec3f nVert(int idx);
	Vec3f nNorm(int idx);
	Vec2f nTexture(int idx);

	// Vec3f& operator[](int idx)
	// {
	// 	if (idx == 0) return
	// }
private:
	Vec3f verts_[3];
	Vec3f norms_[3];
	Vec2f textures_[3];
};

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> normals_;
	std::vector<Vec2f> textures_;
	std::vector<Trangle> faces_;

	TGAImage textureMap;
	Vec2i vtexture(std::vector<Vec2f> &vts, Vec3f &bc_screen);
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int ntextures();
	int nnormals();
	Trangle face(int idx);
	Vec3f vert(int iface, int ivert);
	Vec3f normal(int iface, int ivert);
	Vec2f texture(int iface, int ivert);
	void load_texture(std::string filename);
	TGAColor getTexture(std::vector<Vec2f> &vts, Vec3f &bc_screen);
};

#endif //__MODEL_H__