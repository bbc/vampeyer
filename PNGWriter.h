#ifndef PNGWRITER_H
#define PNGWRITER_H

#include <iostream>
#include <png.h>

using std::cerr;
using std::endl;

#define BYTES_PER_PIXEL 4

class PNGWriter
{
  public:
    PNGWriter(int width, int height, unsigned char *buffer);
    int write(const char *filename);
  protected:
    int width;
    int height;
    unsigned char *image;
};

#endif
