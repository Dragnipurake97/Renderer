
#include "tgaimage.h"
#include "model.h"

#include <iostream>
#include <string>

// Constants
const TGAColour white = TGAColour(255, 255, 255, 255);
const TGAColour red = TGAColour(255, 0, 0, 255);
const int WIDTH = 800;
const int HEIGHT = 800;

// Prototypes
void drawLine(int x0, int y0, int x1, int y1, TGAImage &image, TGAColour colour);
void drawWireframe(Model *model, TGAImage &image);
void drawModel(Model *model, TGAImage &image);
void drawTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec2i vt0, Vec2i vt1, Vec2i vt2, TGAImage &image, TGAImage texture, float diffuse);
void drawTriangleTop(Vec3f v0, Vec3f v1, Vec3f v2, Vec2i vt0, Vec2i vt1, Vec2i vt2, TGAImage &image, TGAImage texture, float diffuse);
void drawTriangleBottom(Vec3f v0, Vec3f v1, Vec3f v2, Vec2i vt0, Vec2i vt1, Vec2i vt2, TGAImage &image, TGAImage texture, float diffuse);
void resetZBuf();
bool loadTexture(std::string filename, TGAImage& texture);

// Globals
float zbuf[WIDTH * HEIGHT];

int main() 
{
	resetZBuf();

	TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

	Model model("obj/african_head.obj");
	drawModel(&model, image);
	
	image.flip_vertically();
	image.write_tga_file("render.tga");
	std::cout << "Render Saved" << std::endl;

	return 0;
}

void drawModel(Model *model, TGAImage &image)
{
	Vec3f lightDir(0, 0, -1);


	TGAImage texture;
	loadTexture("textures/head_texture.tga", texture);

	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vec3f screen_points[3];
		Vec3f world_points[3];
		Vec2i texture_coords[3];

		for (int j = 0; j < 3; j++)
		{
			Vec3f v = model->vert(face[j]);
			screen_points[j] = Vec3f((v.x + 1.) * WIDTH / 2., (v.y + 1.) * HEIGHT / 2., v.z); // Convert to screen space
			world_points[j] = v; // Get raw obj values (-1 to 1)
		}

		// Diffuse Lighting
		Vec3f norm = (world_points[2] - world_points[0]) ^ (world_points[1] - world_points[0]); // Get orthagonal vector (normal)
		norm.normalize();

		float diffuse = norm * lightDir;

		if (diffuse > 0)
		{
			for (int j = 0; j < 3; j++)
			{
				texture_coords[j] = model->getTexCoord(i, j, texture);
			}

			drawTriangle(screen_points[0], screen_points[1], screen_points[2], texture_coords[0], texture_coords[1], texture_coords[2], image, texture, diffuse);
		}
	}
}

void drawTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Vec2i vt0, Vec2i vt1, Vec2i vt2, TGAImage &image, TGAImage texture, float diffuse)
{
	Vec3f points[3] = { v0, v1, v2 };
	Vec2i texture_coords[3] = { vt0, vt1, vt2 };
	Vec3f temp;
	Vec2i tempt;

	// Sort Verticies (top to bottom) via bubblesort
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			if (points[j].y < points[j + 1].y)
			{
				temp = points[j];
				points[j] = points[j + 1];
				points[j + 1] = temp;

				tempt = texture_coords[j];
				texture_coords[j] = texture_coords[j + 1];
				texture_coords[j + 1] = tempt;
			}
		}
	}

	v0 = points[0]; // Top
	v1 = points[1];
	v2 = points[2]; // Bottom

	if ((int)(v0.y + 0.5f) == (int)(v2.y + 0.5f))
		return;

	vt0 = texture_coords[0];
	vt1 = texture_coords[1];
	vt2 = texture_coords[2];

		if ((int)points[0].y == (int)points[1].y) // If flat on top
		{
			drawTriangleBottom(points[0], points[1], points[2], texture_coords[0], texture_coords[1], texture_coords[2], image, texture, diffuse);
		}
		else if((int)points[1].y == (int)points[2].y) // If flat on bottom
		{
			drawTriangleTop(points[0], points[1], points[2], texture_coords[0], texture_coords[1], texture_coords[2], image, texture, diffuse);
		}
		else 	// Else triangle needs to be split
		{

			// Find new vertex to split triangle
			float x = v2.x + ((v1.y - v2.y) / (v0.y - v2.y)) * (v0.x - v2.x);
			float z = v2.z + ((v1.y - v2.y) / (v0.y - v2.y)) * (v0.z - v2.z);
			temp = { x, v1.y, z }; // New vertex

			tempt = vt2 + ((vt0 - vt2) * ((temp.y - v2.y) / (v0.y - v2.y)));

			drawTriangleTop(v0, temp, v1, vt0, tempt, vt1, image, texture, diffuse);
			drawTriangleBottom(v1, temp, v2, vt1, tempt, vt2, image, texture, diffuse);
	}
}


void drawTriangleTop(Vec3f v0, Vec3f v1, Vec3f v2, Vec2i vt0, Vec2i vt1, Vec2i vt2, TGAImage &image, TGAImage texture, float diffuse)
{
	// Make sure are left and right
	if (v1.x > v2.x)
	{
		std::swap(v1, v2);
		std::swap(vt1, vt2);
	}

	//     v0
	//    |  |
	//  |      |
	// v1-------v2

	int height = (int)v0.y - (int)v1.y; // Cast to int so they subtract as ints

	if (height == 0)
		return;

	Vec3f start = v1;
	Vec3f end = v2;

	Vec2i tex_start = vt1;
	Vec2i tex_end = vt2;
	Vec2i tex_coord = vt1;

	float z = start.z;
	float y_factor; // How far into y of triangle (0 to 1)

	for (int y = v1.y; y <= v0.y; y++)
	{
		y_factor = (y - (int)v1.y) / (float)height;

		start = v1 + ((v0 - v1) * y_factor); // Cast to int or float because of how it loops through y
		end = v2 + ((v0 - v2) * y_factor);

		z = start.z;

		tex_start = vt1 + ((vt0 - vt1) * y_factor);
		tex_end = vt2 + ((vt0 - vt2) * y_factor);

		tex_coord = tex_start;

		for (int x = start.x; x <= end.x; x++)
		{
			if ((int)zbuf[(y * WIDTH) + x] * 100 < (int)z * 100)
			{
				TGAColour c = texture.get(tex_coord.x, tex_coord.y);
				image.set(x, y, TGAColour(c.r * diffuse, c.g * diffuse, c.b * diffuse, c.a));
				zbuf[(y * WIDTH) + x] = z;
			}

			z = start.z + ((end.z - start.z) * ((x - start.x) / (end.x - start.x))); // Find z along line
			tex_coord = tex_start + ((tex_end - tex_start) * ((x - start.x) / (end.x - start.x)));
		}
	}
}


void drawTriangleBottom(Vec3f v0, Vec3f v1, Vec3f v2, Vec2i vt0, Vec2i vt1, Vec2i vt2, TGAImage &image, TGAImage texture, float diffuse)
{
	// Make sure are left and right
	if (v0.x > v1.x)
	{
		std::swap(v0, v1);
		std::swap(vt0, vt1);
	}

	// V0 ----- v1
	//   |     |
	//     | |
	//      v2

	int height = (int)v0.y - (int)v2.y;

	if (height == 0)
		return;

	Vec3f start = v2;
	Vec3f end = v2;

	Vec2i tex_start = vt2;
	Vec2i tex_end = vt2;
	Vec2i tex_coord = vt2;

	float z;
	float y_factor;

	for (int y = v2.y; y <= v0.y; y++)
	{
		y_factor = (y - (int)v2.y) / (float)height;

		start = v2 + ((v0 - v2) * y_factor); // New start position
		end = v2 + ((v1 - v2) * y_factor); // New end position
		z = start.z;

		tex_start = vt2 + ((vt0 - vt2) * y_factor); // New texture start position
		tex_end = vt2 + ((vt1 - vt2) * y_factor); // New texture end position

		tex_coord = tex_start;

		for (int x = start.x; x <= end.x; x++)
		{
			if ((int)zbuf[(y * WIDTH) + x] * 100 < (int)z * 100)
			{
				TGAColour c = texture.get(tex_coord.x, tex_coord.y);
				image.set(x, y, TGAColour(c.r * diffuse, c.g * diffuse, c.b * diffuse, c.a));
				zbuf[(y * WIDTH) + x] = z;
			}

			z = start.z + ((end.z - start.z) * ((x - start.x) / (end.x - start.x))); // Find z along line
			tex_coord = tex_start + ((tex_end - tex_start) * ((x - start.x) / (end.x - start.x)));
		}
	}
}

void drawWireframe(Model *model, TGAImage &image)
{
	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		for (int j = 0; j < 3; j++)
		{
			Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j + 1) % 3]);
			int x0 = (v0.x + 1.) * WIDTH / 2.;
			int y0 = (v0.y + 1.) * HEIGHT / 2.;
			int x1 = (v1.x + 1.) * WIDTH / 2.;
			int y1 = (v1.y + 1.) * HEIGHT / 2.;

			drawLine(x0, y0, x1, y1, image, white);
		}
	}
}

void drawLine(int x0, int y0, int x1, int y1, TGAImage &image, TGAColour colour) 
{
	bool isSteep = false;

	// Check if line has is more that 45 degrees
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) 
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		isSteep = true;
	}

	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y0;

	for (int x = x0; x <= x1; x++) 
	{
		if (isSteep) 
		{
			image.set(y, x, colour);
		}
		else 
		{
			image.set(x, y, colour);
		}

		error2 += derror2;

		if (error2 > dx)
		{
			y += (y1 > y0 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

void resetZBuf()
{
	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			zbuf[(y * HEIGHT) + x] = -1.0f;
		}
	}
}

bool loadTexture(std::string filename, TGAImage& texture)
{
	if (texture.read_tga_file(filename.c_str()))
	{
		std::cout << filename << " loaded successfully" << std::endl;
		texture.flip_vertically();
		return true;
	}
	else
	{
		std::cout << filename << " unable to load" << std::endl;
		return false;
	}
}
