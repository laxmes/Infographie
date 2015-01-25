#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char * path) {
  std::ifstream in;
  in.open(path, std::ifstream::in);
  if(in.fail()) return;
  std::string line;
  while(! in.eof()) {
    std::getline(in, line);
    std::istringstream iss(line.c_str());
    char trash;
    if(line.compare(0,2, "v ") == 0) {
      Vec3f v;
      iss >> trash >> v.x >> v.y >> v.z;
      verts_.push_back(v);
    } else if(line.compare(0, 2, "f ") == 0) {
      std::vector<int> f;
      int itrash, idx;
	iss >> trash;
      while(iss >> idx >> trash >> itrash >> trash >> itrash) {
	idx--;
	f.push_back(idx);
      }
      faces_.push_back(f);
    }
  }
  printf("nb verts : %d, nb faces : %d\n", this->nverts(), this->nfaces());
}

Model::~Model(){}

int Model::nverts() {
return (int)verts_.size();
}

int Model::nfaces() {
return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
return faces_[idx];
}

Vec3f Model::vert(int i) {
return verts_[i];
}
