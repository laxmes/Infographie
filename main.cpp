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

void triangle(TGAImage &image, Vec3i p0, Vec3i p1, Vec3i p2, TGAColor color, int * zbuffer) {
	if(p0.x == p1.x && p0.x == p2.x) return;
	if(p0.x > p1.x) std::swap(p0, p1);
	if(p0.x > p2.x) std::swap(p0, p2);
	if(p1.x > p2.x) std::swap(p1, p2);

	float distance_p0p2 = p2.x - p0.x;
	for(int i = 0; i < distance_p0p2; ++i) {
		bool invert = (i > p1.x - p0.x) || (p0.x == p1.x);
		float coef0 = (float) i / distance_p0p2;
		float coef1 = (float) (i - ((invert) ? p1.x - p0.x : 0)) / ((invert) ? p2.x - p1.x : p1.x - p0.x);

		Vec3i A = p0 + ((p2 - p0) * coef0);
		Vec3i B = ((invert) ?
				(p1 + ((p2 - p1) * coef1)) :
				(p0 + ((p1 - p0) * coef1)));

		if(A.y > B.y) std::swap(A, B);
		for(int j = A.y; j <= B.y; j++) {
			int idx = j + (p0.x + i) * height;
			int z = A.z + ((B.y - A.y == 0) ? B.z : (((B.z - A.z) / (B.y - A.y)) * j));
			if(z > zbuffer[idx]) {
				zbuffer[idx] = z;
				image.set(i + p0.x, j, color);
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
		Vec3i screen_coords[3];
		Vec3f world_coords[3];
		for(int j = 0; j < 3; j++) {
			Vec3f v = model->vert(face[j]);
			screen_coords[j] = Vec3i((v.x + 1) * width / 2, (v.y + 1) * height / 2, (v.z + 1) * depth / 2);
			world_coords[j] = v;
		}

		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		n.normalize();
		float intensity = n * light;
		if(intensity > 0)
			triangle(image, screen_coords[0], screen_coords[1], screen_coords[2], TGAColor(255 * intensity, 255 * intensity, 255 * intensity, 255), zbuffer);
	}

	delete model;
	delete [] zbuffer;

	image.flip_vertically();
	image.write_tga_file("dump.tga");

	return 0;
}
