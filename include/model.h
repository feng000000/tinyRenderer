#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<std::pair<int, int>>> faces_;
	std::vector<Vec2f> textures_;
    TGAImage textureMap;
	Vec2i vtexture(std::vector<Vec2f> &vts, Vec3f &bc_screen);
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int ntextures();
	Vec3f vert(int idx);
	std::vector<std::pair<int, int>> face(int idx);
	Vec2f texture(int idx);
	void load_texture(std::string filename);
	TGAColor getTexture(std::vector<Vec2f> &vts, Vec3f &bc_screen);
};

#endif //__MODEL_H__