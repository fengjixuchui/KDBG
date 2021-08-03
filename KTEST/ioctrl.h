#ifndef _IOCTRL_H
#define _IOCTRL_H

#include "global.h"

/*
* Write control codes.
*/

#define KM_WRITE_MEMORY_PROCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0100, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_WRITE_MEMORY_KERNEL  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0101, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

/*
* Read control codes.
*/

#define KM_READ_MEMORY_PROCESS  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0200, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_READ_MEMORY_KERNEL   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0201, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define KM_READ_MODULES_PROCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0202, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_READ_MODULES_KERNEL  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0203, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define KM_READ_THREADS_PROCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0204, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

/*
* Trace control codes.
*/

#define KM_TRACE_CONTEXT_START  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0300, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_TRACE_CONTEXT_STOP   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0301, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

/*
* Debug control codes.
*/

#define KM_DEBUG_BREAKPOINT_SET CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0400, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_DEBUG_BREAKPOINT_REM CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0401, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

/*
* Write request types.
*/

typedef struct _WRITE_MEMORY_PROCESS
{
  ULONG Pid = 0;
  WCHAR ImageName[256] = {};
  ULONG Offset = 0;
  ULONG Size = 0;
  BYTE Bytes[1024] = {};
} WRITE_MEMORY_PROCESS, * PWRITE_MEMORY_PROCESS;
typedef struct _WRITE_MEMORY_KERNEL
{
  CHAR ImageName[256] = {};
  ULONG Offset = 0;
  ULONG Size = 0;
  BYTE Bytes[1024] = {};
} WRITE_MEMORY_KERNEL, * PWRITE_MEMORY_KERNEL;

/*
* Read request types.
*/

typedef struct _READ_MEMORY_PROCESS
{
  ULONG Pid = 0;
  WCHAR ImageName[256] = {};
  ULONG Offset = 0;
  ULONG Size = 0;
} READ_MEMORY_PROCESS, * PREAD_MEMORY_PROCESS;
typedef struct _READ_MEMORY_KERNEL
{
  CHAR ImageName[256] = {};
  ULONG Offset = 0;
  ULONG Size = 0;
} READ_MEMORY_KERNEL, * PREAD_MEMORY_KERNEL;

typedef struct _READ_MODULES_PROCESS
{
  ULONG Pid = 0;
  ULONG Size = 0;
} READ_MODULES_PROCESS, * PREAD_MODULES_PROCESS;
typedef struct _READ_MODULES_KERNEL
{
  ULONG Size = 0;
} READ_MODULES_KERNEL, * PREAD_MODULES_KERNEL;

typedef struct _READ_THREADS_PROCESS
{
  ULONG Pid = 0;
  ULONG Size = 0;
} READ_THREADS_PROCESS, * PREAD_THREADS_PROCESS;

/*
* Trace request types.
*/

typedef struct _TRACE_CONTEXT_START
{
  ULONG64 Address = 0;
} TRACE_CONTEXT_START, * PTRACE_CONTEXT_START;
typedef struct _TRACE_CONTEXT_STOP
{
  ULONG Id = 0;
} TRACE_CONTEXT_STOP, * PTRACE_CONTEXT_STOP;

/*
* Debug request types.
*/

typedef struct _DEBUG_BREAKPOINT_SET
{
  ULONG64 Base = 0;
  enum TYPE
  {
    Software,
    Hardware,
  } Type = Software;
} DEBUG_BREAKPOINT_SET, * PDEBUG_BREAKPOINT_SET;
typedef struct _DEBUG_BREAKPOINT_REM
{
  ULONG64 Base = 0;
} DEBUG_BREAKPOINT_REM, * PDEBUG_BREAKPOINT_REM;

#endif