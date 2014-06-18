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
#ifndef VISHOST_H
#define VISHOST_H

#include "VisPlugin.h"
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
    int sampleRate;
    Plugin::FeatureSet resultsFilt;
    map<VisPlugin::VampPlugin, Plugin::FeatureSet > vampResults;
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
