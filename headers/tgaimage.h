#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>

#pragma pack(push,1)
struct TGA_Header 
{
	char idlength;
	char colormaptype;
	char datatypecode;
	short colormaporigin;
	short colormaplength;
	char colormapdepth;
	short x_origin;
	short y_origin;
	short width;
	short height;
	char  bitsperpixel;
	char  imagedescriptor;
};
#pragma pack(pop)



struct TGAColour 
{
	union
	{
		struct 
		{
			unsigned char b, g, r, a;
		};
		unsigned char raw[4];
		unsigned int val;
	};
	int bytespp;

	TGAColour() : val(0), bytespp(1) 
	{
	}

	TGAColour(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : b(B), g(G), r(R), a(A), bytespp(4)
	{
	}

	TGAColour(int v, int bpp) : val(v), bytespp(bpp)
	{
	}

	TGAColour(const TGAColour &c) : val(c.val), bytespp(c.bytespp) 
	{
	}

	TGAColour(const unsigned char *p, int bpp) : val(0), bytespp(bpp) 
	{
		for (int i=0; i<bpp; i++) 
		{
			raw[i] = p[i];
		}
	}

	TGAColour & operator =(const TGAColour &c)
	{
		if (this != &c)
		{
			bytespp = c.bytespp;
			val = c.val;
		}
		return *this;
	}
};


class TGAImage
{
protected:
	unsigned char* data;
	int width;
	int height;
	int bytespp;

	bool   load_rle_data(std::ifstream &in);
	bool unload_rle_data(std::ofstream &out);
public:
	enum Format
	{
		GRAYSCALE=1, RGB=3, RGBA=4
	};

	TGAImage();
	TGAImage(int w, int h, int bpp);
	TGAImage(const TGAImage &img);
	bool read_tga_file(const char *filename);
	bool write_tga_file(const char *filename, bool rle=true);
	bool flip_horizontally();
	bool flip_vertically();
	bool scale(int w, int h);
	TGAColour get(int x, int y);
	bool set(int x, int y, TGAColour c);
	~TGAImage();
	TGAImage & operator =(const TGAImage &img);
	int get_width();
	int get_height();
	int get_bytespp();
	unsigned char *buffer();
	void clear();
};

#endif //__IMAGE_H__
