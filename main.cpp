#include "plugins/VisPlugin.h"
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

int main(int argc, char** argv)
{
  bool verbose;
  string pngfile, visplugin, wavfile, size;
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
    visplugin = visPluginArg.getValue();
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
 
  // load the visualization library
  if (verbose) cout << " * Loading visualization plugin...";
  void* handle = dlopen(visplugin.c_str(), RTLD_LAZY);
  if (!handle) {
    cerr << "ERROR: Cannot load library: " << dlerror() << endl;
    return 1;
  }

  // reset errors
  dlerror();

  // load the symbols
  create_t* create_plugin = (create_t*) dlsym(handle, "create");
  const char* dlsym_error = dlerror();
  if (dlsym_error) {
    cerr << "ERROR: Cannot load symbol create: " << dlsym_error << endl;
    dlclose(handle);
    return 1;
  }
  destroy_t* destroy_plugin = (destroy_t*) dlsym(handle, "destroy");
  dlsym_error = dlerror();
  if (dlsym_error) {
    cerr << "ERROR: Cannot load symbol destroy: " << dlsym_error << endl;
    dlclose(handle);
    return 1;
  }

  // create an instance of the class
  VisPlugin* plugin = create_plugin();
  if (verbose) cout << " [done]" << endl;

  // open .wav file
  if (verbose) cout << " * Loading audio file...";
  SNDFILE *sndfile;
  SF_INFO sfinfo;
  memset(&sfinfo, 0, sizeof(SF_INFO));
  sndfile = sf_open(wavfile.c_str(), SFM_READ, &sfinfo);
  if (!sndfile) {
    cerr << ": ERROR: Failed to open input file \""
      << wavfile << "\": " << sf_strerror(sndfile) << endl;
    destroy_plugin(plugin);
    dlclose(handle);
    return 1;
  }
  if (verbose) cout << " [done]" << endl;

  // find and parse vamp plugin name
  if (verbose) cout << " * Loading Vamp plugins...";
  istringstream ss(plugin->getVampPlugin());
  string vampGroup, vampPlugin, vampOutput;
  getline( ss, vampGroup, ':' );
  getline( ss, vampPlugin, ':' );
  getline( ss, vampOutput, ':' );

  // initialise vamp host
  VampHost host(sndfile, sfinfo, vampGroup+":"+vampPlugin);
  if (host.findOutputNumber(vampOutput) < 0) {
    cerr << "ERROR: Requested Vamp output could not be found." << endl;
    destroy_plugin(plugin);
    dlclose(handle);
    return 1;
  }
  if (verbose) cout << " [done]" << endl;

  // process audio file
  if (verbose) cout << " * Processing audio...";
  vector<Plugin::FeatureSet> results;
  if (host.run(results)) {
    cerr << "ERROR: Vamp plugin could not process audio." << endl;
    destroy_plugin(plugin);
    dlclose(handle);
    return 1;
  }
  if (verbose) cout << " [done]" << endl;

  if (verbose) cout << " * Refactoring data...";
  Plugin::FeatureSet resultsFilt;
  //TODO for each output
  Plugin::FeatureList featList;
  int outNum = host.findOutputNumber(vampOutput);
  for (unsigned int i=0; i<results.size(); i++)
  {
    Plugin::FeatureList feats = results.at(i)[0];
    if (feats.size() >= (unsigned int)outNum+1)
      featList.push_back(feats.at(outNum));
  }
  resultsFilt[0] = featList;
  if (verbose) cout << " [done]" << endl;

  // set up memory for bitmap
  if (verbose) cout << " * Generating image...";
  unsigned char buffer[width*height*BYTES_PER_PIXEL];

  // get bitmap from library
  if (plugin->ARGB(resultsFilt, width, height, buffer)) {
    cerr << "ERROR: Plugin failed to produce bitmap." << endl;
    destroy_plugin(plugin);
    dlclose(handle);
    return 1;
  }
  if (verbose) cout << " [done]" << endl;

  if (pngfile != "")
  {
    if (verbose) cout << " * Writing PNG...";
    PNGWriter pngWriter(width, height, buffer);
    if (pngWriter.write(pngfile.c_str())) {
      cerr << "ERROR: Failed to write PNG." << endl;
      destroy_plugin(plugin);
      dlclose(handle);
      return 1;
    }
    if (verbose) cout << " [done]" << endl;
  }
  else
  {
    if (verbose) cout << " * Displaying image...";
    GUI window(width, height, buffer);
    if (verbose) cout << " [done]" << endl;
  }

  // clean up
  destroy_plugin(plugin);
  dlclose(handle);
}
