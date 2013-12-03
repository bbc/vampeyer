#include "VisPlugin.h"
#include <cairo/cairo.h>
#include <cmath>
#include <iostream>

using std::cout;
using std::endl;

class MFCC: public VisPlugin {

public:

    virtual double getVersion() const {
        return 0.1;
    }

    virtual int ARGB(Plugin::FeatureSet features, int width,
        int height, unsigned char *bitmap)
    {
      const double pi = 3.14159265358979323846264338327950288;
      unsigned int coeffs = features[0].at(0).values.size();
      unsigned int filters = 40;

      double IDCT[filters][coeffs];
      for (unsigned int k=0; k<filters; k++)
      {
        for (unsigned int j=1; j<coeffs; j++)
        {
          IDCT[k][j] = sqrt(2./(double)coeffs)*cos(pi/coeffs*j*(k+0.5));
          //cout << IDCT[k][j] << ",";
        }
        //cout << endl;
      }

      int frames = features[0].size();
      double results[frames][filters];
      double min = 0;
      double max = 0;
      for (int frame=0; frame<frames; frame++)
      {
        for (unsigned int k=0; k<filters; k++)
        {
          results[frame][k] = sqrt(2)/2*features[0].at(frame).values.at(0);
          for (unsigned int j=1; j<coeffs; j++)
          {
            results[frame][k] += IDCT[k][j]*features[0].at(frame).values.at(j); 
          }
          //cout << results[frame][k] << ",";
          if (results[frame][k] < min) min=results[frame][k];
          if (results[frame][k] > max) max=results[frame][k];
        }
        //cout << endl;
      }

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

      // draw amplitude
      frames = features[0].size();
      for (int frame=0; frame<frames; frame++)
      {
        for (unsigned int k=0; k<filters; k++)
        {
          double value = (results[frame][k]-min)/(max-min);
          cairo_set_source_rgba(cr, 1.-value,
                                    1.-value,
                                    1.-value, 1);
          cairo_rectangle(cr, (double)frame/(double)frames,
                              (double)k/(double)filters,
                              1.0/(double)frames,
                              1.0/(double)filters);
          cairo_fill(cr);
        }
      }

      // clean up
      cairo_destroy(cr);
      cairo_surface_destroy (surface);

      return 0;
    }

    virtual VampOutputList getVampPlugins()
    {
      VampOutputList pluginList;

      VampPlugin qmMFCC = {"qm-vamp-plugins:qm-mfcc", 0, 0};
      VampOutput mfcc = {qmMFCC, "coefficients"};

      pluginList.push_back(mfcc);
      return pluginList;
    }
};

extern "C" VisPlugin* create() {
    return new MFCC;
}

extern "C" void destroy(VisPlugin* p) {
    delete p;
}
