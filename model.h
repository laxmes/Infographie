#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "geometry.h"

class Model {
public:
  Model(const char * path);
  ~Model();
  int nverts();
  int nfaces();
  Vec3f vert(int i);
  std::vector<int> face(int idx);
 private:
  std::vector<Vec3f> verts_;
  std::vector<std::vector<int> > faces_;
};

#endif
