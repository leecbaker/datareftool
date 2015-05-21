#include "clipboard.h"
#include "XPLMUtilities.h"

#include <string>
#include <fstream>
#include <streambuf>

// implementation from @sparker256
std::string getClipboard() {

    std::string command = "xclip -o -sel > ./Resources/plugins/DataRefTool/linxclip.txt";

    if(0 != system(command.c_str())) {
        XPLMDebugString("DRT: Paste command failed. Do you have xclip on your system?\n");
    }

    // TODO @sparker256
    // Find a way to get the stdout from "xclip -o -sel" into std::string result

    // This is a hack using a txt file to store selection and read it back into result
    // When I get the stdout to std::string result working I will use that.

    std::ifstream t("./Resources/plugins/DataRefTool/linxclip.txt");
    std::string result;

    t.seekg(0, std::ios::end);
    result.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    result.assign((std::istreambuf_iterator<char>(t)),
                std::istreambuf_iterator<char>());

    return result;
}

// implementation from @sparker256
void setClipboard(const std::string & s) {
	std::string command = "echo " + s + " | xclip -sel c";
	if(0 != system(command.c_str())) {
        XPLMDebugString("DRT: Copy command failed. Do you have xclip on your system?\n");
	}
}
