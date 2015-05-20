#include "clipboard.h"

std::string getClipboard() {
	//TODO
	return "";
}

void setClipboard(const std::string & s) {
    // Use xclip
    // TODO(sparker256): Need to check if xclip is on system
    // If not give method to install it
    // sudo apt-get install xclip
    std::string s1 = "echo ";
    std::string s2 = " | xclip -sel c";
    std::string s3 = s1+ s +s2;
    system(s3.c_str());

}
