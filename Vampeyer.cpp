/*
   Copyright 2014 British Broadcasting Corporation

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "plugins/VisPlugin.h"
#include "VisHost.h"
#include "VampHost.h"
#include "GUI.h"
#include "PNGWriter.h"
#include <iostream>
#include <dlfcn.h>
#include <sstream>
#include <string>
#include <sndfile.h>
#include <tclap/CmdLine.h>

#define BYTES_PER_PIXEL 4

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::flush;
using std::istringstream;

int main(int argc, char** argv)
{
  bool verbose;
  string pngfile, visPluginPath, wavfile, size;
  int width=0, height=0;

  // parse command line arguments
  try
  {
    TCLAP::CmdLine cmd("Audio visualiser", ' ', "0.1");
    TCLAP::UnlabeledValueArg<string> wavFileArg("wavFile",
        "Path of audio file", true, "", "filename.wav");
    TCLAP::ValueArg<string> visPluginArg("p", "plugin",
        "Path of visualization plugin", true, "", "library.so");
    TCLAP::ValueArg<string> pngFileArg("o", "pngFile",
        "File to save output PNG", false, "", "filename.png");
    TCLAP::ValueArg<string> sizeArg("s", "size",
        "Size of output image in pixels", false, "600x200",
        "width>x<height");
    TCLAP::SwitchArg verboseArg("V", "verbose", "Enable verbose output",
        false);

    cmd.add(wavFileArg);
    cmd.add(visPluginArg);
    cmd.add(pngFileArg);
    cmd.add(sizeArg);
    cmd.add(verboseArg);

    // parse arguments
    cmd.parse(argc, argv);
    wavfile = wavFileArg.getValue();
    visPluginPath = visPluginArg.getValue();
    pngfile = pngFileArg.getValue();
    size = sizeArg.getValue();
    verbose = verboseArg.getValue();

    // parse size
    istringstream ss(size);
    string widthStr, heightStr;
    getline( ss, widthStr, 'x' );
    getline( ss, heightStr );
    istringstream(widthStr) >> width;
    istringstream(heightStr) >> height;

    // check size is valid 
    if (width <= 0 || height <= 0)
    {
      cerr << "ERROR: Could not parse size argument." << endl;
      return 1;
    }

  } catch (TCLAP::ArgException &e)
  {
    cerr << "ERROR: " << e.error() << " for arg " << e.argId() << endl;
    return 1;
  }

  // declare plugin and buffer space
  unsigned char* buffer = new unsigned char[width*height*BYTES_PER_PIXEL];
  VisHost visHost(visPluginPath);

  // set verbosity level
  visHost.verbose = verbose;

  // initialise plugin 
  if (visHost.init()) {
    cerr << "ERROR: Could not initialise visualisation plugin." << endl;
    return 1;
  }

  // run audio analysis
  if (visHost.process(wavfile)) {
    cerr << "ERROR: Could not process audio file." << endl;
    return 1;
  }

  // draw visualisation
  if (visHost.render(width, height, buffer)) {
    cerr << "ERROR: Could not render visualisation." << endl;
    return 1;
  }

  // if filename is set, write png
  if (pngfile != "")
  {
    if (verbose) cout << " * Writing PNG..." << flush;
    PNGWriter pngWriter(width, height, buffer);
    if (pngWriter.write(pngfile.c_str())) {
      cerr << "ERROR: Failed to write PNG." << endl;
      return 1;
    }
    if (verbose) cout << " [done]" << endl;
  }
  // otherwise, display image on screen
  else
  {
    if (verbose) cout << " * Displaying image..." << flush;
    GUI window(width, height, buffer);
    if (verbose) cout << " [done]" << endl;
  }
}
