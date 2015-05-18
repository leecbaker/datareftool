# Data Ref Tool for X-Plane plugin development

DRT is an X-plane plugin intended to replacement for Sandy Barbour's Data Ref Editor (DRE). While the core functionality is much the same, many features are improved to make the experience nicer:

* Case insensitive search option
* Regex search option
* Only show datarefs that have changed recently
* Easier to deselect the search field
* Better display of array data

DRT is a work in progress; code contributions are welcome.

![Screenshot of data ref tool](doc/datareftool.png)

###License
DRT is available under the MIT license. See the LICENSE file for more details.

### Feature list

* Read, write, and search for datarefs
* Case insensitive search
* Regex search
* Windows, Mac, Linux are all supported
* Filter only recently changed datarefs, and color datarefs that recently changed
* Displays all data types
* Display multiple elements of an array at once
* Make it easy to deselect the search field (unlike DRE). Use enter, return, escape, or tab, or click elsewhere in the DRT window.
* Cut/copy/paste/select all in search field and for datarefs (uses standard keyboard shortcuts)
* Multiple DRT windows viewable at once

### How to build
Building requires cmake and a c++11 compiler. Here's how I do it:

    mkdir build
    cd build && cmake ..
    make

#### Building on Windows
Boost is now required, which complicates the build a bit. If you don't have the required paths set up I'd recommend looking at my [personal build script](build_win.bat).

On Windows, you can use the Visual Studio project generator to do something like this:

    mkdir build
    cd build && cmake -G "Visual Studio 12 Win64" -DMSVC_RUNTIME=dynamic ..

at which point you can just use Visual Studio to build the project, or you can build on the command line:

    msbuild src/plugin.vcxproj /p:Configuration=Release /flp:logfile=plugin_build.log;verbosity=normal

Note: You no longer need the VC++ CTP compiler to build DRT.

### Adding custom datarefs
You can use DRT to display your plugin's custom datarefs. Just send a message of type 0x01000000 with a pointer to the name of the dataref as the payload. There is an example of how to do this in [plugin_custom_dataref.cpp](src/plugin_custom_dataref.cpp). (This is exactly the same method that you use to add a custom dataref to Data Ref Editor.)

### Author
DRT is written by Lee C. Baker. If you benefitted from this plugin, please consider purchasing the <a href="https://planecommand.com">PlaneCommand voice recognition plugin.

&copy; 2015 Lee C. Baker.