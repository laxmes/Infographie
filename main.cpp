#include "tgaimage.h"
#include <cmath>
#include <stdlib.h>
#include <limits>
#include "model.h"
#include "geometry.h"

const int width  = 800;
const int height = 800;
const int depth  = 500;

Model * model = NULL;
int * zbuffer = NULL;
Vec3f light = Vec3f(0., 0., 1.);

void triangle(TGAImage &image, Vec3i p0, Vec3i p1, Vec3i p2, float intensity[], int * zbuffer) {
  if(p0.y == p1.y && p0.y == p2.y) return;
  if(p0.y > p1.y) { std::swap(p0, p1); std::swap(intensity[0], intensity[1]);}
  if(p0.y > p2.y) { std::swap(p0, p2); std::swap(intensity[0], intensity[2]);}
  if(p1.y > p2.y) { std::swap(p1, p2); std::swap(intensity[1], intensity[2]);}

  int distance_p0p2 = p2.y - p0.y;
  for(int i = 0; i <= distance_p0p2; i++) {
    bool inverted = (i + p0.y > p1.y) || p0.y == p1.y;

    float coef0 = (float) i / distance_p0p2;
    float coef1 = (float) (i - ((inverted) ? p1.y - p0.y : 0)) / ((inverted) ? p2.y - p1.y : p1.y - p0.y);

    Vec3i A = p0 + ((p2 - p0) * coef0);
    Vec3i B = ((inverted) ?
	       p1 + ((p2 - p1) * coef1) :
	       p0 + ((p1 - p0) * coef1));
    float nA = intensity[0] + ((intensity[2] - intensity[0]) * coef0);
    float nB = ((inverted) ?
		intensity[1] + ((intensity[2] - intensity[1]) * coef1) :
		intensity[0] + ((intensity[1] - intensity[0]) * coef1));

    if(A.x > B.x) { std::swap(A, B); std::swap(nA, nB);}
    for(int j = A.x; j <= B.x; j++) {
      float coef = (B.x == A.x) ? 1. : (float) (j - A.x) / (float) (B.x - A.x);
      float Pz = A.z + ((B.z - A.z) * coef);
      float nP = nA + (nB - nA) * coef;
      int id = ((p0.y + i) * width) + j;
      if(Pz > zbuffer[id]) {
	zbuffer[id] = Pz;
	nP = (nP < 0.) ? 0. : (nP > 1.) ? 1.: nP ;
	image.set(j, i + (int) p0.y, TGAColor(255 * nP, 255 * nP, 255 * nP, 255));
      }
    }
  }
}

int main() {
  TGAImage image(width, height, 3);
  //triangle(image, Vec3i(0,10,0), Vec3i(50,50,0), Vec3i(10,10,0), TGAColor(100,100,100,255), NULL);


  model = new Model("./obj/african_head.obj");
  zbuffer = new int[width * height];
  for(int i = 0; i < width * height; i++) {
    zbuffer[i] = std::numeric_limits<int>::min();
  }


  for(int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    float intensity[3];
    Vec3i screen_coords[3];
    for(int j = 0; j < 3; j++) {
      Vec3f v = model->vert(face[j]);
      screen_coords[j] = Vec3i((v.x + 1) * width / 2., (v.y + 1) * height / 2., (v.z + 1) * depth / 2.);
    }

    for(int j = 3; j < 6; j++) intensity[j - 3] = model->normal(face[j]).normalize() * light;

    triangle(image, screen_coords[0], screen_coords[1], screen_coords[2], intensity, zbuffer);
  }

  delete model;
  delete [] zbuffer;

  image.flip_vertically();
  image.write_tga_file("dump.tga");

  return 0;
}
