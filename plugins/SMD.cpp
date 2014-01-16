#include "VisPlugin.h"
#include <cairo/cairo.h>
#include <math.h>
#include <iostream>
#include <cmath>
#include <algorithm>

using std::cout;
using std::endl;

#define AUBIO_STEP 256
#define AUBIO_THRESH -40.f
#define PALETTE_RED {0.196, 0, 1, 1}
#define PALETTE_GREEN {0, 0.863, 0.878, 0.274}
#define PALETTE_BLUE {0.784, 0.314, 0, 0}
#define PALETTE_LENGTH 4

class SMD: public VisPlugin {

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
  
  bool between(double cur, double *start, double *end, unsigned int length)
  {
    for (unsigned int i=0; i<length; i++)
      if (cur>start[i] && cur<end[i]) return true;
    return false;
  }
  
  double closest(double cur, double *times, unsigned int length)
  {
    double minDist=99999999;
    for (unsigned int i=0; i<length; i++)
    {
      double dist=std::abs(cur-times[i]);
      if (dist < minDist) minDist=dist;
    }
    return minDist;
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
      cairo_set_source_rgba(cr, 1, 1, 1, 1);
      cairo_paint(cr);

      // find number of frames for peak/spec centroid
      unsigned int peakFrames = features[0].size();
      unsigned int silentPoints = features[1].size();
      double *silentStart = new double[silentPoints];
      double *silentEnd = new double[silentPoints];
      for (unsigned int silentPoint=0; silentPoint<silentPoints; silentPoint++)
      {
        RealTime time = features[1].at(silentPoint).timestamp;
        RealTime dur = features[1].at(silentPoint).duration;
        silentStart[silentPoint] = (double)time.sec + (double)time.nsec/1000000000;
        silentEnd[silentPoint] = silentStart[silentPoint] + (double)dur.sec \
                                 + (double)dur.nsec/1000000000;
      }

      // for each peak frame, draw a colored line
      for (unsigned int peakFrame=0; peakFrame<peakFrames; peakFrame++)
      {
        // calculate time of frame
        double time = AUBIO_STEP/(double)sampleRate*peakFrame; 

        // if frame is in a silence, draw as black
        double dist=0;
        if (!between(time, silentStart, silentEnd, silentPoints))
        {
          double startDist = closest(time, silentStart, silentPoints);
          double endDist = closest(time, silentEnd, silentPoints);
          dist=std::max(startDist, endDist);
          if (dist>1) dist=1.0;
        }
        cairo_set_source_rgba(cr, dist, dist, dist, 1.0);
        cairo_rectangle(cr, (double)peakFrame/(double)peakFrames,
            0.0,
            2.0/(double)peakFrames, // TODO Fix bodge
            1.0);
        cairo_fill(cr);

        // get peak values
        //double peak1 = features[0].at(peakFrame).values[0];
        //double peak2 = features[0].at(peakFrame).values[1];
        
        // draw waveform
        //cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
        //cairo_line_to(cr, (double)peakFrame/(double)peakFrames,
                          //0.5-(peak1*0.5));
        //cairo_line_to(cr, (double)peakFrame/(double)peakFrames,
                          //0.5-(peak2*0.5));
        //cairo_set_line_width (cr, 1.0/(double)peakFrames);
        //cairo_stroke (cr);
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

      VampParameterList aubioSilentParams;
      VampParameter thresh = {"silencethreshold", AUBIO_THRESH};
      aubioSilentParams.push_back(thresh);
      VampPlugin aubioSilent = {"vamp-aubio:aubiosilence",
                                AUBIO_STEP,
                                AUBIO_STEP,
                                aubioSilentParams};
      VampOutput silent = {aubioSilent, "silent"};

      pluginList.push_back(peaks);
      pluginList.push_back(silent);
      return pluginList;
    }
};

extern "C" VisPlugin* create() {
    return new SMD;
}

extern "C" void destroy(VisPlugin* p) {
    delete p;
}
