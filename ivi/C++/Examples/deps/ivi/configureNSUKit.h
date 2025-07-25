#ifndef IVI_CONFIGNSUKITPARAM_H
#define IVI_CONFIGNSUKITPARAM_H

#ifdef IVI_WITH_VPS

# define configure_NSUKit_Param(param, name, value)                         \
do{                                                                         \
    if ((name) == "cmd_tcp_port"){                                          \
        (param).cmd_tcp_port = *static_cast<nsuRegAddr_t *>(value);         \
    }                                                                       \
    if ((name) == "cmd_tcp_timeout"){                                       \
        (param).cmd_tcp_timeout = *static_cast<float *>(value);             \
    }                                                                       \
    if ((name) == "stream_tcp_port"){                                       \
        (param).stream_tcp_port = *static_cast<nsuRegAddr_t *>(value);      \
    }                                                                       \
    if ((name) == "stream_tcp_block"){                                      \
        (param).stream_tcp_block = *static_cast<nsuRegAddr_t *>(value);     \
    }                                                                       \
    if ((name) == "cmd_baud_rate"){                                         \
        (param).cmd_baud_rate = *static_cast<nsuRegAddr_t *>(value);        \
    }                                                                       \
    if ((name) == "cmd_sent_base"){                                         \
        (param).cmd_sent_base = *static_cast<nsuRegAddr_t *>(value);        \
    }                                                                       \
    if ((name) == "cmd_recv_base"){                                         \
        (param).cmd_recv_base = *static_cast<nsuRegAddr_t *>(value);        \
    }                                                                       \
    if ((name) == "cmd_irq_base"){                                          \
        (param).cmd_irq_base = *static_cast<nsuRegAddr_t *>(value);         \
    }                                                                       \
    if ((name) == "cmd_sent_down_base"){                                    \
        (param).cmd_sent_down_base = *static_cast<nsuRegAddr_t *>(value);   \
    }                                                                       \
    if ((name) == "sim_target"){                                            \
        (param).sim_target = *static_cast<nsuRegAddr_t *>(value);           \
    }                                                                       \
    if ((name) == "icd_path"){                                              \
        (param).icd_path = *static_cast<std::string *>(value);              \
    }                                                                       \
    if ((name) == "check_cs_recv"){                                         \
        (param).check_cs_recv = *static_cast<bool *>(value);                \
    }                                                                       \
    if ((name) == "stream_mode"){                                           \
        if (*static_cast<std::string *>(value) == "REAL"){                  \
            (param).stream_mode = nsuMwParam_t::StreamMode::REAL;           \
        } else if (*static_cast<std::string *>(value) == "VIRTUAL"){        \
            (param).stream_mode = nsuMwParam_t::StreamMode::VIRTUAL;        \
        }                                                                   \
    }                                                                       \
    if ((name) == "vps_feedback_type"){                                     \
            (param).vps_feedback_type = *static_cast<nsuRegAddr_t *>(value);\
        }                                                                   \
} while(0)                                                                  \

# define create_NSUkit(name, kit)                                                                            \
    if ((name) == "xdma;xdma;xdma"){                                                                         \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;        \
    } else if ((name) == "xdma;xdma;sim"){                                                                   \
        (kit) =  new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;        \
    } else if ((name) == "xdma;xdma;tcp"){                                                                   \
        (kit) =  new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;        \
    } else if ((name) == "xdma;xdma;serial"){                                                                \
        (kit) =  new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;     \
    } else if ((name) == "xdma;sim;xdma"){                                                                   \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;         \
    } else if ((name) == "xdma;sim;sim"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;          \
    } else if ((name) == "xdma;sim;tcp"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;          \
    } else if ((name) == "xdma;sim;serial"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;       \
    } else if ((name) == "xdma;tcp;xdma"){                                                                   \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;         \
    } else if ((name) == "xdma;tcp;sim"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;         \
    } else if ((name) == "xdma;tcp;tcp"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;          \
    } else if ((name) == "xdma;tcp;serial"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;       \
    } else if ((name) == "xdma;serial;xdma"){                                                                \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;      \
    } else if ((name) == "xdma;serial;sim"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;       \
    } else if ((name) == "xdma;serial;tcp"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;       \
    } else if ((name) == "xdma;serial;serial"){                                                              \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;    \
    } else if ((name) == "sim;xdma;xdma"){                                                                   \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;         \
    } else if ((name) == "sim;xdma;sim"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;          \
    } else if ((name) == "sim;xdma;tcp"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;          \
    } else if ((name) == "sim;xdma;serial"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;       \
    } else if ((name) == "sim;sim;xdma"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;          \
    } else if ((name) == "sim;sim;sim"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;           \
    } else if ((name) == "sim;sim;tcp"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;           \
    } else if ((name) == "sim;sim;serial"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;        \
    } else if ((name) == "sim;tcp;xdma"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;          \
    } else if ((name) == "sim;tcp;sim"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::TCPCmdUItf, nsukit::SimStreamUItf>;           \
    } else if ((name) == "sim;tcp;tcp"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;           \
    } else if ((name) == "sim;tcp;serial"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;        \
    } else if ((name) == "sim;serial;xdma"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;       \
    } else if ((name) == "sim;serial;sim"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;        \
    } else if ((name) == "sim;serial;tcp"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;        \
    } else if ((name) == "sim;serial;serial"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;     \
    } else if ((name) == "tcp;xdma;xdma"){                                                                   \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;         \
    } else if ((name) == "tcp;xdma;sim"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;          \
    } else if ((name) == "tcp;xdma;tcp"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;          \
    } else if ((name) == "tcp;xdma;serial"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;       \
    } else if ((name) == "tcp;sim;xdma"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;          \
    } else if ((name) == "tcp;sim;sim"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;           \
    } else if ((name) == "tcp;sim;tcp"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;           \
    } else if ((name) == "tcp;sim;serial"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;        \
    } else if ((name) == "tcp;tcp;xdma"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;          \
    } else if ((name) == "tcp;tcp;sim"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::TCPCmdUItf, nsukit::SimStreamUItf>;           \
    } else if ((name) == "tcp;tcp;tcp"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;           \
    } else if ((name) == "tcp;tcp;serial"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;        \
    } else if ((name) == "tcp;serial;xdma"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;       \
    } else if ((name) == "tcp;serial;sim"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;        \
    } else if ((name) == "tcp;serial;tcp"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;        \
    } else if ((name) == "tcp;serial;serial"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;     \
    } else if ((name) == "serial;xdma;xdma"){                                                                \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;      \
    } else if ((name) == "serial;xdma;sim"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;       \
    } else if ((name) == "serial;xdma;tcp"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;       \
    } else if ((name) == "serial;xdma;serial"){                                                              \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;    \
    } else if ((name) == "serial;sim;xdma"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;       \
    } else if ((name) == "serial;sim;sim"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;        \
    } else if ((name) == "serial;sim;tcp"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;        \
    } else if ((name) == "serial;sim;serial"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;     \
    } else if ((name) == "serial;tcp;xdma"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;       \
    } else if ((name) == "serial;tcp;sim"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::TCPCmdUItf, nsukit::SimStreamUItf>;        \
    } else if ((name) == "serial;tcp;tcp"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;        \
    } else if ((name) == "serial;tcp;serial"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;     \
    } else if ((name) == "serial;serial;xdma"){                                                              \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;    \
    } else if ((name) == "serial;serial;sim"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;     \
    } else if ((name) == "serial;serial;tcp"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;     \
    } else if ((name) == "serial;serial;serial"){                                                            \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;  \
    } else if ((name) == "vps;serial;serial"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;     \
    } else if ((name) == "vps;serial;xdma"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;       \
    } else if ((name) == "vps;serial;tcp"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;        \
    } else if ((name) == "vps;serial;sim"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;        \
    } else if ((name) == "vps;xdma;serial"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;       \
    } else if ((name) == "vps;xdma;xdma"){                                                                   \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;         \
    } else if ((name) == "vps;xdma;tcp"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;          \
    } else if ((name) == "vps;xdma;sim"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;          \
    } else if ((name) == "vps;tcp;serial"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;        \
    } else if ((name) == "vps;tcp;xdma"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;          \
    } else if ((name) == "vps;tcp;tcp"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;           \
    } else if ((name) == "vps;tcp;sim"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::TCPCmdUItf, nsukit::SimStreamUItf>;           \
    } else if ((name) == "vps;sim;serial"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;        \
    } else if ((name) == "vps;sim;xdma"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;          \
    } else if ((name) == "vps;sim;tcp"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;           \
    } else if ((name) == "vps;sim;sim"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::VPSCmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;           \
    } else {                                                                                                 \
        VI_ERROR_PRINT("IviBase","ERROR, kit will be Base");                                                 \
        (kit) = new nsukit::NSUSoc <nsukit::I_BaseCmdUItf, nsukit::I_BaseCmdUItf, nsukit::I_BaseStreamUItf>; \
    }

#else

# define configure_NSUKit_Param(param, name, value)                         \
do{                                                                         \
    if ((name) == "cmd_tcp_port"){                                          \
        (param).cmd_tcp_port = *static_cast<nsuRegAddr_t *>(value);         \
    }                                                                       \
    if ((name) == "cmd_tcp_timeout"){                                       \
        (param).cmd_tcp_timeout = *static_cast<float *>(value);             \
    }                                                                       \
    if ((name) == "stream_tcp_port"){                                       \
        (param).stream_tcp_port = *static_cast<nsuRegAddr_t *>(value);      \
    }                                                                       \
    if ((name) == "stream_tcp_block"){                                      \
        (param).stream_tcp_block = *static_cast<nsuRegAddr_t *>(value);     \
    }                                                                       \
    if ((name) == "cmd_baud_rate"){                                         \
        (param).cmd_baud_rate = *static_cast<nsuRegAddr_t *>(value);        \
    }                                                                       \
    if ((name) == "cmd_sent_base"){                                         \
        (param).cmd_sent_base = *static_cast<nsuRegAddr_t *>(value);        \
    }                                                                       \
    if ((name) == "cmd_recv_base"){                                         \
        (param).cmd_recv_base = *static_cast<nsuRegAddr_t *>(value);        \
    }                                                                       \
    if ((name) == "cmd_irq_base"){                                          \
        (param).cmd_irq_base = *static_cast<nsuRegAddr_t *>(value);         \
    }                                                                       \
    if ((name) == "cmd_sent_down_base"){                                    \
        (param).cmd_sent_down_base = *static_cast<nsuRegAddr_t *>(value);   \
    }                                                                       \
    if ((name) == "sim_target"){                                            \
        (param).sim_target = *static_cast<nsuRegAddr_t *>(value);           \
    }                                                                       \
    if ((name) == "icd_path"){                                              \
        (param).icd_path = *static_cast<std::string *>(value);              \
    }                                                                       \
    if ((name) == "check_cs_recv"){                                         \
        (param).check_cs_recv = *static_cast<bool *>(value);                \
    }                                                                       \
    if ((name) == "stream_mode"){                                           \
        if (*static_cast<std::string *>(value) == "REAL"){                  \
            (param).stream_mode = nsuMwParam_t::StreamMode::REAL;           \
        } else if (*static_cast<std::string *>(value) == "VIRTUAL"){        \
            (param).stream_mode = nsuMwParam_t::StreamMode::VIRTUAL;        \
        }                                                                   \
    }                                                                       \
} while(0)                                                                  \

# define create_NSUkit(name, kit)                                                                            \
    if ((name) == "xdma;xdma;xdma"){                                                                         \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;        \
    } else if ((name) == "xdma;xdma;sim"){                                                                   \
        (kit) =  new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;        \
    } else if ((name) == "xdma;xdma;tcp"){                                                                   \
        (kit) =  new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;        \
    } else if ((name) == "xdma;xdma;serial"){                                                                \
        (kit) =  new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;     \
    } else if ((name) == "xdma;sim;xdma"){                                                                   \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;         \
    } else if ((name) == "xdma;sim;sim"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;          \
    } else if ((name) == "xdma;sim;tcp"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;          \
    } else if ((name) == "xdma;sim;serial"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;       \
    } else if ((name) == "xdma;tcp;xdma"){                                                                   \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;         \
    } else if ((name) == "xdma;tcp;sim"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;         \
    } else if ((name) == "xdma;tcp;tcp"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;          \
    } else if ((name) == "xdma;tcp;serial"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;       \
    } else if ((name) == "xdma;serial;xdma"){                                                                \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;      \
    } else if ((name) == "xdma;serial;sim"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;       \
    } else if ((name) == "xdma;serial;tcp"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;       \
    } else if ((name) == "xdma;serial;serial"){                                                              \
        (kit) = new nsukit::NSUSoc<nsukit::PCIECmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;    \
    } else if ((name) == "sim;xdma;xdma"){                                                                   \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;         \
    } else if ((name) == "sim;xdma;sim"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;          \
    } else if ((name) == "sim;xdma;tcp"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;          \
    } else if ((name) == "sim;xdma;serial"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;       \
    } else if ((name) == "sim;sim;xdma"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;          \
    } else if ((name) == "sim;sim;sim"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;           \
    } else if ((name) == "sim;sim;tcp"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;           \
    } else if ((name) == "sim;sim;serial"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;        \
    } else if ((name) == "sim;tcp;xdma"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;          \
    } else if ((name) == "sim;tcp;sim"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::TCPCmdUItf, nsukit::SimStreamUItf>;           \
    } else if ((name) == "sim;tcp;tcp"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;           \
    } else if ((name) == "sim;tcp;serial"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;        \
    } else if ((name) == "sim;serial;xdma"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;       \
    } else if ((name) == "sim;serial;sim"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;        \
    } else if ((name) == "sim;serial;tcp"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;        \
    } else if ((name) == "sim;serial;serial"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::SimCmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;     \
    } else if ((name) == "tcp;xdma;xdma"){                                                                   \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;         \
    } else if ((name) == "tcp;xdma;sim"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;          \
    } else if ((name) == "tcp;xdma;tcp"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;          \
    } else if ((name) == "tcp;xdma;serial"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;       \
    } else if ((name) == "tcp;sim;xdma"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;          \
    } else if ((name) == "tcp;sim;sim"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;           \
    } else if ((name) == "tcp;sim;tcp"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;           \
    } else if ((name) == "tcp;sim;serial"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;        \
    } else if ((name) == "tcp;tcp;xdma"){                                                                    \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;          \
    } else if ((name) == "tcp;tcp;sim"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::TCPCmdUItf, nsukit::SimStreamUItf>;           \
    } else if ((name) == "tcp;tcp;tcp"){                                                                     \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;           \
    } else if ((name) == "tcp;tcp;serial"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;        \
    } else if ((name) == "tcp;serial;xdma"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;       \
    } else if ((name) == "tcp;serial;sim"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;        \
    } else if ((name) == "tcp;serial;tcp"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;        \
    } else if ((name) == "tcp;serial;serial"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::TCPCmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;     \
    } else if ((name) == "serial;xdma;xdma"){                                                                \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::PCIECmdUItf, nsukit::PCIEStreamUItf>;      \
    } else if ((name) == "serial;xdma;sim"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::PCIECmdUItf, nsukit::SimStreamUItf>;       \
    } else if ((name) == "serial;xdma;tcp"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::PCIECmdUItf, nsukit::TCPStreamUItf>;       \
    } else if ((name) == "serial;xdma;serial"){                                                              \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::PCIECmdUItf, nsukit::SerialStreamUItf>;    \
    } else if ((name) == "serial;sim;xdma"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SimCmdUItf, nsukit::PCIEStreamUItf>;       \
    } else if ((name) == "serial;sim;sim"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SimCmdUItf, nsukit::SimStreamUItf>;        \
    } else if ((name) == "serial;sim;tcp"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SimCmdUItf, nsukit::TCPStreamUItf>;        \
    } else if ((name) == "serial;sim;serial"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SimCmdUItf, nsukit::SerialStreamUItf>;     \
    } else if ((name) == "serial;tcp;xdma"){                                                                 \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::TCPCmdUItf, nsukit::PCIEStreamUItf>;       \
    } else if ((name) == "serial;tcp;sim"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::TCPCmdUItf, nsukit::SimStreamUItf>;        \
    } else if ((name) == "serial;tcp;tcp"){                                                                  \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::TCPCmdUItf, nsukit::TCPStreamUItf>;        \
    } else if ((name) == "serial;tcp;serial"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::TCPCmdUItf, nsukit::SerialStreamUItf>;     \
    } else if ((name) == "serial;serial;xdma"){                                                              \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SerialCmdUItf, nsukit::PCIEStreamUItf>;    \
    } else if ((name) == "serial;serial;sim"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SerialCmdUItf, nsukit::SimStreamUItf>;     \
    } else if ((name) == "serial;serial;tcp"){                                                               \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SerialCmdUItf, nsukit::TCPStreamUItf>;     \
    } else if ((name) == "serial;serial;serial"){                                                            \
        (kit) = new nsukit::NSUSoc<nsukit::SerialCmdUItf, nsukit::SerialCmdUItf, nsukit::SerialStreamUItf>;  \
    } else {                                                                                                 \
        VI_ERROR_PRINT("IviBase","ERROR, kit will be Base");                                                 \
        (kit) = new nsukit::NSUSoc <nsukit::I_BaseCmdUItf, nsukit::I_BaseCmdUItf, nsukit::I_BaseStreamUItf>; \
    }
#endif
# endif //IVI_CONFIGNSUKITPARAM_H

