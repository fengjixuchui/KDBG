#ifndef DEVICE_H
#define DEVICE_H

#include "global.h"

/*
* Communication device
*/

NTSTATUS
CreateDevice(
  PDRIVER_OBJECT driver,
  PDEVICE_OBJECT* device,
  PCWCHAR deviceName,
  PCWCHAR symbolicName);

NTSTATUS
DeleteDevice(
  PDEVICE_OBJECT device,
  PCWCHAR symbolicName);

#endif