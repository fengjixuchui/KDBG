#ifndef _KDRV_H
#define _KDRV_H

#include "global.h"
#include "ioctrl.h"

struct KDRV
{
  NTSTATUS Status = STATUS_SUCCESS;
  UNICODE_STRING DeviceName;
  UNICODE_STRING DeviceSymName;
  PDEVICE_OBJECT DeviceObject = NULL;

  NTSTATUS Initialize(PDRIVER_OBJECT driverObject);
  NTSTATUS DeInitialize(PDRIVER_OBJECT driverObject);
};

#endif