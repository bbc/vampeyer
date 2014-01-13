#include "VisPlugin.h"
#include <cairo/cairo.h>
#include <iostream>
#include <math.h>

using std::cout;
using std::endl;

class SpecCont: public VisPlugin {

public:

    virtual double getVersion() const {
        return 0.1;
    }

    virtual int ARGB(Plugin::FeatureSet features, int width,
        int height, unsigned char *bitmap)
    {
      const double mindB=-45.0;
      int frames;
      unsigned int coeffs = features[0].at(0).values.size();

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
      cairo_set_source_rgba(cr, 1, 1, 1, 1);
      cairo_paint(cr);

      // start line in bottom left corner
      cairo_set_line_width(cr, 0);
      cairo_set_source_rgba(cr, 0, 0, 0, 1);
      cairo_move_to(cr, 0, 1);

      // draw MFCCs
      //frames = features[0].size();
      //for (int frame=0; frame<frames; frame++)
      //{
        //for (unsigned int coeff=0; coeff<coeffs; coeff++)
        //{
          //float value = features[0].at(frame).values.at(coeff); 
          //if (value < -5.0) value = -5.0;
          //if (value > 1.0) value = 1.0;
          //value = (value + 5.0) / 6.0;
          //cairo_set_source_rgba(cr, value,
                                    //value,
                                    //value, 1);
          //cairo_rectangle(cr, (double)frame/(double)frames,
                              //(double)coeff/(double)coeffs,
                              //1.0/(double)frames,
                              //1.0/(double)coeffs);
          //cairo_fill(cr);
        //}
      //}



      // start line in bottom left corner
      cairo_set_line_width(cr, 0);
      cairo_set_source_rgba(cr, 0, 0, 0, 1);
      cairo_move_to(cr, 0, 1);

      frames = features[0].size();
      for (int frame=0; frame<frames; frame++)
      {
        Plugin::Feature feats = features[0].at(frame);
        double max = feats.values[0];
        double maxdB = 20.0*log10(max);
        if (maxdB < mindB) maxdB=mindB;
        cout << max << endl;
        cairo_rectangle(cr, (double)frame/(double)frames,
                            -maxdB/(-mindB*2),
                            1.0/(double)frames,
                            (-mindB+maxdB)/-mindB);
        cairo_fill(cr);
      }

      // finish line
      //cairo_line_to(cr, 1, 1);
      //cairo_close_path(cr);
      //cairo_fill(cr);

      // clean up
      cairo_destroy(cr);
      cairo_surface_destroy (surface);

      return 0;
    }

    virtual VampOutputList getVampPlugins()
    {
      VampOutputList pluginList;

      VampPlugin libxHigh = {"vamp-libxtract:highest_value", 0, 0};
      VampOutput high = {libxHigh, "highest_value"};
      VampPlugin libxLow = {"vamp-libxtract:lowest_value", 0, 0};
      VampOutput low = {libxLow, "lowest_value"};
      VampPlugin libxSpecCent = {"vamp-libxtract:spectral_centroid", 0, 0};
      VampOutput specCent = {libxSpecCent, "spectral_centroid"};

      pluginList.push_back(high);
      pluginList.push_back(low);
      pluginList.push_back(specCent);
      return pluginList;
    }
};

extern "C" VisPlugin* create() {
    return new SpecCont;
}

extern "C" void destroy(VisPlugin* p) {
    delete p;
}
