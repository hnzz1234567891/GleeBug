#ifndef MYDEBUGGER_H
#define MYDEBUGGER_H

#include <GleeBug/Debugger.h>

using namespace GleeBug;

class MyDebugger : public Debugger
{
protected:
    void cbEntryBreakpoint(const BreakpointInfo & info)
    {
        printf("Reached entry breakpoint! GIP: 0x%p\n",
            mRegisters->Gip());
        if (mProcess->DeleteBreakpoint(info.address))
            printf("Entry breakpoint deleted!\n");
        else
            printf("Failed to delete entry breakpoint...\n");
        mThread->StepInto([this]()
        {
            printf("Step after entry breakpoint! GIP: 0x%p\n",
                mRegisters->Gip());
        });
    }

    void cbEntryHardwareBreakpoint(const BreakpointInfo & info)
    {
        printf("Reached entry hardware breakpoint! GIP: 0x%p\n",
            mRegisters->Gip());
        if (mProcess->DeleteHardwareBreakpoint(info.address))
            printf("Entry hardware breakpoint deleted!\n");
        else
            printf("Failed to delete entry hardware breakpoint...\n");
        mThread->StepInto([this]()
        {
            printf("Step after entry hardware breakpoint! GIP: 0x%p\n",
                mRegisters->Gip());
        });
    }

    void cbStepSystem()
    {
        printf("Reached step after system breakpoint, GIP: 0x%p!\n",
            mRegisters->Gip());
    }

    void cbCreateProcessEvent(const CREATE_PROCESS_DEBUG_INFO & createProcess, const Process & process) override
    {
        ptr entry = ptr(createProcess.lpStartAddress);
        printf("Process %d created with entry 0x%p\n",
            mDebugEvent.dwProcessId,
            entry);
        /*HardwareSlot slot;
        if (mProcess->GetFreeHardwareBreakpointSlot(slot))
        {
            if (mProcess->SetHardwareBreakpoint(entry, slot, this, &MyDebugger::cbEntryHardwareBreakpoint, HardwareType::Execute, HardwareSize::SizeByte))
                printf("Hardware breakpoint set at 0x%p!\n", entry);
            else
                printf("Failed to set hardware breakpoint at 0x%p\n", entry);
        }
        else
            printf("No free hardware breakpoint slot...\n");*/

        if(mProcess->SetBreakpoint(entry, this, &MyDebugger::cbEntryBreakpoint))
            printf("Breakpoint set at 0x%p!\n", entry);
        else
            printf("Failed to set breakpoint at 0x%p...\b", entry);
        uint8 test[5];
        ptr start = entry - 2;
        printf("unsafe: ");
        mProcess->MemReadUnsafe(start, test, sizeof(test));
        for (int i = 0; i < sizeof(test); i++)
            printf("%02X ", test[i]);
        puts("");
        mProcess->MemReadSafe(start, test, sizeof(test));
        printf("  safe: ");
        for (int i = 0; i < sizeof(test); i++)
            printf("%02X ", test[i]);
        puts("");
    }

    void cbExitProcessEvent(const EXIT_PROCESS_DEBUG_INFO & exitProcess, const Process & process) override
    {
        printf("Process %u terminated with exit code 0x%08X\n",
            mDebugEvent.dwProcessId,
            exitProcess.dwExitCode);
    }

    void cbCreateThreadEvent(const CREATE_THREAD_DEBUG_INFO & createThread, const Thread & thread) override
    {
        printf("Thread %u created with entry 0x%p\n",
            mDebugEvent.dwThreadId,
            createThread.lpStartAddress);
    }

    void cbExitThreadEvent(const EXIT_THREAD_DEBUG_INFO & exitThread, const Thread & thread) override
    {
        printf("Thread %u terminated with exit code 0x%08X\n",
            mDebugEvent.dwThreadId,
            exitThread.dwExitCode);
    }

    void cbLoadDllEvent(const LOAD_DLL_DEBUG_INFO & loadDll, const Dll & dll) override
    {
        printf("DLL loaded at 0x%p\n",
            loadDll.lpBaseOfDll);
    }

    void cbUnloadDllEvent(const UNLOAD_DLL_DEBUG_INFO & unloadDll, const Dll & dll) override
    {
        printf("DLL 0x%p unloaded\n",
            unloadDll.lpBaseOfDll);
    }

    void cbExceptionEvent(const EXCEPTION_DEBUG_INFO & exceptionInfo) override
    {
        const char* exceptionType = exceptionInfo.dwFirstChance ? "First Chance" : "Second Chance";
        printf("%s exception with code 0x%08X at 0x%p\n",
            exceptionType,
            exceptionInfo.ExceptionRecord.ExceptionCode,
            exceptionInfo.ExceptionRecord.ExceptionAddress);
    }

    void cbDebugStringEvent(const OUTPUT_DEBUG_STRING_INFO & debugString) override
    {
        printf("Debug string at 0x%p with length %d\n",
            debugString.lpDebugStringData,
            debugString.nDebugStringLength);
    }

    void cbRipEvent(const RIP_INFO & rip) override
    {
        printf("RIP event type 0x%X, error 0x%X",
            rip.dwType,
            rip.dwError);
    }

    void cbAttachBreakpoint() override
    {
        printf("Attach breakpoint reached, GIP: 0x%p\n",
            mRegisters->Gip());
    }

    void cbSystemBreakpoint() override
    {
        printf("System breakpoint reached, GIP: 0x%p\n",
            mRegisters->Gip());
        mThread->StepInto(this, &MyDebugger::cbStepSystem);
    }

    void cbInternalError(const std::string & error) override
    {
        printf("Internal Error: %s\n",
            error.c_str());
    }

    void cbBreakpoint(const BreakpointInfo & info) override
    {
        printf("Breakpoint on 0x%p!\n",
            info.address);
    }

    void cbUnhandledException(const EXCEPTION_RECORD & exceptionRecord, bool firstChance) override
    {
        printf("Unhandled exception (%s) 0x%08X on 0x%p, GIP: 0x%p\n",
            firstChance ? "first chance" : "second chance",
            exceptionRecord.ExceptionCode,
            exceptionRecord.ExceptionAddress,
            mRegisters->Gip());
    }
};

#endif //MYDEBUGGER_H