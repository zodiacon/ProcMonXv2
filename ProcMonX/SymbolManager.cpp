#include "pch.h"
#include "SymbolManager.h"

SymbolManager& SymbolManager::Get() {
    static SymbolManager mgr;
    return mgr;
}

SymbolManager::~SymbolManager() {
    Term();
}

void SymbolManager::Term() {
    _procSymbols.clear();
}

SymbolsHandler* SymbolManager::GetCommon() {
    return &_commonSymbols;
}

SymbolsHandler* SymbolManager::GetForProcess(DWORD pid) {
    auto it = _procSymbols.find(pid);
    if (it == _procSymbols.end()) {
        // attempt to get one
        
        auto symbols = SymbolsHandler::CreateForProcess(pid);
        if (symbols == nullptr)
            return nullptr;
        auto sym = symbols.get();
        _procSymbols.insert({ pid, std::move(symbols) });
        return sym;
    }
    if (::WaitForSingleObject(it->second->GetHandle(), 0) == WAIT_OBJECT_0) {
        // process dead, remove and try again
        _procSymbols.erase(pid);
        return GetForProcess(pid);
    }
    return it->second.get();
}

std::unique_ptr<SymbolInfo> SymbolManager::GetSymbolFromAddress(DWORD pid, DWORD64 address, PDWORD64 offset) {
    if ((int64_t)address < 0) {
        // kernel address
        _commonSymbols.LoadKernelModule(address);
        return _commonSymbols.GetSymbolFromAddress(address, offset);
    }

    auto handler = GetForProcess(pid);
    if (handler) {
        auto symbol = handler->GetSymbolFromAddress(address, offset);
        if (symbol)
            return symbol;
    }
    return nullptr;
}

SymbolManager::SymbolManager() {
}
