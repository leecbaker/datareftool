#include "clipboard.h"

#import <Appkit/Appkit.h>


std::string getClipboard() {
	NSString * contents = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
	const char * utf8 = [contents UTF8String];
	return std::string(utf8);
}

void setClipboard(const std::string & s) {
	NSString * nss = [NSString stringWithCString:s.c_str() encoding:[NSString defaultCStringEncoding]];
	[[NSPasteboard generalPasteboard] clearContents];
	[[NSPasteboard generalPasteboard] setString:nss forType:NSPasteboardTypeString];
}
