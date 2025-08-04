//
// Created by ljz_w on 2024/11/8.
//
#ifndef NSUKIT_TRF_H
#define NSUKIT_TRF_H

#include "base_itf.h"
#include "../xdma_api.h"
#include "../cs_over_pc.h"

#include <queue>
#include <list>
#include <unordered_map>

namespace nsukit {
    class vps_XDMAOperation_t {
    protected:
        std::map<int, int> openedBoard = {};
        std::mutex OpLock{};
        std::map<int, int> vpsNUMs = {};
//        uint8_t vpsNUMs=0;

    public:
        /**
         * 开启板卡，并记录开启次数，只有第一次真正开启板卡
         * @param board xdma板卡号
         * @return
         */
        nsukitStatus_t vps_open_xdma_board(int board);

        /**
         * 关闭板卡并记录关闭次数，只有最后一次真正关闭
         * @param board xdma板卡号
         * @return
         */
        nsukitStatus_t vps_close_xdma_board(int board);
    };

    class NSU_DLLEXPORT VPSCmdUItf : public I_BaseCmdUItf {
    protected:

        nsuBoardNum_t vpsBoard = 0;
        uint32_t feedback_type = 0;

        std::queue<nsuCharBuf_p> feedback_buf_q;
        std::mutex OpLock{};

//        std::map<int, int> vpsObject_map = {};
        /**
         * ICD over AXI方式的读取反馈前的预处理，等待对端发送反馈
         * @return
         */
//        nsukitStatus_t per_recv();

    public:


        /**
         * @note 将基类 nsukit::I_BaseCmdUItf 的成员变量mixin_赋值为 nsukit::Mixin_NativeRegCmd 的实例
         */
        VPSCmdUItf() { mixin_ = new Mixin_NativeRegCmd(this); };

        ~VPSCmdUItf() override { 
            delete mixin_; 
//            vpsObject_map.clear();
        };

        std::map<nsuBoardNum_t, CsOverPc*> vpsObject_map = {};

//        std::list<CsOverPc *> vpsObject_list = {};

        /**
         *
         * @param board 板卡号
         * @param sent_base ICDOverAXI发送基地址
         * @param recv_base ICDOverAXI接收基地址
         * @return 是否连接成功
         */
        nsukitStatus_t accept(nsuInitParam_t *param) override;

        /**
         *
         * @return
         */
        nsukitStatus_t close() override;


//        /**
//         *
//         * @param addr
//         * @param value
//         * @return
//         */
//        nsukitStatus_t write(nsuRegAddr_t addr, nsuRegValue_t value) override;
//
//        /**
//         *
//         * @param addr
//         * @param buf
//         * @return
//         */
//        nsukitStatus_t read(nsuRegAddr_t addr, nsuRegValue_t *buf) override;

        /**
         *
         * @param bytes
         * @return
         */
        nsukitStatus_t send_bytes(nsuBytes_t &bytes) override;

        /**
         *
         * @param bytes
         * @return
         */
        nsukitStatus_t send_bytes(nsuCharBuf_p bytes, nsuSize_t length) override;

        /**
         *
         * @param size
         * @param buf
         * @return
         */
        nsukitStatus_t recv_bytes(nsuSize_t size, nsuCharBuf_p buf) override;
    };
}
#endif