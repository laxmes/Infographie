#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
#include <cmath>
#include <stdlib.h>
#include <limits>
#include <chrono>
#include <string>

Model * model = NULL;
int * zbuffer;
int * shadowbuffer;

const int WIDTH     = 512;
const int HEIGHT    = 512;

Vec3f light;
float light_ity;

Matrix invM;
Matrix MShadow;

Vec3f camera = Vec3f(0., 2., 5.), local_camera;
Vec3f center = Vec3f(0., 0., 0.);
Vec3f up = Vec3f(0., 1., 0.);

void triangle(Vec3i p1, Vec3i p2, Vec3i p3, Vec2i texture[], TGAImage & image, int * localzbuffer, bool isShadowBuffer) {
	if (p1.y == p2.y && p2.y == p3.y) return;
	if (p1.y > p2.y) { std::swap(p1, p2); std::swap(texture[0], texture[1]); }
	if (p1.y > p3.y) { std::swap(p1, p3); std::swap(texture[0], texture[2]); }
	if (p2.y > p3.y) { std::swap(p2, p3); std::swap(texture[1], texture[2]); }

	Vec3f reflect;
	float spec, nP;
	int distance_p1_p3 = p3.y - p1.y;
	Vec3i p3_minus_p1 = p3 - p1, p3_minus_p2 = p3 - p2, p2_minus_p1 = p2 - p1;
	Vec2i t2_minus_t0 = texture[2] - texture[0], t2_minus_t1 = texture[2] - texture[1], t1_minus_t0 = texture[1] - texture[0];
	for (int i = 0; i <= distance_p1_p3; i++) {
		bool inverted = (i + p1.y > p2.y) || p1.y == p2.y;

		float coef1 = (float)i / distance_p1_p3;
		float coef2 = (float)(i - ((inverted) ? p2.y - p1.y : 0)) / ((inverted) ? p3.y - p2.y : p2.y - p1.y);

		Vec3i A = p1 + (p3_minus_p1 * coef1);
		Vec3i B = (inverted) ? p2 + ((p3_minus_p2)* coef2)
			: p1 + (p2_minus_p1 * coef2);

		Vec2i tA = texture[0] + (t2_minus_t0 * coef1);
		Vec2i tB = (inverted) ? texture[1] + (t2_minus_t1 * coef2)
			: texture[0] + (t1_minus_t0 * coef2);

		if (A.x > B.x) { std::swap(A, B); std::swap(tA, tB); }
		Vec3i B_minus_A = B - A;
		Vec2i tB_minus_tA = tB - tA;
		for (int j = A.x; j <= B.x; j++) {
			float coef3 = (B_minus_A.x == 0 ? 1. : (float)(j - A.x) / (float) B_minus_A.x);
			Vec3f pZ(j, i + p1.y, A.z + (B_minus_A.z * coef3));
			Vec2i tZ = tA + (tB_minus_tA * coef3);
			int idZ = ((i + p1.y) * WIDTH) + j;
			if (pZ.z > localzbuffer[idZ]) {
				localzbuffer[idZ] = pZ.z;
				if (isShadowBuffer) {
					image.set(pZ.x, pZ.y, TGAColor(pZ.z, pZ.z, pZ.z));
				} else {
					Vec3f n = model->normal(tZ);

					float tmp_light = 0.f, tmp_specular = model->specular(tZ);

					reflect = (n * (n * light * 2.f) - light).normalize();
					spec = pow((reflect.z < 0) ? 0.f : reflect.z, tmp_specular);
					nP = n * light;
					spec = (spec > 1) ? 1. : spec;
					nP = ((nP < 0.) ? 0. : (nP > 1.) ? 1. : nP) * light_ity;
					tmp_light = nP + (spec * light_ity);

					TGAColor diffuse = model->diffuse(tZ);

					Vec3f shadowP = proj<3>(invM * embed<4>(pZ));

					float shadow = .3 + .7 * (shadowbuffer[int(shadowP.x) + int(shadowP.y) * WIDTH]);

					for (int c = 0; c < 3; c++) {
						int tmp =  shadow;
						diffuse[c] = (tmp > 255) ? 255 : tmp;
					}

					image.set(pZ.x, pZ.y, diffuse);
				}
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

	auto start_time = std::chrono::high_resolution_clock::now();

	light = Vec3f(1., 1., 1.).normalize();
	light_ity = 0.8f;

	zbuffer = new int[WIDTH * HEIGHT];
	shadowbuffer = new int[WIDTH * HEIGHT];

	Matrix viewPortMatrix = viewport(WIDTH / 8, HEIGHT / 8, WIDTH * 3 / 4, HEIGHT * 3 / 4, 255);
	for (int r = 0; r < 360; r += 4) {
		char numstr[21];
		sprintf_s(numstr, "%d", r);

		TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);
		for (int i = 0; i < WIDTH * HEIGHT; i++) {
			zbuffer[i] = std::numeric_limits<int>::min();
			shadowbuffer[i] = std::numeric_limits<int>::min();
		}

		image.flip_vertically();

		float rad = 3.14159265358979323846 * ((float)r / 180);

		Matrix rotationMatrix = Matrix::identity();
		rotationMatrix[0][0] = cos(rad);rotationMatrix[2][2] = cos(rad);
		rotationMatrix[0][2] = -sin(rad);rotationMatrix[2][0] = sin(rad);

		local_camera = proj<3>(rotationMatrix * embed<4>(camera));

		Matrix projectionMatrix = projection(0);
		Matrix M;
		Matrix lookatMatrix;

		TGAImage imageshadow(WIDTH, HEIGHT, TGAImage::RGB);

		lookatMatrix = lookat(light, up, center);
		M = viewPortMatrix * projectionMatrix * lookatMatrix;

		MShadow = M;

		for (int i = 0; i < model->nfaces(); i++) {
			std::vector<Vec3i> face = model->face(i);
			Vec2i texture[3];
			Vec3i screen_coords[3];
			for (int j = 0; j < 3; j++) {
				Vec3f v = model->vert(face[j][0]);
				screen_coords[j] = proj<3>(M * embed<4>(v));
			}

			for (int j = 0; j < 3; j++) texture[j] = model->uv(i, j);

			triangle(screen_coords[0], screen_coords[1], screen_coords[2], texture, imageshadow, shadowbuffer, true);
		}

		imageshadow.flip_vertically(); // to place the origin in the bottom left corner of the image
		imageshadow.write_tga_file(((std::string("output/depth/depth") + numstr) + ".tga").c_str());

		lookatMatrix = lookat(local_camera, up, center);
		projectionMatrix = projection(-1.f / (local_camera - center).norm());
		M = viewPortMatrix * projectionMatrix * lookatMatrix;

		invM = M * M.invert();

		for (int i = 0; i < model->nfaces(); i++) {
			std::vector<Vec3i> face = model->face(i);
			Vec2i texture[3];
			Vec3i screen_coords[3];
			for (int j = 0; j < 3; j++) {
				Vec3f v = model->vert(face[j][0]);
				screen_coords[j] = proj<3>(M * embed<4>(v));
			}

			for (int j = 0; j < 3; j++) texture[j] = model->uv(i, j);

			triangle(screen_coords[0], screen_coords[1], screen_coords[2], texture, image, zbuffer, false);
		}

		auto end_time = std::chrono::high_resolution_clock::now();

		auto time = end_time - start_time;

		std::cout << "render took " << std::chrono::duration_cast<std::chrono::milliseconds>(time).count() << "ms to run.\n";

		image.flip_vertically();
		image.write_tga_file(((std::string("output/dump") + numstr) + ".tga").c_str());
	}

	delete model;
	delete[] zbuffer;
	delete[] shadowbuffer;

	std::getchar();
	return 0;
}