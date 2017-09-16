
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
void testTriangles();

//float zbuf[WIDTH][HEIGHT];
bool isDebugging = false;

int main() 
{
	TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);
	

	Vec3f v0 = { 100, (float)400.11, 0 };
	Vec3f v1 = { 300, 400, 0 };
	Vec3f v2 = { 200, (float)200.45, 0 };

	Model model("obj/african_head.obj");
	drawModel(&model, image);
	
	//isDebugging = true;
	//drawTriangle(v0, v1, v2, image, red);

	image.flip_vertically();
	image.write_tga_file("render.tga");
	std::cout << "Render Saved" << std::endl;

	//testTriangles();

	return 0;
}

void drawModel(Model *model, TGAImage &image)
{

	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vec3f screen_points[3];
		for (int j = 0; j < 3; j++)
		{
			Vec3f v = model->vert(face[j]);
			screen_points[j] = Vec3f((v.x + 1.) * WIDTH / 2., (v.y + 1.) * HEIGHT / 2., v.z);
		}

		drawTriangle(screen_points[0], screen_points[1], screen_points[2], image, TGAColour(rand() % 255, rand() % 255, rand() % 255, 255));
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

	if (isDebugging)
	{
		std::cout << "0: " << points[0] << std::endl;
		std::cout << "1: " << points[1] << std::endl;
		std::cout << "2: " << points[2] << std::endl;
	}
	v0 = points[0];
	v1 = points[1];
	v2 = points[2];


	// If flat on top
	if (points[0].y == points[1].y)
	{
		if (isDebugging)
			std::cout << "Flat top" << std::endl;
		drawTriangleBottom(points[0], points[1], points[2], image, colour);
	}
	// If flat on bottom
	else if(points[1].y == points[2].y)
	{
		if (isDebugging)
			std::cout << "Flat Bottom" << std::endl;
		drawTriangleTop(points[0], points[1], points[2], image, colour);
	}
	// Else triangle needs to be split
	else
	{
		if (isDebugging)
			std::cout << "Complex" << std::endl;
		// Find new vertex to split triangle
		float x = v0.x + ((v1.y - v0.y) / (v2.y - v0.y)) * (v2.x - v0.x);
		float z = v0.z + ((v1.y - v0.y) / (v2.y - v0.y)) * (v2.z - v0.z);
		if (isDebugging)
			std::cout << "x: " << x << ", y: " << v1.y << ", z: " << z << std::endl;
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


	// Find left and right slopes
	left_slope = (v0.x - v1.x) / (y_top - y_bottom); //(v0.y - v1.y);
	right_slope = (v0.x - v2.x) / (y_top - y_bottom); //(v0.y - v2.y);

	//if the difference between top and bottom is neglible, ignore to reduce accuracy errors
	//if (abs(v0.y - v2.y <= 2))
	//{
	//	drawLine(v0.x, v0.y, v1.x, v1.y, image, colour);
	//	drawLine(v1.x, v1.y, v2.x, v2.y, image, colour);
	//	drawLine(v2.x, v2.y, v0.x, v0.y, image, colour);
	//	return;
	//}


	if (isDebugging)
	{
		std::cout << "\n\nLeft Slope: " << left_slope << std::endl;
		std::cout << "Right Slope: " << right_slope << std::endl;
		std::cout << "x_start: " << x_start << std::endl;
		std::cout << "x_end: " << x_end << std::endl;
		std::cout << "y_top: " << y_top << std::endl;
		std::cout << "y_bottom: " << y_bottom << std::endl;
	}
	

	// Draw scanlines across each y to fill triangle
	for (int y = y_bottom; y <= y_top; y++)
	{
		drawLine((int)x_start, y, (int)x_end, y, image, colour);

		if (isDebugging)
			std::cout << "Line Drawn: (" << (int)x_start << ", " << y << "), (" <<  (int)x_end << ", " << y << ")" << std::endl;
		// Apply slopes
		x_start += left_slope;
		x_end += right_slope;
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

	// Find left and right slopes
	left_slope = (v0.x - v2.x) / (y_top - y_bottom); //(v0.y - v2.y);
	right_slope = (v1.x - v2.x) / (y_top - y_bottom); //(v1.y - v2.y);


	//if the difference between top and bottom is neglible, ignore to reduce accuracy errors
	//if (abs(v0.y - v2.y <= 2))
	//{
	//	drawLine(v0.x, v0.y, v1.x, v1.y, image, colour);
	//	drawLine(v1.x, v1.y, v2.x, v2.y, image, colour);
	//	drawLine(v2.x, v2.y, v0.x, v0.y, image, colour);
	//	return;
	//}


	if (isDebugging)
	{
		std::cout << "\n\nLeft Slope: " << left_slope << std::endl;
		std::cout << "Right Slope: " << right_slope << std::endl;
		std::cout << "x_start: " << x_start << std::endl;
		std::cout << "x_end: " << x_end << std::endl;
		std::cout << "y_top: " << y_top << std::endl;
		std::cout << "y_bottom: " << y_bottom << std::endl;
	}
	

	// Draw scanlines across each y to fill triangle
	for (int y = y_bottom; y <= y_top; y++)
	{
		drawLine((int)x_start, y, (int)x_end, y, image, colour);

		if (isDebugging)
			std::cout << "Line Drawn: (" << (int)x_start << ", " << y << "), (" << (int)x_end << ", " << y << ")" << std::endl;

		// Apply slopes
		x_start += left_slope;
		x_end += right_slope;
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

void testTriangles()
{

	// Flat Topped Triangle
	 
	TGAImage flatTopImage(WIDTH, HEIGHT, TGAImage::RGB);

	Vec3f flatTopV0 = { 40, 20, 0 };
	Vec3f flatTopV1 = { 30, 50, 0 };
	Vec3f flatTopV2 = { 50, 50, 0 };

	drawTriangle(flatTopV0, flatTopV1, flatTopV2, flatTopImage, white);

	flatTopImage.flip_vertically();
	flatTopImage.write_tga_file("tests/FlatTop.tga");

	// Flat Bottomed Triangle

	TGAImage flatBottomImage(WIDTH, HEIGHT, TGAImage::RGB);

	Vec3f flatBottomV0 = { 40, 50, 0 };
	Vec3f flatBottomV1 = { 30, 20, 0 };
	Vec3f flatBottomV2 = { 50, 20, 0 };

	drawTriangle(flatBottomV0, flatBottomV1, flatBottomV2, flatBottomImage, white);

	flatBottomImage.flip_vertically();
	flatBottomImage.write_tga_file("tests/FlatBottom.tga");

	// Complex Triangle

	TGAImage complexImage(WIDTH, HEIGHT, TGAImage::RGB);

	Vec3f complexV0 = { 40, 20, 0 };
	Vec3f complexV1 = { 30, 10, 0 };
	Vec3f complexV2 = { 20, 50, 0 };

	drawTriangle(complexV0, complexV1, complexV2, complexImage, white);

	complexImage.flip_vertically();
	complexImage.write_tga_file("tests/Complex.tga");

	std::cout << "Tests Finished" << std::endl;
}