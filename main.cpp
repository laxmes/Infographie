#include "tgaimage.h"
#include <cmath>
#include <stdlib.h>
#include <limits>
#include "model.h"
#include "geometry.h"

const int WIDTH  = 800;
const int HEIGHT = 800;
const int DEPTH  = 500;

Model * model = NULL;
int * zbuffer = NULL;

Vec3f light  = Vec3f(0., 0., 1.);
Vec3f camera = Vec3f(0., 0.5, 1.);
Vec3f center = Vec3f(0.,0.,0.);
Vec3f up     = Vec3f(0.,1.,0);


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
      Vec3f Pz;
      Pz.x = j;
      Pz.y = i + p0.y;
      Pz.z = A.z + ((B.z - A.z) * coef);
      float nP = nA + (nB - nA) * coef;
      int id = ((p0.y + i) * WIDTH) + j;
      if(Pz.z > zbuffer[id]) {
	zbuffer[id] = Pz.z;
	nP = (nP < 0.) ? 0. : (nP > 1.) ? 1.: nP ;
	image.set(Pz.x, Pz.y, TGAColor(255 * nP, 255 * nP, 255 * nP, 255));
      }
    }
  }
}

Matrix lookat(Vec3f camera, Vec3f up, Vec3f center) {
  Vec3f z = (camera - center).normalize();
  Vec3f x = (up ^ z).normalize();
  Vec3f y = z ^ x;

  Matrix base = Matrix();
  base.identity();
  for(int i = 0; i < 3; i++) {
    base[0][i] = x[i];
    base[1][i] = y[i];
    base[2][i] = z[i];
    base[i][3] = -center[i];
  }

  return base;
}

Matrix projection(float coef) {
  Matrix matrix = Matrix();
  matrix.identity();
  matrix[3][2] = coef;
  return matrix;
}

Matrix viewport(int x, int y, int w, int h, int p) {
  Matrix vp = Matrix();
  vp.identity();
  vp[0][0] = w / 2.f;
  vp[1][1] = h / 2.f;
  vp[2][2] = p / 2.f;

  vp[0][3] = x + (w/2.f);
  vp[1][3] = y + (h/2.f);
  vp[2][3] = p / 2.f;

  return vp;
}

int main() {
  TGAImage image(WIDTH, HEIGHT, 3);
  //triangle(image, Vec3i(0,10,0), Vec3i(50,50,0), Vec3i(10,10,0), TGAColor(100,100,100,255), NULL);


  model = new Model("./obj/african_head.obj");
  zbuffer = new int[WIDTH * HEIGHT];
  for(int i = 0; i < WIDTH * HEIGHT; i++) {
    zbuffer[i] = std::numeric_limits<int>::min();
  }

  Matrix look       = lookat(camera, up, center);
  Matrix projection = projection(-1.f / (camera - center).norm());
  Matrix viewPort   = viewport(WIDTH / 8, HEIGHT / 8, WIDTH * 3/4, HEIGHT * 3/4, DEPTH);

  for(int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    float intensity[3];
    Vec3i screen_coords[3];
    for(int j = 0; j < 3; j++) {
      Vec3f v = model->vert(face[j]);
      screen_coords[j] = Vec3f(viewPort * projection * modelView * Matrix(v));
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
