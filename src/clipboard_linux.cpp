#include "clipboard.h"
#include "XPLMUtilities.h"

std::string getClipboard() {
	//TODO
	return "";
}

void setClipboard(const std::string & s) {
	std::string command = "echo " + s + " | xclip -sel c";
	if(0 != system(command.c_str())) {
		XPLMDebugString("Copy command failed. Do you have xclip on your system?");
	}
}
