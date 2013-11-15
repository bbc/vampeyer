#ifndef GUI_H
#define GUI_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_RGB_Image.H>

class GUI
{
  protected:
    Fl_Window *win;
    int winWidth;
    int winHeight;
  public:
    GUI(int width, int height, unsigned char *buffer);
    ~GUI();
};

#endif
