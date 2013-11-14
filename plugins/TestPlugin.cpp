#include "VisPlugin.h"
#include <cmath>
#include <cairo/cairo.h>

class TestPlugin : public VisPlugin {

public:

    virtual double getVersion() const {
        return 0.1;
    }

    virtual int ARGB(int width, int height, unsigned char *bitmap)
    {
      // set up cairo surface
      cairo_surface_t *surface;
      cairo_format_t format = CAIRO_FORMAT_ARGB32;
      int stride = cairo_format_stride_for_width(format, width);
      surface = cairo_image_surface_create_for_data (bitmap, format, width, height, stride);

      // set up cairo context
      cairo_t *cr;
      cr = cairo_create(surface);
      cairo_scale(cr, width, height);
      cairo_paint_with_alpha (cr, 0);

      // draw black rectangle
      cairo_set_line_width(cr, 0.1);
      cairo_set_source_rgba(cr, 1, 1, 0, 1);
      cairo_rectangle(cr, 0.25, 0.25, 0.5, 0.5);
      cairo_stroke(cr);

      // clean up
      cairo_destroy(cr);
      cairo_surface_destroy (surface);

      return 0;
    }
};

extern "C" VisPlugin* create() {
    return new TestPlugin;
}

extern "C" void destroy(VisPlugin* p) {
    delete p;
}
