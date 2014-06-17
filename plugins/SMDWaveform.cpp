#include "VisPlugin.h"
#include <cairo/cairo.h>
#include <math.h>

#define BG_COLOUR 0.866, 0.874, 0.882, 1
#define WAVEFORM_COLOUR 0.38, 0.423, 1, 1
#define PALETTE_RED {0.38, 0.38, 1.0}
#define PALETTE_GREEN {0.423, 0.423, 0.38}
#define PALETTE_BLUE {1.0, 1.0, 0.38}
#define PALETTE_LENGTH 3

using std::cerr;
using std::endl;

class Waveform: public VisPlugin {

private:

  // given a list of gradient values, return the value of a point
  // on that gradient
  //
  // e.g. color={1.0, 0.5, 0.0}
  // colormap(0.25, color, 3) returns 0.75
  double colormap(double value, double color[], int length)
  {
    int iter=0;
    double step = 1.0/(double)(length-1);
    if (value>=1) return color[length-1];
    if (value<=0) return color[0];
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

      // find number of frames for peak/rms
      unsigned int peakFrames = features[0].size();
      unsigned int dipFrames = features[1].size();

      if (peakFrames != dipFrames) cerr << "Warning: Frames for input features\
        do not match!" << endl;

      // for each peak frame, draw a colored line
      for (unsigned int peakFrame=0; peakFrame<peakFrames-1; peakFrame++)
      {
        // get pDip value and set colour
        double dip = features[1].at(peakFrame).values[0];
        cairo_set_source_rgba(cr, red(dip), green(dip), blue(dip), 1);

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

      VampPlugin bbcPeaks = {"bbc-vamp-plugins:bbc-peaks", 1024, 1024};
      VampOutput peaks = {bbcPeaks, "peaks"};

      VampPlugin bbcEnergy = {"bbc-vamp-plugins:bbc-energy", 1024, 1024};
      VampOutput pdip = {bbcEnergy, "pdip"};

      pluginList.push_back(peaks);
      pluginList.push_back(pdip);
      return pluginList;
    }
};

extern "C" VisPlugin* create() {
    return new Waveform;
}

extern "C" void destroy(VisPlugin* p) {
    delete p;
}
