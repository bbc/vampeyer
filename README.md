# Vamp<font color="red">eye</font>r
A framework for creating plugins which visualise the output of
[Vamp](http://vamp-plugins.org/) audio analysis plugins as a bitmap image.

## Building the host

    sudo apt-get install libpng12-dev libsndfile1-dev vamp-plugin-sdk libvamp-hostsdk3 libfltk1.3-dev libtclap-dev
    make

## Building the plugins

    sudo apt-get install libcairo2-dec
    cd plugins
    make
