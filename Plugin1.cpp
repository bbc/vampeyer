#include "stdio.h"
#include "VisPlugin.h"

class Plugin1 : public VisPlugin
{
  public:
    const char* getName() const
    {
      return "Plugin1";
    }
    virtual void process()
    {
      for (int i=0; i<3; i++)
      {
        printf("Plugin 1 is processing...\n");
      }
      printf("Plugin 1 processing done.\n");
    }
};

extern "C"
{
  __attribute__ ((visibility ("default"))) VisPlugin* createPlugin()
  {
    return new Plugin1();
  }
  __attribute__ ((visibility ("default"))) void releasePlugin(VisPlugin* p_plugin)
  {
    delete p_plugin;
  }
}
