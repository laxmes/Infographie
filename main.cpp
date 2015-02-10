#include "tgaimage.h"
#include <cmath>
#include <stdlib.h>
#include <limits>
#include "model.h"
#include "geometry.h"

const int WIDTH  = 1024;
const int HEIGHT = 1024;
const int DEPTH  = 1024;

Model * model = NULL;
int * zbuffer = NULL;

Vec3f light  = Vec3f(1, 1, -1).normalize();
Vec3f camera = Vec3f(0, 0, 5);
Vec3f center = Vec3f(0, 0, 0);
Vec3f up     = Vec3f(0, 1, 0);


void triangle(TGAImage &image, Vec3i p0, Vec3i p1, Vec3i p2, Vec2i texture[], int * zbuffer) {
  if(p0.y == p1.y && p0.y == p2.y) return;
  if(p0.y > p1.y) { std::swap(p0, p1); std::swap(texture[0], texture[1]);}
  if(p0.y > p2.y) { std::swap(p0, p2); std::swap(texture[0], texture[2]);}
  if(p1.y > p2.y) { std::swap(p1, p2); std::swap(texture[1], texture[2]);}
    
  int distance_p0p2 = p2.y - p0.y;
  for(int i = 0; i <= distance_p0p2; i++) {
    bool inverted = (i + p0.y > p1.y) || p0.y == p1.y;
        
    float coef0 = (float) i / distance_p0p2;
    float coef1 = (float) (i - ((inverted) ? p1.y - p0.y : 0)) / ((inverted) ? p2.y - p1.y : p1.y - p0.y);
        
    Vec3i A = p0 + ((p2 - p0) * coef0);
    Vec3i B = ((inverted) ?
	       p1 + ((p2 - p1) * coef1) :
	       p0 + ((p1 - p0) * coef1));
    Vec2i tA = texture[0] + ((texture[2] - texture[0]) * coef0);
    Vec2i tB = ((inverted) ?
		texture[1] + ((texture[2] - texture[1]) * coef1) :
		texture[0] + ((texture[1] - texture[0]) * coef1));
    if(A.x > B.x) { std::swap(A, B); std::swap(tA, tB);}
    for(int j = A.x; j <= B.x; j++) {
      float coef = (B.x == A.x) ? 1. : (float) (j - A.x) / (float) (B.x - A.x);
      Vec3f Pz;
      Pz.x = j;
      Pz.y = i + p0.y;
      Pz.z = A.z + ((B.z - A.z) * coef);
      Vec2i tP = tA + (tB - tA) * coef;
      int id = ((p0.y + i) * WIDTH) + j;
      if(Pz.z > zbuffer[id]) {
	Vec3f n = model->normal(tP).normalize();
	Vec3f reflect = (n * (n * light * 2.f) - light).normalize();
	float spec = pow(std::max(0.f,reflect.z), model->spec(tP));
	spec = std::min(1.f,spec);
	TGAColor color = model->diffuse(tP);
	zbuffer[id] = Pz.z;
	float nP = n*light;
	nP = (nP < 0.) ? 0. : (nP > 1.) ? 1.: nP ;
	color.r = std::min(5.f + color.r * (nP + (.9f * spec)), 255.f);
	color.g = std::min(5.f + color.g * (nP + (.9f * spec)), 255.f);
	color.b = std::min(5.f + color.b * (nP + (.9f * spec)), 255.f);
	image.set(Pz.x, Pz.y, color);
      }
    }
  }
}

Matrix lookat(Vec3f camera, Vec3f up, Vec3f center) {
  Vec3f z = (camera - center).normalize();
  Vec3f x = (up ^ z).normalize();
  Vec3f y = z ^ x;
    
  Matrix base = Matrix::identity();
  for(int i = 0; i < 3; i++) {
    base[0][i] = x[i];
    base[1][i] = y[i];
    base[2][i] = z[i];
    base[i][3] = -center[i];
  }
    
  return base;
}

Matrix projection(float coef) {
  Matrix matrix = Matrix::identity();
  matrix[3][2] = coef;
  return matrix;
}

Matrix viewport(int x, int y, int w, int h, int p) {
  Matrix vp = Matrix::identity();
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
  Matrix projectionMatrix = projection(-1.f / (camera - center).norm());
  Matrix viewPort   = viewport(WIDTH / 8, HEIGHT / 8, WIDTH * 3/4, HEIGHT * 3/4, DEPTH);
    
  for(int i = 0; i < model->nfaces(); i++) {
    std::vector<Vec3i> face = model->face(i);
    Vec2i texture[3];
    Vec3i screen_coords[3];
    for(int j = 0; j < 3; j++) {
      Vec3f v = model->vert(face[j][0]);
      Vec4f tmp = viewPort * projectionMatrix * look * embed<4>(v);
      screen_coords[j] = proj<3>(tmp/tmp[3]);
    }
        
    for(int j = 0; j < 3; j++) {
      texture[j] = model->uv(i, j);
    }
        
    triangle(image, screen_coords[0], screen_coords[1], screen_coords[2], texture, zbuffer);
  }
    
  delete model;
  delete [] zbuffer;
    
  image.flip_vertically();
  image.write_tga_file("dump.tga");
    
  return 0;
}
