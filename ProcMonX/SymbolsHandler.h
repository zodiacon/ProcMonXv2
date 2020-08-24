#pragma once

#pragma warning(disable:4091)
#include <DbgHelp.h>
#pragma warning(default:4091)

class SymbolInfo {
public:
    SymbolInfo();
    ~SymbolInfo();

    operator PSYMBOL_INFO() const  {
        return m_Symbol;
    }

    SYMBOL_INFO* GetSymbolInfo() const {
        return m_Symbol;
    }

    IMAGEHLP_MODULE64 ModuleInfo;

private:
    SYMBOL_INFO* m_Symbol;
};

class SymbolsHandler final {
public:
    SymbolsHandler(HANDLE hProcess = ::GetCurrentProcess(), PCSTR searchPath = nullptr, DWORD symOptions =
        SYMOPT_UNDNAME | SYMOPT_CASE_INSENSITIVE | SYMOPT_AUTO_PUBLICS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_OMAP_FIND_NEAREST | SYMOPT_DEFERRED_LOADS);
    static std::unique_ptr<SymbolsHandler> CreateForProcess(DWORD pid, PCSTR searchPath = nullptr);
    ~SymbolsHandler();

    HANDLE GetHandle() const;
    ULONG64 LoadSymbolsForModule(PCSTR moduleName, DWORD64 baseAddress = 0);
    std::unique_ptr<SymbolInfo> GetSymbolFromName(PCSTR name);
    std::unique_ptr<SymbolInfo> GetSymbolFromAddress(DWORD64 address, PDWORD64 offset = nullptr);
    IMAGEHLP_MODULE64 GetModuleInfo(DWORD64 address) const;
    bool LoadDefaultModules();
    DWORD64 LoadKernelModule(DWORD64 address);

private:
    BOOL Callback(ULONG code, ULONG64 data);

    HANDLE m_hProcess;
};

