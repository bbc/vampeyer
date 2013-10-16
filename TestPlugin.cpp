#include "VisPlugin.h"
#include <cmath>

class TestPlugin : public VisPlugin {
public:
    virtual double getVersion() const {
        return 0.1;
    }
};


// the class factories

extern "C" VisPlugin* create() {
    return new TestPlugin;
}

extern "C" void destroy(VisPlugin* p) {
    delete p;
}
