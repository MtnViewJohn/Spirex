//
//  selfdel.c
//
//  Self deleting executable for Win9x/WinNT (all versions)
//
//  J Brown 1/10/2003
//
//  This source file must be compiled with /GZ turned OFF
//  (basically, disable run-time stack checks)
//
//  Under debug build this is always on (MSVC6)
//
//
#include <windows.h>
#include <tchar.h>

#pragma pack(push, 1)

#define CODESIZE 0x200

//
//  Structure to inject into remote process. Contains
//  function pointers and code to execute.
//
typedef struct _SELFDEL
{
    struct _SELFDEL *Arg0;          // pointer to self

    BYTE    opCodes[CODESIZE];          // code

    HANDLE  hParent;                // parent process handle

    FARPROC fnWaitForSingleObject;
    FARPROC fnCloseHandle;
    FARPROC fnDeleteFile;
    FARPROC fnSleep;
    FARPROC fnExitProcess;
    FARPROC fnRemoveDirectory;
    FARPROC fnGetLastError;

    BOOL    fRemDir;

    TCHAR   szFileName[MAX_PATH];   // file to delete

} SELFDEL;

#pragma pack(pop)

#ifdef _DEBUG
#define FUNC_ADDR(func) (PVOID)(*(DWORD *)((BYTE *)func + 1) + (DWORD)((BYTE *)func + 5))
#else
#define FUNC_ADDR(func) func
#endif

//
//  Routine to execute in remote process.
//
static void remote_thread(SELFDEL *remote)
{
    // wait for parent process to terminate
    remote->fnWaitForSingleObject(remote->hParent, INFINITE);
    remote->fnCloseHandle(remote->hParent);

    // try to delete the executable file 
    while(!remote->fnDeleteFile(remote->szFileName))
    {
        // failed - try again in one second's time
        remote->fnSleep(1000);
    }

    // finished! exit so that we don't execute garbage code
    remote->fnExitProcess(0);
}

//
//  Delete currently running executable and exit
//
BOOL SelfDelete(BOOL fRemoveDirectory)
{
    STARTUPINFO         si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    CONTEXT             context;
    DWORD               oldProt;
    SELFDEL             local;
    DWORD               entrypoint;

    TCHAR               szExe[MAX_PATH] = _T("explorer.exe");

    //
    //  Create executable suspended
    //
    if(CreateProcess(0, szExe, 0, 0, 0, CREATE_SUSPENDED|IDLE_PRIORITY_CLASS, 0, 0, &si, &pi))
    {
        local.fnWaitForSingleObject     = (FARPROC)WaitForSingleObject;
        local.fnCloseHandle             = (FARPROC)CloseHandle;
        local.fnDeleteFile              = (FARPROC)DeleteFile;
        local.fnSleep                   = (FARPROC)Sleep;
        local.fnExitProcess             = (FARPROC)ExitProcess;
        local.fnRemoveDirectory         = (FARPROC)RemoveDirectory;
        local.fnGetLastError            = (FARPROC)GetLastError;

        local.fRemDir                   = fRemoveDirectory;

        // Give remote process a copy of our own process handle
        DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), 
            pi.hProcess, &local.hParent, 0, FALSE, 0);

        GetModuleFileName(0, local.szFileName, MAX_PATH);

        // copy in binary code
        memcpy(local.opCodes, FUNC_ADDR(remote_thread), CODESIZE);

        //
        // Allocate some space on process's stack and place
        // our SELFDEL structure there. Then set the instruction pointer 
        // to this location and let the process resume
        //
        context.ContextFlags = CONTEXT_INTEGER|CONTEXT_CONTROL;
        GetThreadContext(pi.hThread, &context);

        // Allocate space on stack (aligned to cache-line boundary)
        entrypoint = (context.Esp - sizeof(SELFDEL)) & ~0x1F;
        
        //
        // Place a pointer to the structure at the bottom-of-stack 
        // this pointer is located in such a way that it becomes 
        // the remote_thread's first argument!!
        //
        local.Arg0 = (SELFDEL *)entrypoint;

        context.Esp = entrypoint - 4;   // create dummy return address
        context.Eip = entrypoint + 4;   // offset of opCodes within structure

        // copy in our code+data at the exe's entry-point
        VirtualProtectEx(pi.hProcess,   (PVOID)entrypoint, sizeof(local), PAGE_EXECUTE_READWRITE, &oldProt);
        WriteProcessMemory(pi.hProcess, (PVOID)entrypoint, &local, sizeof(local), 0);

        FlushInstructionCache(pi.hProcess, (PVOID)entrypoint, sizeof(local));

        SetThreadContext(pi.hThread, &context);

        // Let the process continue
        ResumeThread(pi.hThread);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        return TRUE;
    }

    return FALSE;
}


