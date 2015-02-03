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

* Case insensitive search
* Regex search
* Windows, Mac, Linux are all supported
* Filter only recently changed datarefs, and color datarefs that recently changed
* Displays all data types
* Display multiple elements of an array at once
* Make it easy to deselect the search field (unlike DRE). Use enter, return, escape, or tab, or click elsewhere in the DRT window.
* Multiple DRT windows viewable at once

Stuff that I'm not planning (but welcome pull requests for):

* Ability to modify datarefs
* Remember window positions and search details when X-Plane is closed

### How to build
Building requires cmake and a c++11 compiler. Here's how I do it:

    mkdir build
    cd build && cmake ..
    make

#### Building on Windows
On Windows, you can use the Visual Studio project generator to do something like this:

    mkdir build
    cd build && cmake -G "Visual Studio 12 Win64" -DMSVC_RUNTIME=dynamic ..

at which point you can just use Visual Studio to build the project, or you can build on the command line:

    msbuild src/plugin.vcxproj /p:Configuration=Release /p:PlatformToolset=CTP_Nov2013 /flp:logfile=plugin_build.log;verbosity=normal

Note that you'll need the Nov 2013 CTP compiler or Visual Studio 14 preview to be able to build the plugin, since it uses some C++11 features. Both are a free download from Microsoft. You'll see a ton of warnings (6890 warnings on my machine!), but I couldn't find any that really had much relevance.

### Adding custom datarefs
You can use DRT to display your plugin's custom datarefs. Just send a message of type 0x01000000 with a pointer to the name of the dataref as the payload. There is an example of how to do this in [plugin_custom_dataref.cpp](plugin_custom_dataref.cpp). (This is exactly the same method that you use to add a custom dataref to Data Ref Editor.)

### Author
DRT is written by Lee C. Baker. If you benefitted from this plugin, please consider purchasing the <a href="https://planecommand.com">PlaneCommand voice recognition plugin.

&copy; 2015 Lee C. Baker.