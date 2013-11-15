#include "plugins/VisPlugin.h"
#include "VampHost.h"
#include <iostream>
#include <dlfcn.h>
#include <png.h>
#include <sstream>
#include <string>
#include <sndfile.h>

#define BYTES_PER_PIXEL 4

using std::cout;
using std::cerr;
using std::endl;
using std::string;

int writePNG(char* filename, int width, int height, unsigned char *buffer)
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
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
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

int main()
{
    char pngfile[] = "test.png";
    char visplugin[] = "./plugins/TestPlugin.so";
    char wavfile[] = "/data/audio/music/Radiohead/The King Of Limbs [2011]/01 Bloom.wav";

    // load the test library
    void* handle = dlopen(visplugin, RTLD_LAZY);
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

    // print version number of library
    cout << "Version number: " << test->getVersion() << endl
         << "Vamp plugin: " << test->getVampPlugin() << endl;

    // open .wav file
    SNDFILE *sndfile;
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(SF_INFO));
    sndfile = sf_open(wavfile, SFM_READ, &sfinfo);
    if (!sndfile) {
      cerr << ": ERROR: Failed to open input file \""
           << wavfile << "\": " << sf_strerror(sndfile) << endl;
      destroy_plugin(test);
      dlclose(handle);
      return 1;
    }

    // find and parse vamp plugin name
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

    // process audio file
    vector<Plugin::FeatureSet> results;
    if (host.run(results)) {
      cerr << "ERROR: Vamp plugin could not process audio." << endl;
      destroy_plugin(test);
      dlclose(handle);
      return 1;
    }

    // set up memory for bitmap
    int width=800;
    int height=300;
    unsigned char buffer[width*height*BYTES_PER_PIXEL];

    // get bitmap from library and save as PNG
    if (test->ARGB(results, width, height, buffer)) {
      cerr << "ERROR: Plugin failed to produce bitmap." << endl;
      destroy_plugin(test);
      dlclose(handle);
      return 1;
    }
    if (writePNG(pngfile, width, height, buffer)) {
      cerr << "ERROR: Failed to write PNG." << endl;
      destroy_plugin(test);
      dlclose(handle);
      return 1;
    }

    // clean up
    destroy_plugin(test);
    dlclose(handle);
}
