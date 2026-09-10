#pragma once

#include <iostream>

#if defined(_WIN32)
    #include <windows.h>
    #include <dbghelp.h>
#elif defined(__unix__) || defined(__APPLE__)
    #include <execinfo.h>
#endif

namespace StackTrace{
void print()
{
#if defined(_WIN32)
    HANDLE process = GetCurrentProcess();
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

    if (!SymInitialize(process, NULL, TRUE)) {
        std::cout << "SymInitialize failed\n";
        return;
    }

    // Load symbols for current module
    DWORD64 base = SymLoadModuleEx(process, NULL, NULL, NULL, 0, 0, NULL, 0);

    void* stack[64];
    USHORT frames = CaptureStackBackTrace(0, 64, stack, NULL);

    BYTE buffer[sizeof(SYMBOL_INFO) + 256];
    SYMBOL_INFO* symbol = (SYMBOL_INFO*)buffer;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = 255;

    for (USHORT i = 0; i < frames; ++i) {
        DWORD64 addr = (DWORD64)stack[i];

        if (SymFromAddr(process, addr, 0, symbol)) {
            std::cout << "[" << i << "] 0x" << std::hex << addr
                      << " " << symbol->Name << "\n";
        } else {
            std::cout << "[" << i << "] 0x" << std::hex << addr
                      << " (no symbol)\n";
        }
    }

#elif defined(__unix__) || defined(__APPLE__)

    void* buffer[64];
    int frames = backtrace(buffer, 64);

    std::cout << "Stack trace (" << frames << " frames):\n";
    for (int i = 0; i < frames; ++i)
        std::cout << "  [" << i << "] " << buffer[i] << "\n";

#else

    std::cout << "Stack trace not supported on this platform.\n";

#endif
}
}