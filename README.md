# Rasterizer
This is a simple triangle rasterizer using this tutorial as a guide: https://github.com/ssloy/tinyrenderer. The project uses the tutorials premade code to handle tga image handling, obj file parsing and geometric defintions (vectors, matrixs, etc.). These files are tgaimage.cpp/.h + model.cpp/.h + geometry.cpp/.h). The wiki details the progress made for this rasterizer.

## Current Progress
The rasterizer can currently display a the triangles of a .obj file in with some horizontal line artifacts caused by trying to draw a small triangle with a large slope. Here is the current render progress shown with a head model:   
      
![](https://github.com/LibLib97/Renderer/raw/master/render.png)

## Resources Used
- https://github.com/ssloy/tinyrenderer
- http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
