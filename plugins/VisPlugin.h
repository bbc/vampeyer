#ifndef VISPLUGIN_HPP
#define VISPLUGIN_HPP

#include "../VampHost.h"
#include <string>
#include <vector>

class VisPlugin
{
  protected:
    int width;
    int height;

  public:
    VisPlugin() {}

    virtual ~VisPlugin() {}

    virtual int ARGB(Plugin::FeatureSet features,
                     int width,
                     int height,
                     unsigned char *bitmap)
    {
      return -1;
    }

    virtual std::string getVampPlugin() {
      return "";
    }

    virtual double getVersion() const = 0;
};

// the types of the class factories
typedef VisPlugin* create_t();
typedef void destroy_t(VisPlugin*);

#endif
