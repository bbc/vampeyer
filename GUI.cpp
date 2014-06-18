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

  // initialise FLTK window
  Fl::visual(FL_RGB);
  win = new Fl_Window(winWidth, winHeight, "AudioVis");

  // move to centre of screen
  win->position((Fl::w() - win->w())/2, (Fl::h() - win->h())/2);

  // draw image to window
  Fl_Box imageBox(0,0,winWidth,winHeight);
  Fl_RGB_Image image(buffer, width, height, 4); 
  imageBox.image(image);

  // display window
  win->end();
  win->show();
  Fl::run();
}

GUI::~GUI()
{
  delete win;
}
