#include "GUI.h"

GUI::GUI(int width, int height, unsigned char *buffer)
{
  winWidth=width;
  winHeight=height;

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
