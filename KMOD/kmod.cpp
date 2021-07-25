#include "global.h"
#include "common.h"
#include "ioctrl.h"
#include "krnl.h"
#include "pe.h"
#include "thread.h"
#include "trace.h"

// TODO: refactor ptr to stack objects

/*
* Global driver state.
*/

ULONG Pid = 0;

MODULE ModulesKernel[KMOD_MAX_MODULES_KERNEL] = {}; // Buffer is required in order to copy from process memory to kernel memory to process again.
MODULE ModulesProcess[KMOD_MAX_MODULES_PROCESS] = {}; // Buffer is required in order to copy from process memory to kernel memory to process again.
THREAD Threads[KMOD_MAX_THREADS] = {}; // Buffer is required in order to copy from process memory to kernel memory to process again.

/*
* Process utilities relative to kernel space.
*/

NTSTATUS CopyUserSpaceMemorySafe(PVOID dst, PVOID src, SIZE_T size, KPROCESSOR_MODE mode)
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  PMDL mdl = IoAllocateMdl(src, size, FALSE, FALSE, NULL);
  if (mdl)
  {
    MmProbeAndLockPages(mdl, mode, IoReadAccess);
    PVOID mappedSrc = MmMapLockedPagesSpecifyCache(mdl, mode, MmNonCached, NULL, FALSE, HighPagePriority);
    if (mappedSrc)
    {
      status = MmProtectMdlSystemAddress(mdl, PAGE_READONLY);
      if (NT_SUCCESS(status))
      {
        __try
        {
          memcpy(dst, mappedSrc, size);
          status = STATUS_SUCCESS;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
          KM_LOG_INFO("Something went wrong\n");
        }
      }
      MmUnmapLockedPages(mappedSrc, mdl);
    }
    MmUnlockPages(mdl);
  }
  IoFreeMdl(mdl);
  return status;
}

NTSTATUS FetchKernelModules()
{
  return STATUS_UNSUCCESSFUL;
}

NTSTATUS FetchProcessModules()
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  PEPROCESS process = NULL;
  KAPC_STATE apc;
  status = PsLookupProcessByProcessId((HANDLE)Pid, &process);
  if (NT_SUCCESS(status))
  {
    status = STATUS_UNSUCCESSFUL;
    KeStackAttachProcess(process, &apc);
    __try
    {
      PPEB64 peb = (PPEB64)PsGetProcessPeb(process);
      if (peb)
      {
        memset(ModulesProcess, 0, sizeof(MODULE) * KMOD_MAX_MODULES_PROCESS);
        PVOID imageBase = peb->ImageBaseAddress;
        PLDR_DATA_TABLE_ENTRY modules = CONTAINING_RECORD(peb->Ldr->InMemoryOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);;
        PLDR_DATA_TABLE_ENTRY module = NULL;
        PLIST_ENTRY moduleHead = modules->InMemoryOrderLinks.Flink;
        PLIST_ENTRY moduleEntry = moduleHead->Flink;
        ULONG count = 0;
        while (moduleEntry != moduleHead)
        {
          module = CONTAINING_RECORD(moduleEntry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
          if (module && module->DllBase)
          {
            CopyUserSpaceMemorySafe(&ModulesProcess[count].Base, &module->DllBase, sizeof(ULONG64), KernelMode);
            CopyUserSpaceMemorySafe(&ModulesProcess[count].Name, module->BaseDllName.Buffer, sizeof(WCHAR) * module->BaseDllName.Length, KernelMode);
            CopyUserSpaceMemorySafe(&ModulesProcess[count].Size, &module->SizeOfImage, sizeof(ULONG), KernelMode);
            count++;
            if (count >= KMOD_MAX_MODULES_PROCESS)
            {
              break;
            }
          }
          moduleEntry = moduleEntry->Flink;
        }
        KM_LOG_INFO("Fetched modules\n");
        status = STATUS_SUCCESS;
      }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
      KM_LOG_ERROR("Something went wrong!\n");
    }
    KeUnstackDetachProcess(&apc);
    ObDereferenceObject(process);
  }
  return status;
}
NTSTATUS FetchProcessThreads()
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  ULONG read = 0;
  PBYTE buffer = (PBYTE)KmAllocateMemory(TRUE, 1024 * 1024);
  status = ZwQuerySystemInformation(SystemProcessInformation, buffer, read, &read);
  ULONG processAcc = 0;
  while (1)
  {
    KM_LOG_INFO("Thread count %u\n", ((PSYSTEM_PROCESS_INFORMATION)buffer)->NumberOfThreads);
    //KM_LOG_INFO("Tid %u Pid %u\n", (ULONG)threads[i].ClientId.UniqueThread, (ULONG)threads[i].ClientId.UniqueProcess);
    //KM_LOG_INFO("Copy from %p to %p\n", &threads[i].ClientId.UniqueThread, &Threads[i].Tid);
    //for (ULONG i = 0; i < processInfo->NumberOfThreads; ++i)
    //{
    //  PSYSTEM_THREAD_INFORMATION thread = (PSYSTEM_THREAD_INFORMATION)(((PBYTE)processInfo) + sizeof(SYSTEM_PROCESS_INFORMATION) + sizeof(SYSTEM_THREAD_INFORMATION) * i);
    //  if (verbose)
    //  {
    //    LOG_INFO("\tTid: %u\n", *(PULONG)thread->ClientId.UniqueThread);
    //    LOG_INFO("\tBase: %p\n", thread->StartAddress);
    //    LOG_INFO("\n");
    //  }
    //  //request->Processes[processAcc].Threads[i].Tid = *(PULONG)thread->ClientId.UniqueThread;
    //  //request->Processes[processAcc].Threads[i].Base = thread->StartAddress;
    //  //request->Processes[processAcc].Threads[i].State = thread->ThreadState;
    //}
    if (!((PSYSTEM_PROCESS_INFORMATION)buffer)->NextEntryOffset)
    {
      KM_LOG_INFO("Fetched threads\n");
      status = STATUS_SUCCESS;
      break;
    }
    buffer += ((PSYSTEM_PROCESS_INFORMATION)buffer)->NextEntryOffset;
    processAcc++;
  }
  KmFreeMemory(buffer);
  return status;
}

NTSTATUS GetProcessModules(ULONG pid, SIZE_T size, SIZE_T& count, PVOID buffer)
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  PEPROCESS process = NULL;
  KAPC_STATE apc;
  status = PsLookupProcessByProcessId((HANDLE)pid, &process);
  if (NT_SUCCESS(status))
  {
    status = STATUS_UNSUCCESSFUL;
    KeStackAttachProcess(process, &apc);
    KM_LOG_INFO("Attached\n");
    __try
    {
      PPEB64 peb = (PPEB64)PsGetProcessPeb(process);
      if (peb)
      {
        KM_LOG_INFO("Found PEB\n");
        PVOID imageBase = peb->ImageBaseAddress;
        PLDR_DATA_TABLE_ENTRY modules = CONTAINING_RECORD(peb->Ldr->InMemoryOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);;
        PLDR_DATA_TABLE_ENTRY module = NULL;
        PLIST_ENTRY moduleHead = modules->InMemoryOrderLinks.Flink;
        PLIST_ENTRY moduleEntry = moduleHead->Flink;
        while (moduleEntry != moduleHead)
        {
          module = CONTAINING_RECORD(moduleEntry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
          KM_LOG_INFO("%ls\n", module->BaseDllName.Buffer);
          if (module && module->DllBase)
          {
            //KM_LOG_INFO("Copy from %p to %p\n", &module->DllBase, &((PMODULE)buffer)[count].Base);
            //((PMODULE)buffer)[count].Base = (ULONG64)module->DllBase;
            //wcscpy(((PMODULE)buffer)[count].Name, module->BaseDllName.Buffer);
            //((PMODULE)buffer)[count].Size = module->SizeOfImage;
            CopyUserSpaceMemorySafe(&((PMODULE)buffer)[count].Base, &module->DllBase, sizeof(ULONG64), UserMode);
            CopyUserSpaceMemorySafe(&((PMODULE)buffer)[count].Size, &module->DllBase, sizeof(SIZE_T), UserMode);
            count++;
            KM_LOG_INFO("%llu copied %ls\n", count, module->BaseDllName.Buffer);
            KM_LOG_INFO("value is %p\n");
            if (count >= size)
            {
              break;
            }
          }
          moduleEntry = moduleEntry->Flink;
        }
        status = STATUS_SUCCESS;
      }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
      KM_LOG_ERROR("Something went wrong!\n");
    }
    KeUnstackDetachProcess(&apc);
    ObDereferenceObject(process);
  }
  return status;
}
NTSTATUS GetProcessModuleBase(ULONG pid, PWCHAR name, PVOID& base)
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  PEPROCESS process = NULL;
  KAPC_STATE apc;
  status = PsLookupProcessByProcessId((HANDLE)pid, &process);
  if (NT_SUCCESS(status))
  {
    status = STATUS_UNSUCCESSFUL;
    KeStackAttachProcess(process, &apc);
    __try
    {
      PPEB64 peb = (PPEB64)PsGetProcessPeb(process);
      if (peb)
      {
        PVOID imageBase = peb->ImageBaseAddress;
        PLDR_DATA_TABLE_ENTRY modules = CONTAINING_RECORD(peb->Ldr->InMemoryOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);;
        PLDR_DATA_TABLE_ENTRY module = NULL;
        PLIST_ENTRY moduleHead = modules->InMemoryOrderLinks.Flink;
        PLIST_ENTRY moduleEntry = moduleHead->Flink;
        while (moduleEntry != moduleHead)
        {
          module = CONTAINING_RECORD(moduleEntry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
          if (module && module->DllBase)
          {
            if (_wcsicmp(name, module->BaseDllName.Buffer) == 0)
            {
              break;
            }
          }
          moduleEntry = moduleEntry->Flink;
        }
        base = module->DllBase;
        status = STATUS_SUCCESS;
        KM_LOG_INFO("Selected module %ls\n", module->BaseDllName.Buffer);
      }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
      KM_LOG_ERROR("Something went wrong!\n");
    }
    KeUnstackDetachProcess(&apc);
    ObDereferenceObject(process);
  }
  return status;
}

NTSTATUS ReadVirtualProcessMemory(ULONG pid, PVOID base, SIZE_T size, PVOID buffer)
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  PEPROCESS process = NULL;
  KAPC_STATE apc;
  status = PsLookupProcessByProcessId((HANDLE)pid, &process);
  if (NT_SUCCESS(status))
  {
    status = STATUS_UNSUCCESSFUL;
    PBYTE asyncBuffer = (PBYTE)KmAllocateMemory(TRUE, size);
    if (asyncBuffer)
    {
      PMDL mdl = IoAllocateMdl(base, size, FALSE, FALSE, NULL);
      if (mdl)
      {
        KeStackAttachProcess(process, &apc);
        __try
        {
          MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);
          PBYTE mappedBuffer = (PBYTE)MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmNonCached, NULL, FALSE, HighPagePriority);
          if (mappedBuffer)
          {
            status = MmProtectMdlSystemAddress(mdl, PAGE_READONLY);
            if (NT_SUCCESS(status))
            {
              status = STATUS_UNSUCCESSFUL;
              memcpy(asyncBuffer, mappedBuffer, size);
              KM_LOG_INFO("Copy successfull\n");
              status = STATUS_SUCCESS;
            }
            MmUnmapLockedPages(mappedBuffer, mdl);
          }
          MmUnlockPages(mdl);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
          KM_LOG_ERROR("Something went wrong!\n");
          status = STATUS_UNHANDLED_EXCEPTION;
        }
        KeUnstackDetachProcess(&apc);
        IoFreeMdl(mdl);
      }
      memcpy(buffer, asyncBuffer, size);
      KmFreeMemory(asyncBuffer);
    }
    ObDereferenceObject(process);
  }
  return status;
}
NTSTATUS WriteVirtualProcessMemory(ULONG pid, PVOID base, SIZE_T size, PVOID buffer)
{
  return STATUS_UNSUCCESSFUL;
}

/*
* Communication device.
*/

PDEVICE_OBJECT Device = NULL;

#define KMOD_DEVICE_NAME L"\\Device\\KMOD"
#define KMOD_DEVICE_SYMBOL_NAME L"\\DosDevices\\KMOD"

/*
* Request/Response handlers.
*/

NTSTATUS HandleProcessAttachRequest(PREQ_PROCESS_ATTACH req)
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  Pid = req->In.Pid;
  KM_LOG_INFO("Attached to process %u\n", Pid);
  status = STATUS_SUCCESS;
  return status;
}
NTSTATUS HandleProcessModulesRequest(PREQ_PROCESS_MODULES req)
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  if (Pid)
  {
    status = FetchProcessModules();
    if (NT_SUCCESS(status))
    {
      memcpy(req->Out.Buffer, ModulesProcess, sizeof(MODULE) * KMOD_MAX_MODULES_PROCESS);
    }
  }
  return status;
}
NTSTATUS HandleProcessThreadsRequest(PREQ_PROCESS_THREADS req)
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  if (Pid)
  {
    status = FetchProcessThreads();
    if (NT_SUCCESS(status))
    {
      memcpy(req->Out.Buffer, Threads, sizeof(THREAD) * KMOD_MAX_THREADS);
    }
  }
  return status;
}
NTSTATUS HandleMemoryReadRequest(PREQ_MEMORY_READ req)
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  PVOID base = NULL;
  if (Pid)
  {
    status = GetProcessModuleBase(Pid, req->In.Name, base);
    if (NT_SUCCESS(status))
    {
      req->Out.Base = (ULONG64)base;
      status = ReadVirtualProcessMemory(Pid, (PVOID)((PBYTE)base + req->In.Offset), req->In.Size, req->Out.Buffer);
    }
  }
  return status;
}
NTSTATUS HandleMemoryWriteRequest(PREQ_MEMORY_WRITE req)
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  PVOID base = NULL;
  if (Pid)
  {
    status = GetProcessModuleBase(Pid, req->In.Name, base);
    if (NT_SUCCESS(status))
    {
      req->Out.Base = (ULONG64)base;
      status = WriteVirtualProcessMemory(Pid, (PVOID)((PBYTE)base + req->In.Offset), req->In.Size, req->Out.Buffer);
    }
  }
  return status;
}

/*
* Communication socket.
*/

#define MAX_TCP_SESSIONS 64

KMUTEX ShutdownMutex = {};

HANDLE ListenThreadHandle = NULL;
HANDLE SessionThreadHandles[MAX_TCP_SESSIONS] = {};

INT ServerFd = 0;
INT ClientFds[MAX_TCP_SESSIONS] = {};

VOID SessionThread(PVOID context)
{
  INT clientFd = *(PINT)context;
  char buffer[1024] = {};
  KM_LOG_INFO("New session in receive state\n");
  while (1)
  {
    recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    buffer[sizeof(buffer) - 1] = '\0';
    KM_LOG_INFO("Received %s\n", buffer);
    send(clientFd, buffer, sizeof(buffer), 0);
  }
  KM_LOG_INFO("New session receive state ended\n");
}
VOID ListenThread(PVOID context)
{
  ServerFd = socket_listen(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr{};
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(9095);
  ULONG sessionId = 0;
  while (KeReadStateMutex(&ShutdownMutex) == 0)
  {
    if (bind(ServerFd, (struct sockaddr*)&addr, sizeof(addr)) == 0)
    {
      socklen_t addrlen = sizeof(addr);
      ClientFds[sessionId] = accept(ServerFd, (struct sockaddr*)&addr, &addrlen);
      KM_LOG_INFO("Opened new pipe %u\n", ClientFds[sessionId]);
      NTSTATUS status = PsCreateSystemThread(&SessionThreadHandles[sessionId], STANDARD_RIGHTS_ALL, NULL, NULL, NULL, SessionThread, &ClientFds[sessionId]);
      {
        KM_LOG_INFO("Opened new session for %u\n", ClientFds[sessionId]);
      }
      sessionId++;
    }
    KmSleep(1000);
  }
  KM_LOG_INFO("Mutex signaled closing sessions now\n");
  for (SIZE_T i = 0; i < sessionId; ++i)
  {
    closesocket(ClientFds[i]);
    ZwClose(SessionThreadHandles[i]);
  }
  KM_LOG_INFO("Closing listening thread\n");
}

/*
* Entry point.
*/

VOID DriverUnload(PDRIVER_OBJECT driver)
{
  UNREFERENCED_PARAMETER(driver);
  NTSTATUS status = STATUS_SUCCESS;
  KM_LOG_INFO("Mutex signaled now\n");
  KeReleaseMutex(&ShutdownMutex, TRUE);
  KM_LOG_INFO("Closing listening thread\n");
  status = ZwClose(ListenThreadHandle);
  if (NT_SUCCESS(status))
  {
    KM_LOG_INFO("Listening thread closed\n");
  }
  else
  {
    KM_LOG_ERROR("Failed closing listening thread\n");
  }
  closesocket(ServerFd);
  KsDestroy();
  KM_LOG_INFO("KMOD deinitialized\n");
}
NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING regPath)
{
  UNREFERENCED_PARAMETER(regPath);
  NTSTATUS status = STATUS_SUCCESS;
  driver->DriverUnload = DriverUnload;
  KeInitializeMutex(&ShutdownMutex, 0);
  status = KsInitialize();
  if (NT_SUCCESS(status))
  {
    status = PsCreateSystemThread(&ListenThreadHandle, STANDARD_RIGHTS_ALL, NULL, NULL, NULL, ListenThread, NULL);
    if (NT_SUCCESS(status))
    {
      KM_LOG_INFO("Starting TCP thread\n");
      KM_LOG_INFO("KMOD initialized\n");
    }
  }  
  return status;
}