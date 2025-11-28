//
// Created by sn06129 on 2025/1/6.
//

#ifndef NSUKIT_MAKENSUKIT_H
#define NSUKIT_MAKENSUKIT_H

#include "base/base_kit.h"
#include "version.h"
#include "interface/interface.h"
#include "middleware/middleware.h"
#include "high_level/SuperAWG.h"
#include "NSUKit.h"

namespace nsukit {
    nsukit::BaseKit* make(const std::string& name){
        if (name == "xdma;xdma;xdma"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "xdma;xdma;sim"){
            return  new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "xdma;xdma;tcp"){
            return  new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "xdma;xdma;serial"){
            return  new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "xdma;sim;xdma"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;
        }else if (name == "xdma;sim;sim"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "xdma;sim;tcp"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "xdma;sim;serial"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "xdma;tcp;xdma"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "xdma;tcp;sim"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "xdma;tcp;tcp"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "xdma;tcp;serial"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "xdma;serial;xdma"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "xdma;serial;sim"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "xdma;serial;tcp"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "xdma;serial;serial"){
            return new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "sim;xdma;xdma"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "sim;xdma;sim"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "sim;xdma;tcp"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "sim;xdma;serial"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "sim;sim;xdma"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "sim;sim;sim"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "sim;sim;tcp"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "sim;sim;serial"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "sim;tcp;xdma"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "sim;tcp;sim"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::TCPCmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "sim;tcp;tcp"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "sim;tcp;serial"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "sim;serial;xdma"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "sim;serial;sim"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "sim;serial;tcp"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "sim;serial;serial"){
            return new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "tcp;xdma;xdma"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "tcp;xdma;sim"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "tcp;xdma;tcp"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "tcp;xdma;serial"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "tcp;sim;xdma"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "tcp;sim;sim"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "tcp;sim;tcp"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "tcp;sim;serial"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "tcp;tcp;xdma"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "tcp;tcp;sim"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::TCPCmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "tcp;tcp;tcp"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "tcp;tcp;serial"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "tcp;serial;xdma"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "tcp;serial;sim"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;
        } else if(name == "tcp;serial;tcp"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;
        } else if(name == "tcp;serial;serial"){
            return new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "serial;xdma;xdma"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "serial;xdma;sim"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "serial;xdma;tcp"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "serial;xdma;serial"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "serial;sim;xdma"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "serial;sim;sim"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "serial;sim;tcp"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "serial;sim;serial"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "serial;tcp;xdma"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "serial;tcp;sim"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::TCPCmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "serial;tcp;tcp"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "serial;tcp;serial"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;
        } else if (name == "serial;serial;xdma"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;
        } else if (name == "serial;serial;sim"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;
        } else if (name == "serial;serial;tcp"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;
        } else if (name == "serial;serial;serial"){
            return new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;
        }
        #ifdef NSUKIT_WITH_VPS
            else if (name == "vps;serial;serial"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;
            } else if (name == "vps;serial;xdma"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;
            } else if (name == "vps;serial;tcp"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;
            } else if (name == "vps;serial;sim"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;
            } else if (name == "vps;xdma;serial"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;
            } else if (name == "vps;xdma;xdma"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;
            } else if (name == "vps;xdma;tcp"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;
            } else if (name == "vps;xdma;sim"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;
            } else if (name == "vps;tcp;serial"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;
            } else if (name == "vps;tcp;xdma"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;
            } else if (name == "vps;tcp;tcp"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;
            } else if (name == "vps;tcp;sim"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::TCPCmdUItf, nsukit::SimStreamUItf>;
            } else if (name == "vps;sim;serial"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;
            } else if (name == "vps;sim;xdma"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;
            } else if (name == "vps;sim;tcp"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;
            } else if (name == "vps;sim;sim"){
                return new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;
            }
        #endif
        #ifdef NSUKIT_WITH_VPS

        #endif
        else {
            std::cout << "wrong, kit == Base" <<std::endl;
            return new nsukit::NSUSoc <nsukit::I_BaseCmdUItf, nsukit::I_BaseCmdUItf, nsukit::I_BaseStreamUItf>;
        }
    }
}
# endif //IVI_CONFIGNSUKITPARAM_H