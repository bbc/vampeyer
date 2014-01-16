#ifndef VISHOST_H
#define VISHOST_H

#include "plugins/VisPlugin.h"
#include "VampHost.h"
#include <dlfcn.h>
#include <string>

class VisHost
{
  protected:
    void* handle;
    destroy_t* destroy_plugin;
    VisPlugin* visPlugin;
    string pluginPath;
    SNDFILE *sndfile;
    SF_INFO sfinfo;
    Plugin::FeatureSet resultsFilt;
    map<VisPlugin::VampPlugin, vector<Plugin::FeatureSet> > vampResults;
    map<VisPlugin::VampPlugin, VampHost*> vampHosts;
    set<VisPlugin::VampPlugin> vampPlugins;

  public:
    VisHost(string);
    int init();
    int process(string);
    int render(int width, int height, unsigned char*);
    ~VisHost();
    bool verbose;
};

#endif
