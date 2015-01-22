# Data Ref Tool for X-Plane plugin development

DRT is an X-plane plugin intended to replacement for Sandy Barbour's Data Ref Editor (DRE). While the core functionality is much the same, there are some features to make the experience nicer:

* Case insensitive search
* Regex search
* Only show datarefs that have changed recently
* Easier to deselect the search field

DRT is a work in progress; code contributions are welcome.

![Screenshot of data ref tool](doc/datareftool.png)

###License
DRT is available under the MIT license. See the LICENSE file for more details.

### Feature list
These are the features that I'm planning on completing:

    [X] Case insensitive search
    [X] Regex search
    [X] See only recently changed datarefs
    [X] Color datarefs if recently changed
    [X] Make it easy to deselect the search field (unlike DRE). Use enter, return, escape, or tab, or click elsewhere in the DRT window.
    [X] Multiple DRT windows viewable at once
    [X] Display basic data types (float double int)
    [X] Display other data types (arrays, byte arrays)
    [X] Display multiple elements of an array at once
    [?] Windows support 

Stuff that I'm not planning (but welcome pull requests for):

    [ ] Ability to modify datarefs
    [ ] Remember window positions and search details when X-Plane is closed
    [ ] Display multiple elements of an array at once
    [ ] Linux support

### How to build
Building requires cmake and a c++11 compiler. Here's how I do it:

    mkdir build
    cd build && cmake ..
    make

### Author
DRT is written by Lee C. Baker. If you benefitted from this plugin, please consider purchasing the <a href="https://planecommand.com">PlaneCommand voice recognition plugin.

&copy; 2015 Lee C. Baker.