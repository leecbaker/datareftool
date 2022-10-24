# DataRefTool has moved to DataRefTool.com

All future activity will be at [DataRefTool.com](https://datareftool.com).

This github repo contains DRT version 1, which is compatible with X-Plane 10 and 11, and remains only as an archive. No further development will be tracked here, and I won't be reading any new Issues or Pull Requests.

---

### Data Ref Tool for X-Plane plugin development

**[Download from GitHub](https://github.com/leecbaker/datareftool/releases)**

[![Compile status](https://github.com/leecbaker/datareftool/workflows/Compile/badge.svg)](https://github.com/leecbaker/datareftool/actions)

DRT is an X-plane plugin intended to replacement for Sandy Barbour's Data Ref Editor (DRE). While the core functionality is much the same, many features are improved to make the experience nicer:

* Case insensitive search option
* Regex search option
* Only show datarefs that have changed recently
* Easier to deselect the search field
* Better display of array data

DRT is a work in progress; code contributions are welcome.

![Screenshot of data ref tool](doc/datareftool.png)

## Feature list

* Read, write, and search for datarefs
* Search for and execute commands
* Case insensitive search
* Regex search
* Supports Windows/Linux/Mac on X-Plane 10 and 11
* Filter only recently changed datarefs, and color datarefs that recently changed
* Displays all data types
* Display multiple elements of an array at once
* Make it easy to deselect the search field (unlike DRE). Use enter, return, escape, or tab, or click elsewhere in the DRT window.
* Cut/copy/paste/select all in search field and for datarefs (uses standard keyboard shortcuts)
* Multiple DRT windows viewable at once

## How to install

To install the plugin, download the latest release [on this page](https://github.com/leecbaker/datareftool/releases). Unzip the files, and copy the `DataRefTool_XP11_2020_12_06` to your plugins directory `X-Plane 11/Resources/plugins/`. Make sure to choose the XP11 version for X-Plane 11, or XP10 for X-Plane 10.

## Frequently Asked Questions

### FAQ: What are the keyboard shortcuts?

Glad you asked!

In general, normal text editing shortcuts are respected:

* Ctrl-X / &#8984;-X : cut
* Ctrl-C / &#8984;-C : copy
* Ctrl-V / &#8984;-V : paste
* Ctrl-A / &#8984;-A : select all

In the search window, there are some additional things you can do:

* Ctrl-N / &#8984;-N : open a new search window
* Ctrl-W / &#8984;-W : close current window
* Tab / shift-tab : switch between search field and results

Search field:

* Ctrl-F / &#8984;-F or Ctrl-L / &#8984;-L : go to the search field
* Ctrl-alt-C / &#8997;&#8984;-C : Toggle Change detection
* Ctrl-alt-I / &#8997;&#8984;-I : Toggle case-Insensitive search
* Ctrl-alt-R / &#8997;&#8984;-R : Toggle Regex search
* Ctrl-alt-S / &#8997;&#8984;-S : Toggle Source (dataref/command/all)

Search results:

* Enter/return : open details window for the current dataref/command
* Ctrl-C / &#8984;-C: Copy currently-selected dataref name
* Ctrl-alt-C / &#8984;&#8997;-C : Copy currently-selected dataref value
* Type a number : Set new value for currently-selected scalar dataref (int/float/double only)
* Space: activate current command
* J/K or up/down arrow : Go up or down in the list (like vim)

If you want additional keyboard shortcuts (to open a search window, for instance) these can be added by searching for "datareftool" in X-Plane's keyboard settings.

### FAQ: DRT can't find my dataref

DRT scans files to find datarefs. This might not work if your dataref is in an encrypted Lua file or something, so you have several options:

* If this is an aircraft, add a file called "dataref.txt" inside your aircraft directory with a list of datarefs and commands, one on each line
* Have your plugin send DRT a message with as described above in "Adding custom datarefs"
* Turn on "Impersonate DataRefEditor" on the plugin menu inside X-Plane. Before you do this, ensure that the DataRefEditor plugin is not installed. This way, X-Plane itself will tell DRT about all datarefs it knows about.

### FAQ: Using DRT causes X-Plane to crash

DRT reads every dataref published by every aircraft and plugin, on every frame of the simulation. Sometimes, they haven't fully been debugged, and may crash. (If you're a developer, the best way to do this is to run X-Plane in a debugger and look at the backtrace of the crash- if you see RefRecords::update() in the backtrace, this is likely what happened.)

If you can figure out which dataref caused the crash, the best way to work around this is to add the name of the dataref to a file called `X-Plane 11/Resources/plugins/drt_ignore.txt`. This will cause DRT to never read the value of the dataref, even if it does come up in search results.

### FAQ: Using DRT with the Bell 407 causes X-Plane to crash

If you're flying the Bell 407, there is a known issue with this aircraft where reading some datarefs triggers an immediate crash. To work around this, create a file called `X-Plane 11/Resources/plugins/drt_ignore.txt`, and add the following lines to the file:

```txt
B407/Lights/CL1
B407/Lights/CL2
B407/Lights/CL3
```

The Bell 407 problem is being tracked in issue #33 in this repository.

### FAQ: DataRefTool causes me to loose FPS! Why?

DRT reads datarefs every frame in order to detect changes to values. This takes time; moreover, it takes time for other plugins and for X-Plane to compute the dataref values (which is what actually takes most of the time). It takes CPU time to actually do what DRT does; this is why you're losing FPS.

Here are the best ways to make sure that DRT is having the smallest impact on your FPS:

1. Close all DRT windows when you aren't using them. DRT does stop reading datarefs completely when all windows are closed; you should not lose any FPS at all when all windows are closed.
2. Ignore datarefs from other plugins that are slow to read. You can do this buy creating a file called `X-Plane 11/Resources/Plugins/drt_ignore.txt`, and list the datarefs you want to ignore there. In X-Plane 11, I'd recommend adding these slow datarefs to the ignore file:

    ```text
    sim/airfoils/afl_cd
    sim/airfoils/afl_cm
    sim/airfoils/afl_cl
    ```

    To identify datarefs that are slow to read, use a CPU profiler (I use X-Code Instruments).

3. Buy a faster CPU! :-) DRT is written in a highly optimized and efficient manner; however, it does a lot of work, and that work takes CPU time.

## Development details

### Compiling DataRefTool

The steps for how to build DRT can be seen in the [Github Actions file](.github/workflows/build.yml). You'll need to follow several steps to install all the necessary dependencies using Homebrew (Mac), vcpkg (Windows) or apt-get (Linux).

### Adding custom datarefs

You can use DRT to display your plugin's custom datarefs. Just send a message of type 0x01000000 with a pointer to the name of the dataref as the payload. There is an example of how to do this in [plugin_custom_dataref.cpp](src/plugin_custom_dataref.cpp). (This is exactly the same method that you use to add a custom dataref to Data Ref Editor.)

### Searching DRT from another plugin

DRT provides an API to allow other plugins to search the dataref list. See [doc/API.md](doc/API.md) for details.

## License

DRT is available under the MIT license. See the LICENSE file for more details.

## Author

DRT is written by Lee C. Baker. If you benefitted from this plugin, please consider purchasing the [PlaneCommand](https://planecommand.com) voice recognition plugin.

&copy; 2017-2021 Lee C. Baker.
