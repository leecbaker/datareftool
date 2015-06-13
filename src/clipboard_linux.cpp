#include "clipboard.h"
#include "XPLMUtilities.h"

#include <string>
#include <fstream>
#include <streambuf>

#include <iostream>
#include <cstdio>

// implementation from @sparker256
std::string getClipboard() {

    FILE *res = popen("xclip -o -sel", "r");
    if (!res) {
        XPLMDebugString("DRT: Paste command failed. Do you have xclip on your system?\n");
    }
    char buffer[1024];
    std::string result;

    while(fgets(buffer, sizeof(buffer), res)){
        result += buffer;
    }
    pclose(res);
    return result;
}

// implementation from @sparker256
void setClipboard(const std::string & s) {
    std::string command = "echo -n " + s + " | xclip -sel c";
	if(0 != system(command.c_str())) {
        XPLMDebugString("DRT: Copy command failed. Do you have xclip on your system?\n");
	}
}
