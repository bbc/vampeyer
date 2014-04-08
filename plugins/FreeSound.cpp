#include "VisPlugin.h"
#include <cairo/cairo.h>
#include <math.h>

#define MIN_FREQ 100
#define MAX_FREQ 22050
#define BG_COLOUR 0, 0, 0, 1
#define PALETTE_RED {0.196, 0, 1, 1}
#define PALETTE_GREEN {0, 0.863, 0.878, 0.274}
#define PALETTE_BLUE {0.784, 0.314, 0, 0}
#define PALETTE_LENGTH 4

class SpecCent: public VisPlugin {

private:

  // given a list of gradient values, return the value of a point
  // on that gradient
  //
  // e.g. color={1.0, 0.5, 0.0}
  // colormap(0.25, color, 3) returns 0.75
  double colormap(double value, double color[], int length)
  {
    int iter=0;
    double step = 1.0/(double)length;
    if (value>1) return 0;
    while (value > step*(iter+1)) iter++;
    double weight = (value-step*(double)iter)/step;
    return (1.0-weight)*color[iter] + weight*color[iter+1];
  }

  double red(double value)
  {
    double colors[] = PALETTE_RED;
    return colormap(value, colors, PALETTE_LENGTH);
  }

  double green(double value)
  {
    double colors[] = PALETTE_GREEN;
    return colormap(value, colors, PALETTE_LENGTH);
  }

  double blue(double value)
  {
    double colors[] = PALETTE_BLUE;
    return colormap(value, colors, PALETTE_LENGTH);
  }

public:

    virtual double getVersion() const {
        return 0.1;
    }

    virtual int ARGB(Plugin::FeatureSet features, int width,
        int height, unsigned char *bitmap, int sampleRate)
    {
      const double lower = MIN_FREQ;
      const double higher = MAX_FREQ;
      const double lower_log = log10(lower);
      const double higher_log = log10(higher);

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

      // find number of frames for peak/spec centroid
      unsigned int peakFrames = features[0].size();
      unsigned int scFrames = features[1].size();
      double scScale = (double)scFrames/(double)peakFrames;

      // for each peak frame, draw a colored line
      for (unsigned int peakFrame=0; peakFrame<peakFrames; peakFrame++)
      {
        // find spectral contrast, clip and scale
        double sc = features[1].at(floor(scScale*peakFrame)).values[0];
        if (sc < lower) sc=lower;
        if (sc > higher) sc=higher;
        sc = (log10(sc)-lower_log)/(higher_log-lower_log);

        // get peak values
        double peak1 = features[0].at(peakFrame).values[0];
        double peak2 = features[0].at(peakFrame).values[1];
        
        // draw waveform
        cairo_set_source_rgba(cr, red(sc), green(sc), blue(sc), 1.0);
        cairo_line_to(cr, (double)peakFrame/(double)peakFrames,
                          0.5-(peak1*0.5));
        cairo_line_to(cr, (double)peakFrame/(double)peakFrames,
                          0.5-(peak2*0.5));
        cairo_set_line_width (cr, 0.1/(double)width);
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
      VampPlugin libxSpecCent = {"vamp-libxtract:spectral_centroid", 0, 0};
      VampOutput specCent = {libxSpecCent, "spectral_centroid"};

      pluginList.push_back(peaks);
      pluginList.push_back(specCent);
      return pluginList;
    }
};

extern "C" VisPlugin* create() {
    return new SpecCent;
}

extern "C" void destroy(VisPlugin* p) {
    delete p;
}
