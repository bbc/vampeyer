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
#ifndef VISPLUGIN_HPP
#define VISPLUGIN_HPP

#include "vamp-hostsdk/Plugin.h"
#include <cstring>
#include <vector>

using Vamp::Plugin;
using Vamp::RealTime;

class VisPlugin
{
  protected:
    int width;
    int height;

  public:

    typedef struct _VampParameter
    {
      std::string name;
      float value;

      bool operator<(const _VampParameter &n) const { 
        if (this->name < n.name) return true;
        if (this->name > n.name) return false;
        return this->value < n.value;
      }
    } VampParameter;

    typedef std::vector<VampParameter> VampParameterList;

    typedef struct _VampPlugin
    {
      const char *name;
      int blockSize;
      int stepSize;
      VampParameterList parameters;

      bool operator<( const _VampPlugin &n ) const {
        if (this->name < n.name) return true;
        if (this->name > n.name) return false;
        if (this->blockSize < n.blockSize) return true;
        if (this->blockSize > n.blockSize) return false;
        if (this->stepSize < n.stepSize) return true;
        if (this->stepSize > n.stepSize) return false;
        return this->parameters < n.parameters;
      }
    } VampPlugin;

    typedef struct _VampOutput
    {
      VampPlugin plugin;
      const char *name;
    } VampOutput;

    typedef std::vector<VampOutput> VampOutputList;

    VisPlugin() {}

    virtual ~VisPlugin() {}

    virtual int ARGB(Plugin::FeatureSet features,
                     int width,
                     int height,
                     unsigned char *bitmap,
                     int sampleRate)
    {
      return -1;
    }

    virtual VampOutputList getVampPlugins() {
      VampOutputList out;
      return out;
    }

    virtual double getVersion() const = 0;
};

// the types of the class factories
typedef VisPlugin* create_t();
typedef void destroy_t(VisPlugin*);

#endif
