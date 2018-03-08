#include "tgaimage.h"
#include "model.h"
#include <iostream> 
#include <vector>
#include <algorithm>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

Model *model = NULL;

Vec3f barycentric(Vec2i *pts, Vec2i P) {
	Vec3f u = cross(
		Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]),
		Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1])
	);
	// triangle is degenerate, in this case return smth with negative coordinates 
	if (std::abs(u[2]) < 1) {
		return Vec3f(-1, 1, 1);
	}
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy)*2;
	int error2 = 0;
	int y = y0;

	for (float x = x0; x <= x1; x++) {
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (y1 > y0 ? 1 : -1);
			error2 -= dx*2;
		}
	}
}


void triangle(Vec2i *pts, TGAImage &image, TGAColor color) {
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2i bboxmax(0, 0);
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i<3; i++) {
		for (int j = 0; j<2; j++) {
			bboxmin[j] = std::max(0, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
			image.set(P.x, P.y, color);
		}
	}
}


int main(int argc, char** argv) {
	int width = 2000;
	int height = 2000;

	TGAImage image(width, height, TGAImage::RGB);
	model = new Model("obj/african_head/african_head.obj");

	//Vec2i pts[3] = { Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160) };
	//triangle(pts, image, TGAColor(255, 0, 0));

	/*for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		for (int j = 0; j < 3; j++)
		{
			Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j+1)%3]);
			int x0 = (v0.x + 1.)*width / 2.;
			int y0 = (v0.y + 1.)*height / 2.;
			int x1 = (v1.x + 1.)*width / 2.;
			int y1 = (v1.y + 1.)*height / 2.;
			line(x0, y0, x1, y1, image, white);
		}
	}*/

	//line(13, 20, 80, 40, image, white);
	//line(20, 13, 40, 80, image, red);
	//line(80, 40, 13, 20, image, red);

	Vec3f light_dir(0, 0, -1);
	for (int i = 0; i<model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		Vec2i screen_coords[3];
		Vec3f world_coords[3];
		for (int j = 0; j<3; j++) {
			Vec3f v = model->vert(face[j]);
			screen_coords[j] = Vec2i((v.x + 1.)*width / 2., (v.y + 1.)*height / 2.);
			world_coords[j] = v;
		}
		Vec3f n = cross((world_coords[2] - world_coords[0]),(world_coords[1] - world_coords[0]));
		n.normalize();
		float intensity = n * light_dir;
		if (intensity>0) {
			triangle(screen_coords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
	}

	image.flip_vertically();
	image.write_tga_file("Output.tga");
	return 0;
}