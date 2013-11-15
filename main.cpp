#include "plugins/VisPlugin.h"
#include "VampHost.h"
#include "GUI.h"
#include <iostream>
#include <dlfcn.h>
#include <png.h>
#include <sstream>
#include <string>
#include <sndfile.h>
#include <tclap/CmdLine.h>

#define BYTES_PER_PIXEL 4

using std::cout;
using std::cerr;
using std::endl;
using std::string;

int writePNG(const char* filename, int width, int height, unsigned char *buffer)
{
  // convert 2d array into array of pointers
  unsigned char* row_pointers[height];
  for (int i=0; i<height; i++)
    row_pointers[i] = &buffer[i*width*BYTES_PER_PIXEL];

  // open file for writing
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    cerr << "Could not open file to write PNG." << endl;
    return 1;
  }

  // set up PNG struct
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL, NULL, NULL);
  if (!png_ptr) {
    cerr << "Could not allocate PNG write struct." << endl;
    fclose(fp);
    return 1;
  }

  // set up PNG info struct
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    cerr << "Could not allocate PNG info struct." << endl;
    fclose(fp);
    return 1;
  }

  // initialise PNG writing
  png_init_io(png_ptr, fp);

  // write header
  png_set_IHDR(png_ptr, info_ptr, width, height,
      8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  // write image
  png_set_rows(png_ptr, info_ptr, row_pointers);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, NULL);

  // clean up
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(fp);

  return 0;
}

int main(int argc, char** argv)
{
  bool verbose;
  string pngfile, visplugin, wavfile;

  // parse command line arguments
  try
  {
    TCLAP::CmdLine cmd("Audio visualiser", ' ', "0.1");
    TCLAP::UnlabeledValueArg<string> wavFileArg("wavFile",
        "Path of audio file", true, "", "string");
    TCLAP::ValueArg<string> visPluginArg("p", "visPlugin",
        "Path of visualization plugin", true, "", "string");
    TCLAP::ValueArg<string> pngFileArg("o", "pngFile",
        "File to save output PNG", false, "", "string");
    TCLAP::SwitchArg verboseArg("V", "verbose", "Enable verbose output",
        false);

    cmd.add(wavFileArg);
    cmd.add(visPluginArg);
    cmd.add(pngFileArg);
    cmd.add(verboseArg);

    cmd.parse(argc, argv);

    wavfile = wavFileArg.getValue();
    visplugin = visPluginArg.getValue();
    pngfile = pngFileArg.getValue();
    verbose = verboseArg.getValue();

  } catch (TCLAP::ArgException &e)
  {
    cerr << "ERROR: " << e.error() << " for arg " << e.argId() << endl;
    return 1;
  }

  // load the test library
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
  VisPlugin* test = create_plugin();
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
    destroy_plugin(test);
    dlclose(handle);
    return 1;
  }
  if (verbose) cout << " [done]" << endl;

  // find and parse vamp plugin name
  if (verbose) cout << " * Loading Vamp plugins...";
  istringstream ss(test->getVampPlugin());
  string vampGroup, vampPlugin, vampOutput;
  getline( ss, vampGroup, ':' );
  getline( ss, vampPlugin, ':' );
  getline( ss, vampOutput, ':' );

  // initialise vamp host
  VampHost host(sndfile, sfinfo, vampGroup+":"+vampPlugin);
  if (host.findOutputNumber(vampOutput) < 0) {
    cerr << "ERROR: Requested Vamp output could not be found." << endl;
    destroy_plugin(test);
    dlclose(handle);
    return 1;
  }
  if (verbose) cout << " [done]" << endl;

  // process audio file
  if (verbose) cout << " * Processing audio...";
  vector<Plugin::FeatureSet> results;
  if (host.run(results)) {
    cerr << "ERROR: Vamp plugin could not process audio." << endl;
    destroy_plugin(test);
    dlclose(handle);
    return 1;
  }
  if (verbose) cout << " [done]" << endl;

  // set up memory for bitmap
  if (verbose) cout << " * Generating image...";
  int width=800;
  int height=300;
  unsigned char buffer[width*height*BYTES_PER_PIXEL];

  // get bitmap from library
  if (test->ARGB(results, width, height, buffer)) {
    cerr << "ERROR: Plugin failed to produce bitmap." << endl;
    destroy_plugin(test);
    dlclose(handle);
    return 1;
  }
  if (verbose) cout << " [done]" << endl;

  if (pngfile != "")
  {
    if (verbose) cout << " * Writing PNG...";
    if (writePNG(pngfile.c_str(), width, height, buffer)) {
      cerr << "ERROR: Failed to write PNG." << endl;
      destroy_plugin(test);
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
  destroy_plugin(test);
  dlclose(handle);
}
