#include "VisPlugin.h"
#include <cmath>

class TestPlugin : public VisPlugin {

public:

    virtual double getVersion() const {
        return 0.1;
    }

    virtual int getBitmap(int width, int height, char *&bitmap)
    {
      // return width*height zero array
      int pixels=width*height;
      for (int i=0; i<pixels; i++)
      {
        bitmap[i] = 0;
      }
      return 0;
    }
};

extern "C" VisPlugin* create() {
    return new TestPlugin;
}

extern "C" void destroy(VisPlugin* p) {
    delete p;
}
