#pragma once

struct Settings {
	WINDOWPLACEMENT WindowPlacement{ sizeof(WindowPlacement) };
	LOGFONT Font{};
	bool AlwaysOnTop{ false };

	bool Save(PCWSTR path);
	bool Load(PCWSTR path);
};
