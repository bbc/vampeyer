/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Vamp

    An API for audio analysis and feature extraction plugins.

    Centre for Digital Music, Queen Mary, University of London.
    Copyright 2006 Chris Cannam, copyright 2007-2008 QMUL.
  
    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the names of the Centre for
    Digital Music; Queen Mary, University of London; and Chris Cannam
    shall not be used in advertising or otherwise to promote the sale,
    use or other dealings in this Software without prior written
    authorization.
*/


/*
 * This "simple" Vamp plugin host is no longer as simple as it was; it
 * now has a lot of options and includes a lot of code to handle the
 * various useful listing modes it supports.
 *
 * However, the runPlugin function still contains a reasonable
 * implementation of a fairly generic Vamp plugin host capable of
 * evaluating a given output on a given plugin for a sound file read
 * via libsndfile.
 */

#include <vamp-hostsdk/PluginHostAdapter.h>
#include <vamp-hostsdk/PluginInputDomainAdapter.h>
#include <vamp-hostsdk/PluginLoader.h>

#include <iostream>
#include <fstream>
#include <set>
#include <sndfile.h>
#include <vector>

#include <cstring>
#include <cstdlib>

#include "system.h"

#include <cmath>

using namespace std;

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::RealTime;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;

#define HOST_VERSION "1.5"

class VampHost
{
  protected:
    Plugin *plugin;
    SNDFILE *sndfile;
    sf_count_t frames;
    int blockSize;
    int stepSize;
    int sampleRate;
    int channels;
    int outputNo;
    bool useFrames;
    float *filebuf;
    float **plugbuf;

  public:
    VampHost(SNDFILE *sndfile,
             SF_INFO sfinfo,
             string soname,         // example: qm-vamp-plugins:qm-mfcc
             int blockSize=0,
             int stepSize=0);
    ~VampHost();
    int run(vector<Plugin::FeatureSet>& results);
    int findOutputNumber(string outputName);
    int getBlockSize();
    int getStepSize();
};

