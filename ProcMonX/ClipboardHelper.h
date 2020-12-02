#pragma once

class ClipboardHelper final {
public:
	static bool CopyText(HWND hWnd, const CString& text);
};

