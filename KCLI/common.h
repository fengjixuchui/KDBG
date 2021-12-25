/*
* @author allogic
* @file common.h
* @brief Shared datatypes.
* @copyright allogic 2021. All Rights Reserved.
*/

#ifndef _COMMON_H
#define _COMMON_H

#include "global.h"

/*
* Common request types.
*/

typedef struct _KM_MODULE_PROCESS
{
  WCHAR Name[256] = {};
  ULONG64 Base = 0;
  ULONG Size = 0;
} KM_MODULE_PROCESS, * PKM_MODULE_PROCESS;
typedef struct _KM_MODULE_KERNEL
{
  CHAR Name[256] = {};
  ULONG64 Base = 0;
  ULONG Size = 0;
} KM_MODULE_KERNEL, * PKM_MODULE_KERNEL;

typedef struct _KM_THREAD_PROCESS
{
  ULONG Tid = 0;
  ULONG Pid = 0;
} KM_THREAD_PROCESS, * PKM_THREAD_PROCESS;

#endif