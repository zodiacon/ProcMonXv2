#pragma once

constexpr WCHAR SaveLoadIniFilter[] = L"ini files (*.ini)\0*.ini\0All Files\0*.*\0";

class DialogHelper abstract {
public:
	static void AdjustOKCancelButtons(CWindow* dlg);
	static bool AddIconToButton(CWindow* dlg, WORD id, WORD icon);
	static void SetDialogIcon(CWindow* dlg, UINT icon);
};

