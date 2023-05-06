#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<std::pair<int, int>>> faces_;
	std::vector<Vec3f> textures_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int ntextures();
	Vec3f vert(int idx);
	std::vector<std::pair<int, int>> face(int idx);
	Vec3f texture(int idx);
};

#endif //__MODEL_H__