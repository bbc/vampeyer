#include "GUI.h"

GUI::GUI(int width, int height, unsigned char *buffer)
{
  winWidth=width;
  winHeight=height;

  // convert ARGB to ABGR
  // TODO Work out why this is necessary
  unsigned int* buf = (unsigned int*)&buffer[0];
  unsigned int* end = buf + width*height;
  while(buf < end) {
    unsigned int pixel = *buf;
    *buf = ((pixel&0x00ff0000)>>16) + ((pixel&0x000000ff)<<16) +
      ((pixel&0xff00ff00));
    buf++;
  }

  Fl::visual(FL_RGB);
  win = new Fl_Window(winWidth, winHeight, "AudioVis");
  win->position((Fl::w() - win->w())/2, (Fl::h() - win->h())/2);
  Fl_Box imageBox(0,0,winWidth,winHeight);
  Fl_RGB_Image image(buffer, width, height, 4); 
  imageBox.image(image);
  win->end();
  win->show();
  Fl::run();
}

GUI::~GUI()
{
  delete win;
}
