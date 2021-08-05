#ifndef _IOCTRL_H
#define _IOCTRL_H

#include "global.h"

/*
* Write control codes.
*/

#define KM_WRITE_MEMORY_PROCESS  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0100, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_WRITE_MEMORY_KERNEL   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0101, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

/*
* Read control codes.
*/

#define KM_READ_MEMORY_PROCESS   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0200, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_READ_MEMORY_KERNEL    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0201, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_READ_MODULES_PROCESS  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0202, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_READ_MODULES_KERNEL   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0203, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_READ_THREADS_PROCESS  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0204, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_READ_SCAN_RESULTS     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0205, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

/*
* Trace control codes.
*/

#define KM_TRACE_CONTEXT_START   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0300, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_TRACE_CONTEXT_STOP    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0301, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

/*
* Debug control codes.
*/

#define KM_DEBUG_BREAKPOINT_SET  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0400, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_DEBUG_BREAKPOINT_REM  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0401, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

/*
* Scan control codes.
*/

#define KM_SCAN_NEW              CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0500, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_SCAN_UNDO             CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0501, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_SCAN_INT              CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0502, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_SCAN_REAL             CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0503, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_SCAN_BYTES            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0504, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_SCAN_FILTER_CHANGED   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0505, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_SCAN_FILTER_UNCHANGED CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0506, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_SCAN_FILTER_INCREASED CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0507, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KM_SCAN_FILTER_DECREASED CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0508, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

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
typedef struct _READ_SCAN_RESULTS
{

} READ_SCAN_RESULTS, * PREAD_SCAN_RESULTS;

/*
* Trace request types.
*/

typedef struct _TRACE_CONTEXT_START
{
  ULONG Tid = 0;
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

/*
* Scan request types.
*/

typedef struct _SCAN_NEW
{

} SCAN_NEW, * PSCAN_NEW;
typedef struct _SCAN_UNDO
{

} SCAN_UNDO, * PSCAN_UNDO;
typedef struct _SCAN_INT
{
  union
  {
    CHAR S8;
    UCHAR U8;
    SHORT S16;
    USHORT U16;
    INT S32;
    UINT U32;
  } Value = {};
  enum WIDTH
  {
    Char,
    Short,
    Int,
  } Width = Int;
  enum SIGNEDNESS
  {
    Signed,
    Unsigned,
  } Signedness = Signed;
} SCAN_INT, * PSCAN_INT;
typedef struct _SCAN_REAL
{
  union
  {
    FLOAT R32;
    DOUBLE R64;
  } Value = {};
  enum WIDTH
  {
    Float,
    Double
  } Width = Float;
} SCAN_REAL, * PSCAN_REAL;
typedef struct _SCAN_BYTES
{
  BYTE Bytes[256] = {};
  ULONG Size = 0;
} SCAN_BYTES, * PSCAN_BYTES;
typedef struct _SCAN_FILTER_CHANGED
{

} SCAN_FILTER_CHANGED, * PSCAN_FILTER_CHANGED;
typedef struct _SCAN_FILTER_UNCHANGED
{

} SCAN_FILTER_UNCHANGED, * PSCAN_FILTER_UNCHANGED;
typedef struct _SCAN_FILTER_INCREASED
{

} SCAN_FILTER_INCREASED, * PSCAN_FILTER_INCREASED;
typedef struct _SCAN_FILTER_DECREASED
{

} SCAN_FILTER_DECREASED, * PSCAN_FILTER_DECREASED;

#endif