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

      // find number of frames for peak/rms
      unsigned int peakFrames = features[0].size();
      unsigned int rmsFrames = features[1].size();
      unsigned int zcrFrames = features[2].size();

      double rmsGlobalMax=0.0;
      for (unsigned int peakFrame=0; peakFrame<peakFrames-1; peakFrame++)
      {
        //unsigned int rmsStart = floor((double)peakFrame/
            //(double)peakFrames*(double)rmsFrames);
        //unsigned int rmsEnd = floor((double)(peakFrame+1)/
            //(double)peakFrames*(double)rmsFrames)-1;

        //double rmsLocalMean = 1.0;
        //for (unsigned int rmsFrame=rmsStart; rmsFrame<rmsEnd+1; rmsFrame++)
        //{
          //double rms = features[1].at(rmsFrame).values[0];
          //rmsLocalMean+=rms;
        //}
        //rmsLocalMean = rmsLocalMean / (rmsEnd-rmsStart+1);

        //if (rmsLocalMean != 1.0 && rmsLocalMean > rmsGlobalMax)
          //rmsGlobalMax = rmsLocalMean;
        double rmszcr = features[1].at(peakFrame).values[0]*
          features[2].at(peakFrame).values[0];
        if (rmszcr > rmsGlobalMax)
         rmsGlobalMax = rmszcr; 
      }

      // for each peak frame, draw a colored line
      for (unsigned int peakFrame=0; peakFrame<peakFrames-1; peakFrame++)
      {
        // find start and end of RMS values
        //unsigned int rmsStart = floor((double)peakFrame/
            //(double)peakFrames*(double)rmsFrames);
        //unsigned int rmsEnd = floor((double)(peakFrame+1)/
            //(double)peakFrames*(double)rmsFrames)-1;

        //double rmsMean=1.0;
        //for (unsigned int rmsFrame=rmsStart; rmsFrame<rmsEnd+1; rmsFrame++)
        //{
          //double rms = features[1].at(rmsFrame).values[0];
          //rmsMean+=rms;
        //}
        //rmsMean = rmsMean / (rmsEnd-rmsStart+1);
        //cairo_set_source_rgba(cr, 1.0-rmsMean/rmsGlobalMax,
                                  //1.0-rmsMean/rmsGlobalMax,
                                  //1.0-rmsMean/rmsGlobalMax, 1);
        double rmszcr = features[1].at(peakFrame).values[0]*
          features[2].at(peakFrame).values[0]/rmsGlobalMax;
        cairo_set_source_rgba(cr, rmszcr, rmszcr, rmszcr, 1); 
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

      VampPlugin bbcRMS = {"bbc-vamp-plugins:bbc-energy", 1024, 1024};
      VampOutput rms = {bbcRMS, "rmsenergy"};

      VampPlugin libxZCR = {"vamp-libxtract:zcr", 1024, 1024};
      VampOutput zcr = {libxZCR, "zcr"};

      pluginList.push_back(peaks);
      pluginList.push_back(rms);
      pluginList.push_back(zcr);
      return pluginList;
    }
};

extern "C" VisPlugin* create() {
    return new Waveform;
}

extern "C" void destroy(VisPlugin* p) {
    delete p;
}
