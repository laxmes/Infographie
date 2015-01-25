#include "tgaimage.h"
#include <cmath>
#include <stdlib.h>
#include <limits>
#include "model.h"
#include "geometry.h"

const int width  = 600;
const int height = 600;
const int depth  = 255;

Model * model = NULL;
int * zbuffer = NULL;
Vec3f light(0., 0., -1.);

void triangle(TGAImage &image, Vec6i p0, Vec6i p1, Vec6i p2, TGAColor color, int * zbuffer) {
	if(p0.y == p1.y && p0.y == p2.y) return;
	if(p0.y > p1.y) std::swap(p0, p1);
	if(p0.y > p2.y) std::swap(p0, p2);
	if(p1.y > p2.y) std::swap(p1, p2);

	int distance_p0p2 = p2.y - p0.y;
	for(int i = 0; i < distance_p0p2; i++) {
		bool inverted = (i + p0.y > p1.y) || p0.y == p1.y;

		float coef0 = (float) i / distance_p0p2;
		float coef1 = (float) (i - ((inverted) ? p1.y - p0.y : 0)) / ((inverted) ? p2.y - p1.y : p1.y - p0.y);

		Vec6i A = p0 + ((p2 - p0) * coef0);
		Vec6i B = ((inverted) ?
				p1 + ((p2 - p1) * coef1) :
				p0 + ((p1 - p0) * coef1));

		if(A.x > B.x) std::swap(A, B);
		for(int j = (int) A.x; j <= (int) B.x; j++) {
			float coef = (B.x == A.x) ? 1. : (float) (j - A.x) / (float) (B.x - A.x);
			Vec6i P = A + (B - A) * coef;
			int id = ((p0.y + i) * width) + j;
			if(P.z > zbuffer[id]) {
				//float intensity = Vec3f((float) P.nx, (float) P.ny, (float) P.nz).normalize() * light;
				float intensity = 1 - Vec3f((float) P.x, (float) P.y, (float) P.z).normalize() * light;
				zbuffer[id] = P.z;
				if(intensity > 0)
					image.set(j, i + (int) p0.y, TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, color.a));
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
		Vec6i screen_coords[3];
		for(int j = 0; j < 3; j++) {
			Vec3f v = model->vert(face[j]);
			Vec3f nv = model->normal(face[j]);
			screen_coords[j] = Vec6i(
					Vec3i((v.x + 1) * width / 2., (v.y + 1) * height / 2., (v.z + 1) * depth / 2.),
					Vec3i((nv.x + 1) * width / 2., (nv.y + 1) * height / 2., (nv.z + 1) * depth / 2.));
		}

		triangle(image, screen_coords[0], screen_coords[1], screen_coords[2], TGAColor(255, 255, 255, 255), zbuffer);
	}

	delete model;
	delete [] zbuffer;

	image.flip_vertically();
	image.write_tga_file("dump.tga");

	return 0;
}
