#include "VisPlugin.h"
#include <cairo/cairo.h>

class Template: public VisPlugin {

public:

    // This function returns the version number of the plugin
    virtual double getVersion() const {
        return 0.1;
    }

    // This function returns a list of Vamp plugin outputs that should be
    // extracted and supplied to the ARGB function, and defines how the Vamp
    // plugins should be configured
    virtual VampOutputList getVampPlugins()
    {
      VampOutputList pluginList;

      // declare and initialize vamp plugin with default block/step size
      VampPlugin pluginA = {"vamp-plugin-collection1:vamp-plugin-A", 0, 0};

      // declare and initialize two vamp plugin outputs
      VampOutput outputA1 = {pluginA, "vamp-output-1"};
      VampOutput outputA2 = {pluginA, "vamp-output-2"};

      // declare and initialize parameters for vamp plugin
      VampParameterList pluginParams;
      VampParameter thresh = {"threshold", 0.2};
      pluginParams.push_back(thresh);

      // declare and initialize vamp plugin with
      // defined block/step size and parameters
      VampPlugin pluginB = {"vamp-plugin-collection2:vamp-plugin-B",
                            1024, 512,
                            pluginParams};

      // declare and initialize vamp plugin output 
      VampOutput outputB1 = {pluginB, "vamp-output"};

      // add outputs to list and return
      pluginList.push_back(outputA1);
      pluginList.push_back(outputA2);
      pluginList.push_back(outputB1);
      return pluginList;
    }

    // This function takes the output of the Vamp plugins and returns a bitmap
    // of a given width and height in 32-bit ARGB format
    virtual int ARGB(Plugin::FeatureSet features,
                     int width,
                     int height,
                     unsigned char *bitmap,
                     int sampleRate)
    {
      int frames, coeffs;

      // set up cairo surface
      cairo_surface_t *surface;
      cairo_format_t format = CAIRO_FORMAT_ARGB32;
      int stride = cairo_format_stride_for_width(format, width);
      surface = cairo_image_surface_create_for_data (bitmap, format, width,
          height, stride);

      // set up cairo context
      cairo_t *cr;
      cr = cairo_create(surface);
      cairo_scale(cr, width, height);

      // paint background white
      cairo_set_source_rgba(cr, 1, 1, 1, 1);
      cairo_paint(cr);

      // start line in bottom left corner
      cairo_set_line_width(cr, 0);
      cairo_set_source_rgba(cr, 0, 0, 0, 1);
      cairo_move_to(cr, 0, 1);

      // iterate over each frame for first output (a scalar)
      frames = features[0].size();
      for (int frame=0; frame<frames; frame++)
      {
        // extract the value and draw a line
        double value = features[0].at(frame).values[0];
        cairo_line_to(cr, (double)frame/(double)frames, value);
      }
      
      // finish line in bottom right corner
      cairo_line_to(cr, 1, 1);
      cairo_close_path(cr);
      cairo_fill(cr);

      // iterate over each frame for second output (a vector)
      frames = features[1].size();
      coeffs = features[1].at(0).values.size();
      for (int frame=0; frame<frames; frame++)
      {
        for (int coeff=0; coeff<coeffs; coeff+)
        {
          // extract the value and draw a rectangle
          double value = features[1].at(frame).values.at(coeff);
          cairo_set_source_rgba(cr, value, value, value, 1);
          cairo_rectangle(cr, (double)frame/(double)frames,
                              (double)coeff/(double)coeffs,
                              1.0/(double)frames,
                              1.0/(double)coeffs);
          cairo_fill(cr);
        }
      }

      // clean up
      cairo_destroy(cr);
      cairo_surface_destroy (surface);

      return 0;
    }
};

extern "C" VisPlugin* create() {
    return new Template;
}

extern "C" void destroy(VisPlugin* p) {
    delete p;
}
