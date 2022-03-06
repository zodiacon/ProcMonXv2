#pragma once

#include "SymbolsHandler.h"

class SymbolManager {
public:
	static SymbolManager& Get();
	~SymbolManager();
	void Term();

	SymbolsHandler* GetCommon();
	SymbolsHandler* GetForProcess(DWORD pid);

	std::unique_ptr<SymbolInfo> GetSymbolFromAddress(DWORD pid, DWORD64 address, PDWORD64 offset = nullptr);

private:
	SymbolManager();

	SymbolsHandler _commonSymbols;
	std::unordered_map<DWORD, std::unique_ptr<SymbolsHandler>> _procSymbols;
};

