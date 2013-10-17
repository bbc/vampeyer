#include "plugins/VisPlugin.h"
#include <iostream>
#include <dlfcn.h>

int main() {
    using std::cout;
    using std::cerr;

    // load the test library
    void* handle = dlopen("./plugins/TestPlugin.so", RTLD_LAZY);
    if (!handle) {
        cerr << "Cannot load library: " << dlerror() << '\n';
        return 1;
    }

    // reset errors
    dlerror();
    
    // load the symbols
    create_t* create_plugin = (create_t*) dlsym(handle, "create");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol create: " << dlsym_error << '\n';
        return 1;
    }
    
    destroy_t* destroy_plugin = (destroy_t*) dlsym(handle, "destroy");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol destroy: " << dlsym_error << '\n';
        return 1;
    }

    // create an instance of the class
    VisPlugin* test = create_plugin();

    // use the class
    cout << "Version number: " << test->getVersion() << '\n';

    // destroy the class
    destroy_plugin(test);

    // unload the test library
    dlclose(handle);
}
