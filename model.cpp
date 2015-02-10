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
    if(line.compare(0, 2, "v ") == 0) {
      Vec3f v;
      iss >> trash >> v.x >> v.y >> v.z;
      verts_.push_back(v);
    } else if(line.compare(0, 3, "vn ") == 0) {
      Vec3f vn;
      iss >> trash >> trash >> vn.x >> vn.y >> vn.z;
      normals_.push_back(vn);
    } else if(line.compare(0, 3, "vt ") == 0) {
      Vec2f vt;
      iss >> trash >> trash >> vt.x >> vt.y;
      uv_.push_back(vt);
    } else if(line.compare(0, 2, "f ") == 0) {
      std::vector<Vec3i> f;
      Vec3i tmp;
      iss >> trash;
      while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
	for (int i=0; i<3; i++) tmp[i]--; // in wavefront obj all indices start at 1, not zero
	f.push_back(tmp);
      }
      faces_.push_back(f);
    }
  }
  load_texture(path, "_diffuse.tga", diffusemap_);
  load_texture(path, "_nm.tga", normalmap_);
  load_texture(path, "_spec.tga", specmap_);
  printf("nb verts : %d, nb faces : %d, nb normals : %d\n", this->nverts(), this->nfaces(), this->nnormals());
}

Model::~Model(){}

int Model::nverts() {
  return (int)verts_.size();
}

int Model::nfaces() {
  return (int)faces_.size();
}

int Model::nnormals() {
  return (int)normals_.size();
}

std::vector<Vec3i> Model::face(int idx) {
  return faces_[idx];
}

Vec3f Model::vert(int i) {
  return verts_[i];
}

Vec3f Model::normal(int face, int vert) {
  int idx = faces_[face][vert][2];
  return normals_[idx];
}

Vec2i Model::uv(int face, int vert) {
  int idx = faces_[face][vert][1];
  return Vec2i(uv_[idx].x * diffusemap_.get_width(), uv_[idx].y * diffusemap_.get_height());
}

Vec3f Model::normal(Vec2i uv) {
  TGAColor c = normalmap_.get(uv.x, uv.y);
  Vec3f res;
  res[2] = (float)c.r / 255.f * 2.f - 1.f;
  res[1] = (float)c.g / 255.f * 2.f - 1.f;
  res[0] = (float)c.b / 255.f * 2.f - 1.f;
  return res;
}

void Model::load_texture(std::string filename, const char *suffix, TGAImage &img) {
  std::string texfile(filename);
  size_t dot = texfile.find_last_of(".");
  if (dot != std::string::npos) {
    texfile = texfile.substr(0,dot) + std::string(suffix);
    std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
    img.flip_vertically();
  }
}

TGAColor Model::diffuse(Vec2i uv) {
  return diffusemap_.get(uv.x, uv.y);
}

float Model::spec(Vec2i uv) {
  return specmap_.get(uv.x, uv.y).b / 1.f;
}
