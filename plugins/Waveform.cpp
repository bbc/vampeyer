#include "VisPlugin.h"
#include <cairo/cairo.h>
#include <math.h>

#define BG_COLOUR 0.866, 0.874, 0.882, 1
#define WAVEFORM_COLOUR 0.38, 0.423, 1, 1

class Waveform: public VisPlugin {

public:

    virtual double getVersion() const {
        return 0.1;
    }

    virtual int ARGB(Plugin::FeatureSet features, int width,
        int height, unsigned char *bitmap, int sampleRate)
    {
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
      cairo_set_source_rgba(cr, BG_COLOUR);
      cairo_paint(cr);

      cairo_set_source_rgba(cr, WAVEFORM_COLOUR);
      cairo_set_line_width (cr, 1.0/(double)width);

      // find number of frames for peak/spec centroid
      unsigned int peakFrames = features[0].size();

      // for each peak frame, draw a colored line
      for (unsigned int peakFrame=0; peakFrame<peakFrames; peakFrame++)
      {
        // get peak values
        double peak1 = features[0].at(peakFrame).values[0];
        double peak2 = features[0].at(peakFrame).values[1];
        
        // draw waveform
        cairo_line_to(cr, (double)peakFrame/(double)peakFrames,
                          0.5-(peak1*0.5));
        cairo_line_to(cr, (double)peakFrame/(double)peakFrames,
                          0.5-(peak2*0.5));
        cairo_stroke (cr);
      }

      // clean up
      cairo_destroy(cr);
      cairo_surface_destroy (surface);

      return 0;
    }

    virtual VampOutputList getVampPlugins()
    {
      VampOutputList pluginList;

      VampPlugin bbcPeaks = {"bbc-vamp-plugins:bbc-peaks", 0, 0};
      VampOutput peaks = {bbcPeaks, "peaks"};

      pluginList.push_back(peaks);
      return pluginList;
    }
};

extern "C" VisPlugin* create() {
    return new Waveform;
}

extern "C" void destroy(VisPlugin* p) {
    delete p;
}
