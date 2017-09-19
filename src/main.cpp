
#include "tgaimage.h"
#include "model.h"

#include <iostream>

// Constants
const TGAColour white = TGAColour(255, 255, 255, 255);
const TGAColour red = TGAColour(255, 0, 0, 255);
const int WIDTH = 600;
const int HEIGHT = 600;

// Prototypes
void drawLine(int x0, int y0, int x1, int y1, TGAImage &image, TGAColour colour);
void drawWireframe(Model *model, TGAImage &image);
void drawModel(Model *model, TGAImage &image);
void drawTriangle(Vec3f v0, Vec3f v1, Vec3f v2, TGAImage &image, TGAColour colour);
void drawTriangleTop(Vec3f v0, Vec3f v1, Vec3f v2, TGAImage &image, TGAColour colour);
void drawTriangleBottom(Vec3f v0, Vec3f v1, Vec3f v2, TGAImage &image, TGAColour colour);
void resetZBuf();

float zbuf[WIDTH * HEIGHT];
bool isDebugging = false;

int main() 
{
	TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

	resetZBuf();

	Vec3f A = Vec3f(110, 120, 0.512);
	Vec3f B = Vec3f(120, 150, 0.581);
	Vec3f C = Vec3f(130, 130, 0.612);
	Vec3f D = Vec3f(150, 160, 0.654);

	//drawTriangle(A, B, C, image, white);
	//drawTriangle(B, C, D, image, red);

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

	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vec3f screen_points[3];
		Vec3f world_points[3];
		for (int j = 0; j < 3; j++)
		{
			Vec3f v = model->vert(face[j]);
			screen_points[j] = Vec3f((v.x + 1.) * WIDTH / 2., (v.y + 1.) * HEIGHT / 2., v.z); // Convert to screen space
			world_points[j] = v;
		}
		// Diffuse Lighting
		Vec3f norm = (world_points[2] - world_points[0]) ^ (world_points[1] - world_points[0]); // Get orthagonal vector (normal)
		norm.normalize();
		float diffuse = norm * lightDir;
		if (diffuse > 0)
			drawTriangle(screen_points[0], screen_points[1], screen_points[2], image, TGAColour(255 * diffuse, 255 * diffuse, 255 * diffuse, 255));
	}
}


// Using "II. Standard Algorithm" found here: http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
// With some modifications
void drawTriangle(Vec3f v0, Vec3f v1, Vec3f v2, TGAImage &image, TGAColour colour)
{
	Vec3f points[3] = { v0, v1, v2 };
	Vec3f temp;

	// Sort Verticies (top to bottom)
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			if (points[j].y < points[j + 1].y)
			{
				temp = points[j];
				points[j] = points[j + 1];
				points[j + 1] = temp;
			}
		}
	}

	v0 = points[0];
	v1 = points[1];
	v2 = points[2];

	// If flat on top
	if (points[0].y == points[1].y)
	{
		drawTriangleBottom(points[0], points[1], points[2], image, colour);
	}
	// If flat on bottom
	else if(points[1].y == points[2].y)
	{
		drawTriangleTop(points[0], points[1], points[2], image, colour);
	}
	// Else triangle needs to be split
	else
	{
		// Find new vertex to split triangle
		float x = v0.x + ((v1.y - v0.y) / (v2.y - v0.y)) * (v2.x - v0.x);
		float z = v0.z + ((v1.y - v0.y) / (v2.y - v0.y)) * (v2.z - v0.z);

		temp = { x, v1.y, z };
		drawTriangleTop(v0, temp, v1, image, colour);
		drawTriangleBottom(v1, temp, v2, image, colour);
	}
}

void drawTriangleTop(Vec3f v0, Vec3f v1, Vec3f v2, TGAImage &image, TGAColour colour)
{
	// Make sure are left and right
	if (v1.x > v2.x)
		std::swap(v1, v2);

	float left_slope;
	float right_slope;
	float x_start = v1.x;
	float x_end = v2.x;
	int y_top = v0.y;
	int y_bottom = v2.y;
	float z_left_slope;
	float z_right_slope;
	float z_end = v2.z;
	float z_start = v1.z;
	float current_z = z_start;

	// Find left and right slopes
	left_slope = (v0.x - v1.x) / (y_top - y_bottom); 
	right_slope = (v0.x - v2.x) / (y_top - y_bottom); 
	z_left_slope = (v0.z - v1.z) / (y_top - y_bottom);
	z_right_slope = (v0.z - v2.z) / (y_top - y_bottom);

	// Draw scanlines across each y to fill triangle
	for (int y = y_bottom; y <= y_top; y++)
	{
		for (int x = x_start; x <= x_end; x++)
		{
			if ((int)zbuf[(y * WIDTH) + x] * 100 < (int)current_z * 100) // Force 3 dp comparison
			{
				image.set(x, y, colour);
				zbuf[(y * WIDTH) + x] = current_z;
			};
			current_z += (z_end - z_start) / ((int)x_end - (int)x_start);
		}

		// Apply slopes
		x_start += left_slope;
		x_end += right_slope;

		z_start += z_left_slope;
		z_end += z_right_slope;

		current_z = z_start;
	}
}


void drawTriangleBottom(Vec3f v0, Vec3f v1, Vec3f v2, TGAImage &image, TGAColour colour)
{
	// Make sure are left and right
	if (v0.x > v1.x)
		std::swap(v0, v1);

	float left_slope;
	float right_slope;
	float x_start = v2.x;
	float x_end = v2.x;
	int y_top = v0.y;
	int y_bottom = v2.y;
	float z_left_slope;
	float z_right_slope;
	float z_start = v2.z;
	float z_end = v2.z;
	float current_z = z_start;

	// Find left and right slopes
	left_slope = (v0.x - v2.x) / (y_top - y_bottom); 
	right_slope = (v1.x - v2.x) / (y_top - y_bottom);
	z_left_slope = (v0.z - v2.z) / (y_top - y_bottom);
	z_right_slope = (v1.z - v2.z) / (y_top - y_bottom);

	// Draw scanlines across each y to fill triangle
	for (int y = y_bottom; y <= y_top; y++)
	{
		for (int x = x_start; x <= x_end; x++)
		{
			if ((int)zbuf[(y * WIDTH) + x] * 100 < (int)current_z * 100)
			{
				image.set(x, y, colour);
				zbuf[(y * WIDTH) + x] = current_z;
			}
			current_z += (z_end - z_start) / ((int)x_end - (int)x_start);
		}

		// Apply slopes
		x_start += left_slope;
		x_end += right_slope;

		z_start += z_left_slope;
		z_end += z_right_slope;

		current_z = z_start;
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
