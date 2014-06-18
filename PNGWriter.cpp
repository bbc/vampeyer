/*
   Copyright 2014 British Broadcasting Corporation

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "PNGWriter.h"

PNGWriter::PNGWriter(int width_in, int height_in, unsigned char *buffer)
{
  width = width_in;
  height = height_in;
  image = buffer;
}

int PNGWriter::write(const char* filename)
{
  // convert 2d array into array of pointers
  unsigned char* row_pointers[height];
  for (int i=0; i<height; i++)
    row_pointers[i] = &image[i*width*BYTES_PER_PIXEL];

  // open file for writing
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    cerr << "Could not open file to write PNG." << endl;
    return 1;
  }

  // set up PNG struct
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL, NULL, NULL);
  if (!png_ptr) {
    cerr << "Could not allocate PNG write struct." << endl;
    fclose(fp);
    return 1;
  }

  // set up PNG info struct
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    cerr << "Could not allocate PNG info struct." << endl;
    fclose(fp);
    return 1;
  }

  // initialise PNG writing
  png_init_io(png_ptr, fp);

  // write header
  png_set_IHDR(png_ptr, info_ptr, width, height,
      8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  // write image
  png_set_rows(png_ptr, info_ptr, row_pointers);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, NULL);

  // clean up
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(fp);

  return 0;
}
