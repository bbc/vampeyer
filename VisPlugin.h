#ifndef VISPLUGIN_HPP
#define VISPLUGIN_HPP

class VisPlugin
{
  public:
    VisPlugin() {}

    virtual ~VisPlugin() {}

    virtual double getVersion() const = 0;
};

// the types of the class factories
typedef VisPlugin* create_t();
typedef void destroy_t(VisPlugin*);

#endif
