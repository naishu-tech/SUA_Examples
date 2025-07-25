//
// Created by 56585 on 2023/8/15.
//

#ifndef NSUKIT_INTERFACE_H
#define NSUKIT_INTERFACE_H

#include "base_itf.h"
#include "sim_interface.h"
#include "serial_interface.h"
#include "tcp_interface.h"

#ifdef NSUKIT_WITH_XDMA
#include "pcie_interface.h"
#endif

#ifdef NSUKIT_WITH_VPS
#include "vps_interface.h"
#endif

#ifdef NSUKIT_WITH_P2P
#include "FPGAPeer_interface.h"
#include "GPUPeer_interface.h"
#endif

#endif //NSUKIT_INTERFACE_H
