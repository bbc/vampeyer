#include <dlfcn.h>
#include <iostream>
#include "VisPlugin.h"

//typedef VisPlugin* (*PLUGIN_FACTORY)();
//typedef void (*PLUGIN_CLEANUP)(VisPlugin*);

int main (int argc, char* argv[])
{
  void* handle = dlopen("plugin1.so", RTLD_LAZY);

  VisPlugin* (*create)();
  void (*destroy)(VisPlugin*);

  create = (VisPlugin* (*)())dlsym(handle, "create_object");
  destroy = (void (*)(VisPlugin*))dlsym(handle, "destroy_object");

  VisPlugin* myPlugin = (VisPlugin*)create();
  myPlugin->process();
  destroy(myPlugin);

  return 0;
}
