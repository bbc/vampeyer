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
using std::flush;

int main(int argc, char** argv)
{
  bool verbose;
  string pngfile, visPluginPath, wavfile, size;
  int width=0, height=0;
  map<VisPlugin::VampPlugin, vector<Plugin::FeatureSet> > vampResults;
  map<VisPlugin::VampPlugin, VampHost*> vampHosts;
  set<VisPlugin::VampPlugin> vampPlugins;

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
 
  // load the visualization library
  if (verbose) cout << " * Loading visualization plugin..." << flush;
  void* handle = dlopen(visPluginPath.c_str(), RTLD_LAZY);
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
  VisPlugin* visPlugin = create_plugin();
  if (verbose) cout << " [done]" << endl;

  // create set of unique plugins
  VisPlugin::VampOutputList vampOuts = visPlugin->getVampPlugins();
  for (VisPlugin::VampOutputList::iterator o=vampOuts.begin();
       o!=vampOuts.end(); o++)
  {
    VisPlugin::VampOutput out = *o;
    vampPlugins.insert(out.plugin);
  }

  // for each unique plugin
  for (set<VisPlugin::VampPlugin>::iterator p=vampPlugins.begin();
       p!=vampPlugins.end(); p++)
  {
    // load the plugin
    VisPlugin::VampPlugin plugin = *p;
    if (verbose) cout << " * Processing Vamp plugin " << plugin.name << "..."
      << flush;

    SNDFILE *sndfile;
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(SF_INFO));
    sndfile = sf_open(wavfile.c_str(), SFM_READ, &sfinfo);
    if (!sndfile) {
      cerr << ": ERROR: Failed to open input file \""
        << wavfile << "\": " << sf_strerror(sndfile) << endl;
      destroy_plugin(visPlugin);
      dlclose(handle);
      return 1;
    }
    vampHosts[plugin] = new VampHost(sndfile, sfinfo, plugin.name);

    // process audio file
    if (vampHosts[plugin]->run(vampResults[plugin])) {
      cerr << "ERROR: Vamp plugin " << plugin.name
        << " could not process audio." << endl;
      destroy_plugin(visPlugin);
      dlclose(handle);
      return 1;
    }
    if (verbose) cout << " [done]" << endl;
  }

  int count=0;
  Plugin::FeatureSet resultsFilt;
  for (VisPlugin::VampOutputList::iterator o=vampOuts.begin();
       o!=vampOuts.end(); o++)
  {
    VisPlugin::VampOutput out = *o;
    if (verbose) cout << " * Refactoring data for "
      << out.plugin.name << ":" << out.name << "..." << flush;
    Plugin::FeatureList featList;
    int outNum = vampHosts[out.plugin]->findOutputNumber(out.name);
    for (unsigned int i=0; i<vampResults[out.plugin].size(); i++)
    {
      Plugin::FeatureList feats = vampResults[out.plugin].at(i)[0];
      if (feats.size() >= (unsigned int)outNum+1)
        featList.push_back(feats.at(outNum));
    }
    resultsFilt[count] = featList;
    count++;
    if (verbose) cout << " [done]" << endl;
  }

  // set up memory for bitmap
  if (verbose) cout << " * Processing visualization..." << flush;
  unsigned char buffer[width*height*BYTES_PER_PIXEL];

  // get bitmap from library
  if (visPlugin->ARGB(resultsFilt, width, height, buffer)) {
    cerr << "ERROR: Plugin failed to produce bitmap." << endl;
    destroy_plugin(visPlugin);
    dlclose(handle);
    return 1;
  }
  if (verbose) cout << " [done]" << endl;

  if (pngfile != "")
  {
    if (verbose) cout << " * Writing PNG..." << flush;
    PNGWriter pngWriter(width, height, buffer);
    if (pngWriter.write(pngfile.c_str())) {
      cerr << "ERROR: Failed to write PNG." << endl;
      destroy_plugin(visPlugin);
      dlclose(handle);
      return 1;
    }
    if (verbose) cout << " [done]" << endl;
  }
  else
  {
    if (verbose) cout << " * Displaying image..." << flush;
    GUI window(width, height, buffer);
    if (verbose) cout << " [done]" << endl;
  }

  // clean up
  for (set<VisPlugin::VampPlugin>::iterator p=vampPlugins.begin();
       p!=vampPlugins.end(); p++)
  {
    VisPlugin::VampPlugin plugin = *p;
    delete vampHosts[plugin];
  }
  destroy_plugin(visPlugin);
  dlclose(handle);
}
