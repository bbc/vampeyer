#include "VisPlugin.h"
#include <cairo/cairo.h>
#include <cmath>
#include <iostream>

using std::cout;
using std::endl;

class MFCC: public VisPlugin {

private:
    static const double pi = 3.14159265358979323846264338327950288;
    void IDCT(double **feats, double **results, unsigned int filters,
        unsigned int frames)
    {
      double matrix[filters][filters];
      for (unsigned int k=0; k<filters; k++)
      {
        for (unsigned int j=1; j<filters; j++)
        {
          matrix[k][j] = sqrt(2./(double)filters)*cos(pi/filters*j*(k+0.5));
        }
      }

      double min = 0;
      double max = 0;
      for (unsigned int frame=0; frame<frames; frame++)
      {
        for (unsigned int k=0; k<filters; k++)
        {
          results[frame][k] = sqrt(2)/2*feats[frame][0];
          for (unsigned int j=1; j<filters; j++)
            results[frame][k] += matrix[k][j]*feats[frame][j]; 
          if (results[frame][k] < min) min=results[frame][k];
          if (results[frame][k] > max) max=results[frame][k];
        }
      }
    }

public:

    virtual double getVersion() const {
        return 0.1;
    }

    virtual int ARGB(Plugin::FeatureSet features, int width,
        int height, unsigned char *bitmap)
    {
      unsigned int frames = features[0].size();
      unsigned int filters = features[0].at(0).values.size();

      double **feats;
      double **results;
      feats = new double*[frames];
      results = new double*[frames];
      for (unsigned int i=0; i<frames; i++) {
        results[i] = new double[filters];
        feats[i] = new double[filters];
      }

      for (unsigned int i=0; i<frames; i++) {
        for (unsigned int j=0; j<filters; j++) {
          feats[i][j] = features[0].at(i).values.at(j);
        }
      }

      IDCT(feats, results, filters, frames);

      // find min/max values
      double min=0;
      double max=1;
      for (unsigned int i=0; i<frames; i++)
      {
        for (unsigned int j=0; j<filters; j++)
        {
          if (results[i][j] < min) min=results[i][j];
          if (results[i][j] > max) max=results[i][j];
        }
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
      for (unsigned int frame=0; frame<frames; frame++)
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
