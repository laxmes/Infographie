#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
#include <cmath>
#include <stdlib.h>
#include <limits>

Model * model = NULL;
int * zbuffer = NULL;

const int WIDTH  = 1024;
const int HEIGHT = 1024;
const char NB_LIGHT = 2;

Vec3f light[NB_LIGHT];
float light_ity[NB_LIGHT];

Vec3f camera = Vec3f(2., 2., 5.);
Vec3f center = Vec3f(0., 0., 0.);
Vec3f up     = Vec3f(0., 1., 0.);

void triangle(Vec3i p1, Vec3i p2, Vec3i p3, Vec2i texture[], TGAImage & image) {
	if (p1.y == p2.y && p2.y == p3.y) return;
	if (p1.y > p2.y) { std::swap(p1, p2); std::swap(texture[0], texture[1]); }
	if (p1.y > p3.y) { std::swap(p1, p3); std::swap(texture[0], texture[2]); }
	if (p2.y > p3.y) { std::swap(p2, p3); std::swap(texture[1], texture[2]); }

	Vec3f reflect[NB_LIGHT];
	float spec[NB_LIGHT];
	float nP[NB_LIGHT];

	int distance_p1_p3 = p3.y - p1.y;
	for (int i = 0; i <= distance_p1_p3; i++) {
		bool inverted = (i + p1.y > p2.y) || p1.y == p2.y;

		float coef1 = (float) i / distance_p1_p3;
		float coef2 = (float) (i - ((inverted) ? p2.y - p1.y : 0)) / ((inverted) ? p3.y - p2.y : p2.y - p1.y);

		Vec3i A = p1 + ((p3 - p1) * coef1);
		Vec3i B = (inverted) ? p2 + ((p3 - p2) * coef2)
			: p1 + ((p2 - p1) * coef2);

		Vec2i tA = texture[0] + ((texture[2] - texture[0]) * coef1);
		Vec2i tB = (inverted) ? texture[1] + ((texture[2] - texture[1]) * coef2)
			: texture[0] + ((texture[1] - texture[0]) * coef2);

		if (A.x > B.x) { std::swap(A, B); std::swap(tA, tB); }
		for (int j = A.x; j <= B.x; j++) {
			float coef3 = ((A.x == B.x) ? 1. : (float)(j - A.x) / (float)(B.x - A.x));
			Vec3i pZ(j, i + p1.y, A.z + ((B.z - A.z) * coef3));
			Vec2i tZ = tA + (tB - tA) * coef3;
			int idZ = ((i + p1.y) * WIDTH) + j;
			if (pZ.z > zbuffer[idZ]) {
				zbuffer[idZ] = pZ.z;
				Vec3f n = model->normal(tZ);

				float tmp_all_lights = 0.f;

				for (int l = 0; l < NB_LIGHT; l++) {
					reflect[l] = (n * (n * light[l] * 2.f) - light[l]).normalize();
					spec[l]    = pow((reflect[l].z < 0) ? 0.f : reflect[l].z, model->specular(tZ));
					nP[l]      = n * light[l];

					spec[l] = (spec[l] > 1) ? 1. : spec[l];
					nP[l]   = ((nP[l] < 0.) ? 0. : (nP[l] > 1.) ? 1. : nP[l]) * light_ity[l];
					
					tmp_all_lights += nP[l] + (spec[l] * light_ity[l]);
				}

				TGAColor diffuse = model->diffuse(tZ);
			

				for (int c = 0; c < 3; c++) {
					int tmp = diffuse[c] * tmp_all_lights;
					diffuse[c] = (tmp > 255) ? 255 : tmp;
				}
				
				image.set(pZ.x, pZ.y, diffuse);
			}
		}
	}
}

Matrix lookat(Vec3f camera, Vec3f up, Vec3f center) {
	Matrix matrix = Matrix::identity();
	Vec3f z = (camera - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x);

	for (int i = 0; i < 3; i++) {
		matrix[0][i] = x[i];
		matrix[1][i] = y[i];
		matrix[2][i] = z[i];
		matrix[i][3] = -center[i];
	}

	return matrix;
}

Matrix projection(float coef) {
	Matrix matrix = Matrix::identity();
	matrix[3][2] = coef;
	return matrix;
}

Matrix viewport(int x, int y, int w, int h, int p) {
	Matrix matrix = Matrix::identity();

	matrix[0][0] = w / 2.f;
	matrix[1][1] = h / 2.f;
	matrix[2][2] = p / 2.f;

	matrix[0][3] = x + w / 2.f;
	matrix[1][3] = y + h / 2.f;
	matrix[2][3] = p / 2.f;

	return matrix;
}

int main(int argc, char** argv) {
	model = new Model("./obj/african_head.obj");

	light[0] = Vec3f(1., 1., 1.).normalize();
	light[1] = Vec3f(-1., 1., 1.).normalize();

	light_ity[0] = 0.4f;
	light_ity[1] = 0.8f;

	zbuffer = new int[WIDTH * HEIGHT];
	for (int i = 0; i < WIDTH * HEIGHT; i++)
		zbuffer[i] = std::numeric_limits<int>::min();

	TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

	Matrix viewPortMatrix   = viewport(WIDTH / 8, HEIGHT / 8, WIDTH * 3 / 4, HEIGHT * 3 / 4, 255);
	Matrix projectionMatrix = projection(-1.f / (camera - center).norm());
	Matrix lookAtMatrix     = lookat(camera, up, center);

	Matrix M = viewPortMatrix * projectionMatrix * lookAtMatrix;

	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<Vec3i> face = model->face(i);
		Vec2i texture[3];
		Vec3i screen_coords[3];
		for (int j = 0; j < 3; j++) {
			Vec3f v = model->vert(face[j][0]);
			Vec4f tmp = M * embed<4>(v);
			screen_coords[j] = proj<3>(tmp / tmp[3]);
		}

		for (int j = 0; j < 3; j++) texture[j] = model->uv(i, j);

		triangle(screen_coords[0], screen_coords[1], screen_coords[2], texture, image);
	}

	image.flip_vertically();
	image.write_tga_file("dump.tga");

	delete model;
	delete[] zbuffer;
	return 0;
}