#include "VisHost.h"

VisHost::VisHost(string pluginPath_in)
{
  pluginPath = pluginPath_in;
  verbose=false;
}

int VisHost::init()
{
  // load the visualization library
  if (verbose) cout << " * Loading visualization plugin..." << flush;
  handle = dlopen(pluginPath.c_str(), RTLD_LAZY);
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
  destroy_plugin = (destroy_t*) dlsym(handle, "destroy");
  dlsym_error = dlerror();
  if (dlsym_error) {
    cerr << "ERROR: Cannot load symbol destroy: " << dlsym_error << endl;
    dlclose(handle);
    return 1;
  }

  // create an instance of the class
  visPlugin = create_plugin();
  if (verbose) cout << " [done]" << endl;

  return 0;
}

int VisHost::process(string wavfile)
{
  // open the .wav file
  memset(&sfinfo, 0, sizeof(SF_INFO));
  sndfile = sf_open(wavfile.c_str(), SFM_READ, &sfinfo);
  if (!sndfile) {
    cerr << ": ERROR: Failed to open input file \""
      << wavfile << "\": " << sf_strerror(sndfile) << endl;
    return 1;
  }
  sampleRate = sfinfo.samplerate;

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
    VisPlugin::VampPlugin plugin = *p;
    if (verbose) cout << " * Processing Vamp plugin " << plugin.name << "..."
      << flush;

    // move to beginning of .wav file
    sf_seek(sndfile, 0, SEEK_SET);

    // initialise the plugin
    vampHosts[plugin] = new VampHost(sndfile,
                                     sfinfo,
                                     plugin.name,
                                     plugin.blockSize,
                                     plugin.stepSize);

    // set the parameters
    for (VisPlugin::VampParameterList::iterator r=plugin.parameters.begin();
        r!=plugin.parameters.end(); r++)
    {
      VisPlugin::VampParameter param = *r;
      vampHosts[plugin]->setParameter(param.name, param.value);
    }

    // process audio file
    if (vampHosts[plugin]->run(vampResults[plugin])) {
      cerr << "ERROR: Vamp plugin " << plugin.name
        << " could not process audio." << endl;
      return 1;
    }
    if (verbose) cout << " [done]" << endl;
  }

  int count=0;
  for (VisPlugin::VampOutputList::iterator o=vampOuts.begin();
       o!=vampOuts.end(); o++)
  {
    VisPlugin::VampOutput out = *o;
    if (verbose) cout << " * Refactoring data for "
      << out.plugin.name << ":" << out.name << "..." << flush;
    unsigned int outNum = vampHosts[out.plugin]->findOutputNumber(out.name);
    Plugin::FeatureList featList = vampResults[out.plugin][outNum];
    resultsFilt[count] = featList;
    count++;
    if (verbose) cout << " [done]" << endl;
  }

  return 0;
}

int VisHost::render(int width, int height, unsigned char *buffer)
{

  // set up memory for bitmap
  if (verbose) cout << " * Processing visualization..." << flush;

  // get bitmap from library
  if (visPlugin->ARGB(resultsFilt, width, height, buffer, sampleRate)) {
    cerr << "ERROR: Plugin failed to produce bitmap." << endl;
    return 1;
  }
  if (verbose) cout << " [done]" << endl;

  return 0;
}

VisHost::~VisHost()
{
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
