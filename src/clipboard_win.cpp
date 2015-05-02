#include <Windows.h>

#include "clipboard.h"


std::string getClipboard() {
	std::string ret;

	if (TRUE == OpenClipboard(nullptr)) {
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (nullptr != hData) {
			char * pszText = static_cast<char*>(GlobalLock(hData));
			if (nullptr != pszText) {
				ret.assign(pszText);
			}

			GlobalUnlock(hData);
		}

		CloseClipboard();
	}

	return ret;
}

void setClipboard(const std::string & s) {
	if (TRUE == OpenClipboard(nullptr)) {
		EmptyClipboard();

		HGLOBAL hClipboardData = GlobalAlloc(GMEM_FIXED, s.size() + 1);
		if (nullptr != hClipboardData) {

			char * pchData = (char*)GlobalLock(hClipboardData);
			if (nullptr != pchData) {
				strcpy(pchData, LPCSTR(s.c_str()));

				GlobalUnlock(hClipboardData);

				SetClipboardData(CF_TEXT, hClipboardData);

			}
		}

		CloseClipboard();
	}
}
