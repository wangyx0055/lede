#ifndef	__SDIO_CONFIG_H__
#define	__SDIO_CONFIG_H__

#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <initguid.h> // required for GUID definitions
//
// Disables few warnings so that we can build our driver with MSC W4 level.
// Disable warning C4057; X differs in indirection to slightly different base types from Y
// Disable warning C4100: unreferenced formal parameter
//
#include <ntddsd.h>
#include <Ntstrsafe.h>
#include "nic.h"
#include "CommonDriverUI.h"
#include "adapter.h"
#include "loopback.h"
#include "txbf.h"
#include "Register.h"
#include "fw_def.h"
#include "data_packet.h"
#include "HwHalProcess.h"
#include "devioctl.h"
#include "sdio.h"
#include "sdio_read.h"
#include "sdio_write.h"
#include "fw_cmd.h"
#include "hal.h"
#include "driver_common.h"

#endif	// __SDIO_CONFIG_H__
