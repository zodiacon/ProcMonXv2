#include "pch.h"
#include "ClipboardHelper.h"


bool ClipboardHelper::CopyText(HWND hWnd, const CString& text) {
	if (::OpenClipboard(hWnd)) {
		::EmptyClipboard();
		auto size = (text.GetLength() + 1) * sizeof(WCHAR);
		auto hData = ::GlobalAlloc(GMEM_MOVEABLE, size);
		if (hData) {
			auto p = ::GlobalLock(hData);
			if (p) {
				::memcpy(p, text, size);
				::GlobalUnlock(p);
				::SetClipboardData(CF_UNICODETEXT, hData);
			}
		}
		::CloseClipboard();
		if (hData)
			return true;
	}
	return false;
}
