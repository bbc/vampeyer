#ifndef VISPLUGIN_HPP
#define VISPLUGIN_HPP

class VisPlugin
{
  protected:
    int width;
    int height;

  public:
    VisPlugin() {}

    virtual ~VisPlugin() {}

    virtual int ARGB(int width, int height, unsigned char *bitmap)
    {
      return -1;
    }

    virtual double getVersion() const = 0;
};

// the types of the class factories
typedef VisPlugin* create_t();
typedef void destroy_t(VisPlugin*);

#endif
