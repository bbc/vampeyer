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

#include "VampHost.h"

VampHost::VampHost(SNDFILE *sndfile_in,
             SF_INFO sfinfo,
             string soname,         // example: qm-vamp-plugins:qm-mfcc
             int blockSize_in,
             int stepSize_in)
{
  useFrames = false;
  sndfile = sndfile_in;
  sampleRate = sfinfo.samplerate;
  channels = sfinfo.channels;
  frames = sfinfo.frames;

  // parse plugin name
  string plugid = "";
  string::size_type sep = soname.find(':');
  if (sep != string::npos) {
    plugid = soname.substr(sep + 1);
    soname = soname.substr(0, sep);
  }

  // if no plugin name is found, throw error
  if (plugid == "") {
    cerr << "Plugin not found!" << endl;
  }

  // initiate plugin loading
  PluginLoader *loader = PluginLoader::getInstance();

  // get key of selected plugin
  PluginLoader::PluginKey key = loader->composePluginKey(soname, plugid);

  // load plugin with sample rate of .wav file
  plugin = loader->loadPlugin
      (key, sampleRate, PluginLoader::ADAPT_ALL_SAFE);

  // if plugin failed to load, throw error
  if (!plugin) {
    cerr << ": ERROR: Failed to load plugin \"" << plugid
         << "\" from library \"" << soname << "\"" << endl;
  }

  // set up block size
  blockSize = blockSize_in;
  if (blockSize == 0) {
    blockSize = plugin->getPreferredBlockSize();
  }
  if (blockSize == 0) {
    blockSize = 1024;
  }

  // set up step size
  stepSize = stepSize_in;
  if (stepSize == 0) {
    stepSize = plugin->getPreferredStepSize();
  }

  // if no preference given, set step size as half block
  // size if freq analysis, otherwise block size
  if (stepSize == 0) {
    if (plugin->getInputDomain() == Plugin::FrequencyDomain) {
      stepSize = blockSize/2;
    } else {
      stepSize = blockSize;
    }
  }

  // check step size is smaller or equal to block size
  if (stepSize > blockSize) {
    cerr << "WARNING: stepSize " << stepSize
         << " > blockSize " << blockSize << ", resetting blockSize to ";
    if (plugin->getInputDomain() == Plugin::FrequencyDomain) {
      blockSize = stepSize * 2;
    } else {
      blockSize = stepSize;
    }
    cerr << blockSize << endl;
  }

  // create buffers for reading blocks
  filebuf = new float[blockSize * channels];
  plugbuf = new float*[channels];
  for (int c = 0; c < channels; ++c) plugbuf[c] = new float[blockSize + 2];

  // get list of outputs
  Plugin::OutputList outputs = plugin->getOutputDescriptors();

  // check for no outputs
  if (outputs.empty()) {
    cerr << "ERROR: Plugin has no outputs!" << endl;
  }
}

VampHost::~VampHost()
{
  delete[] filebuf;
  for (int c = 0; c < channels; ++c) delete[] plugbuf[c];
  delete[] plugbuf;
  delete plugin;
}

int VampHost::findOutputNumber(string outputName)
{
  // get list of outputs
  Plugin::OutputList outputs = plugin->getOutputDescriptors();

  // return position of output name in list
  for (size_t oi = 0; oi < outputs.size(); ++oi) {
    if (outputs[oi].identifier == outputName) {
      return oi;
    }
  }

  // if not found, return -1
  cerr << "ERROR: Non-existent output \"" << outputName
    << "\" requested" << endl;
  return -1;
}

int VampHost::run(Plugin::FeatureSet& results)
{
    int overlapSize = blockSize - stepSize;
    sf_count_t currentStep = 0;
    // at end of file, this many part-silent frames needed after we hit EOF
    int finalStepsRemaining = max(1, (blockSize / stepSize) - 1);

    RealTime rt;
    PluginWrapper *wrapper = 0;
    RealTime adjustment = RealTime::zeroTime;

    // Note that the following would be much simpler if we used a
    // PluginBufferingAdapter as well -- i.e. if we had passed
    // PluginLoader::ADAPT_ALL to loader->loadPlugin() above, instead
    // of ADAPT_ALL_SAFE.  Then we could simply specify our own block
    // size, keep the step size equal to the block size, and ignore
    // the plugin's bleatings.  However, there are some issues with
    // using a PluginBufferingAdapter that make the results sometimes
    // technically different from (if effectively the same as) the
    // un-adapted plugin, so we aren't doing that here.  See the
    // PluginBufferingAdapter documentation for details.

    // initialise plugin
    if (!plugin->initialise(channels, stepSize, blockSize)) {
        cerr << "Plugin initialise (channels = " << channels
             << ", stepSize = " << stepSize << ", blockSize = "
             << blockSize << ") failed." << endl;
        return 1;
    }

    // find timestamp adjustment
    wrapper = dynamic_cast<PluginWrapper *>(plugin);
    if (wrapper) {
        // See documentation for
        // PluginInputDomainAdapter::getTimestampAdjustment
        PluginInputDomainAdapter *ida =
            wrapper->getWrapper<PluginInputDomainAdapter>();
        if (ida) adjustment = ida->getTimestampAdjustment();
    }
    
    // Here we iterate over the frames, avoiding asking the numframes
    // in case it's streaming input.
    do {

        int count;

        // if block size matches step size, just read a full block
        if ((blockSize==stepSize) || (currentStep==0)) {
            if ((count = sf_readf_float(sndfile, filebuf, blockSize)) < 0) {
                cerr << "sf_readf_float failed: " << sf_strerror(sndfile)
                  << endl;
                break;
            }
            if (count != blockSize) --finalStepsRemaining;

        // if different, shunt exiting data and read the remainder
        } else {
            memmove(filebuf, filebuf + (stepSize * channels),
                    overlapSize * channels * sizeof(float));
            if ((count = sf_readf_float(sndfile, filebuf + (overlapSize *
                      channels), stepSize)) < 0) {
                cerr << "sf_readf_float failed: " << sf_strerror(sndfile)
                  << endl;
                break;
            }
            if (count != stepSize) --finalStepsRemaining;
            count += overlapSize;
        }

        // copy data to plugin buffer
        for (int c = 0; c < channels; ++c) {
            int j = 0;
            while (j < count) {
                plugbuf[c][j] = filebuf[j * channels + c];
                ++j;
            }
            while (j < blockSize) {
                plugbuf[c][j] = 0.0f;
                ++j;
            }
        }

        // show results
        rt = RealTime::frame2RealTime(currentStep * stepSize, sampleRate);
        Plugin::FeatureSet tmpResults = plugin->process(plugbuf, rt);
        for(Plugin::FeatureSet::iterator it = tmpResults.begin();
            it != tmpResults.end(); ++it)
        {
          int key = it->first;
          Plugin::FeatureList feats = it->second;
          for (unsigned int i=0; i<feats.size(); i++)
            results[key].push_back(feats[i]);
        }

        // count the steps
        ++currentStep;

    } while (finalStepsRemaining > 0);

    // show remaining results
    rt = RealTime::frame2RealTime(currentStep * stepSize, sampleRate);
    Plugin::FeatureSet tmpResults = plugin->getRemainingFeatures();
    for(Plugin::FeatureSet::iterator it = tmpResults.begin();
        it != tmpResults.end(); ++it)
    {
      int key = it->first;
      Plugin::FeatureList feats = it->second;
      for (unsigned int i=0; i<feats.size(); i++)
        results[key].push_back(feats[i]);
    }

    return 0;
}

int VampHost::getBlockSize()
{
  return blockSize;
}

int VampHost::getStepSize()
{
  return stepSize;
}

void VampHost::setParameter(string name, float value)
{
  plugin->setParameter(name, value);
}
