#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
 public:
  Model(const char * path);
  ~Model();
  int nverts();
  int nfaces();
  int nnormals();
  Vec3f vert(int i);
  Vec3f normal(int face, int vert);
  Vec3f normal(Vec2i uv);
  Vec2i uv(int face, int vert);
  float spec(Vec2i uv);
  std::vector<Vec3i> face(int idx);
  TGAColor diffuse(Vec2i uv);
  void load_texture(std::string filename, const char *suffix, TGAImage &image);
 private:
  std::vector<Vec3f> verts_;
  std::vector<Vec3f> normals_;
  std::vector<std::vector<Vec3i> > faces_;
  std::vector<Vec2f> uv_;
  TGAImage diffusemap_;
  TGAImage normalmap_;
  TGAImage specmap_;
};

#endif
