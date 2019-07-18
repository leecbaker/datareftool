#include "clipboard.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

//implementation from @jason-watkins
std::string getClipboard() {
	std::string result;

	Display* display = XOpenDisplay(nullptr);
	if(display == nullptr) {
		return result;
	}
	// Need a window to pass into the conversion request.
	int black = BlackPixel (display, DefaultScreen (display));
	Window root = XDefaultRootWindow (display);
	Window window = XCreateSimpleWindow (display, root, 0, 0, 1, 1, 0, black, black);
	// Get/create the CLIPBOARD atom.
	// TODO(jason-watkins): Should we really ever create this atom? My limited knowledge of Linux is failing me here.
	Atom clipboard = XInternAtom (display, "CLIPBOARD", False);
	// Get/create an atom to represent datareftool
	Atom prop = XInternAtom(display, "DRT_DATA", False);

	// Request that the clipboard value be converted to a string
	XConvertSelection(display, clipboard, XA_STRING, prop, window, CurrentTime);
	XSync (display, False);

	// Spin several times waiting for the conversion to trigger a SelectionNotify
	// event.
	Bool keep_waiting = True;
	for(int i = 0; keep_waiting && i < 200; ++i)
	{
		XEvent event;
		XNextEvent(display, &event);
		switch (event.type) {
			case SelectionNotify:
				if (event.xselection.selection != clipboard) {
					break;
				}
				if (event.xselection.property == None) {
					keep_waiting = False;
				}
				else {
					int format;
					Atom target;
					unsigned char * value;
					unsigned long length;
					unsigned long bytesafter;
					XGetWindowProperty (event.xselection.display,
						event.xselection.requestor,
						event.xselection.property, 0L, 1000000,
						False, (Atom)AnyPropertyType, &target,
						&format, &length, &bytesafter, &value);
					result = (char *)value;
					XFree(value);
					keep_waiting = False;
					XDeleteProperty (event.xselection.display,
						event.xselection.requestor,
						event.xselection.property);
				}
				break;
			default:
				break;
		}
	}
	XCloseDisplay(display);
	return result;
}

// implementation from @sparker256
void setClipboard(const std::string & s) {
	std::string command = "echo -n " + s + " | xclip -sel c";
	if(0 != system(command.c_str())) {
		std::cerr << "Copy command failed. Do you have xclip on your system?" << std::endl;
	}
}
