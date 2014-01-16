#include "VisPlugin.h"
#include <cairo/cairo.h>

class AmpMFCC: public VisPlugin {

public:

    virtual double getVersion() const {
        return 0.1;
    }

    virtual int ARGB(Plugin::FeatureSet features, int width,
        int height, unsigned char *bitmap, int sampleRate)
    {
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
      frames = features[0].size();
      for (int frame=0; frame<frames; frame++)
      {
        for (unsigned int coeff=0; coeff<coeffs; coeff++)
        {
          float value = features[0].at(frame).values.at(coeff); 
          if (value < -5.0) value = -5.0;
          if (value > 1.0) value = 1.0;
          value = (value + 5.0) / 6.0;
          cairo_set_source_rgba(cr, value,
                                    value,
                                    value, 1);
          cairo_rectangle(cr, (double)frame/(double)frames,
                              (double)coeff/(double)coeffs,
                              1.0/(double)frames,
                              1.0/(double)coeffs);
          cairo_fill(cr);
        }
      }

      // start line in bottom left corner
      cairo_set_line_width(cr, 0);
      cairo_set_source_rgba(cr, 0, 0, 0, 1);
      cairo_move_to(cr, 0, 1);

      frames = features[1].size();
      for (int frame=0; frame<frames; frame++)
      {
        Plugin::Feature feats = features[1].at(frame);
        double amp = feats.values[0];
        cairo_line_to(cr, (double)frame/(double)frames, 1.0-amp);
      }

      // finish line
      cairo_line_to(cr, 1, 1);
      cairo_close_path(cr);
      cairo_fill(cr);

      // clean up
      cairo_destroy(cr);
      cairo_surface_destroy (surface);

      return 0;
    }

    virtual VampOutputList getVampPlugins()
    {
      VampOutputList pluginList;

      VampPlugin qmAmp = {"vamp-example-plugins:amplitudefollower", 0, 0};
      VampOutput amp = {qmAmp, "amplitude"};
      VampPlugin qmMFCC = {"qm-vamp-plugins:qm-mfcc", 0, 0};
      VampOutput mfcc = {qmMFCC, "coefficients"};

      pluginList.push_back(mfcc);
      pluginList.push_back(amp);
      return pluginList;
    }
};

extern "C" VisPlugin* create() {
    return new AmpMFCC;
}

extern "C" void destroy(VisPlugin* p) {
    delete p;
}
