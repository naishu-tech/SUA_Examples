//
// Created by sn06129 on 2024/12/5.
//

#ifndef NSUKIT_FPGAPEER_INTERFACE_H
#define NSUKIT_FPGAPEER_INTERFACE_H

#include "base_itf.h"
#include "xdma_api.h"

namespace nsukit {
    class FPGA_P2P_XDMAOperation_t {
    protected:
        std::map<int, int> openedBoard = {};
        std::mutex OpLock{};

    public:
        /**
         * 开启板卡，并记录开启次数，只有第一次真正开启板卡
         * @param board xdma板卡号
         * @return
         */
        nsukitStatus_t FPGA_P2P_open_xdma_board(int board);

        /**
         * 关闭板卡并记录关闭次数，只有最后一次真正关闭
         * @param board xdma板卡号
         * @return
         */
        nsukitStatus_t FPGA_P2P_close_xdma_board(int board);
    };

    class NSU_DLLEXPORT FPGAPeerStreamUItf : public I_BaseStreamUItf {
    private:
        const uint8_t byteWidth = 4;
        const uint8_t maxChnl = 4;
    protected:
        // struct StreamProcess {
        //     nsuStreamLen_t current = 0;
        //     nsuStreamLen_t total = 0;
        // };

        // nsuBoardNum_t pciBoard = 0;

        // std::map<nsuMemory_p, StreamProcess> uploadProcess{};

        // std::map<nsuMemory_p, StreamProcess> downloadProcess{};


        struct StreamProcess {
            nsuBoardNum_t src_board = 0; // 源FPGA板卡号
            nsuBoardNum_t dst_board = 0; // 目标FPGA板卡号
            nsuStreamLen_t current = 0;
            nsuStreamLen_t total = 0;
            HANDLE dma = nullptr; // DMA句柄
            unsigned long long bar_addr = 0; // 目标FPGA的BAR地址
            int fdLength = 0; // 记录申请的fd长度
        };

        nsuBoardNum_t pciBoard = 0;
        nsuBoardNum_t srcBoard = 0;  // 源FPGA板卡号
        nsuBoardNum_t dstBoard = 0;  // 目标FPGA板卡号
        unsigned long long dstBarAddr = 0; // 目标FPGA的bypass BAR地址
        unsigned long long dstBarSize = 0; // 目标FPGA的bypass BAR大小

        std::map<nsuMemory_p, StreamProcess> uploadProcess{};
        std::map<nsuMemory_p, StreamProcess> downloadProcess{};


    public:
        /**
         *
         */
        FPGAPeerStreamUItf();

        ~FPGAPeerStreamUItf() override = default;

        /**
         *
         * @param param
         * @return
         */
        nsukitStatus_t accept(nsuInitParam_t *param) override;

        /**
         *
         * @return
         */
        nsukitStatus_t close() override;

        /**
         *
         * @param s
         * @return
         */
        nsukitStatus_t set_timeout(float s) override;

        /**
         *
         * @param length
         * @param buf
         * @return
         */
        nsuMemory_p alloc_buffer(nsuStreamLen_t length, nsuVoidBuf_p buf = nullptr) override;

        /**
         *
         * @param fd
         * @return
         */
        nsukitStatus_t free_buffer(nsuMemory_p fd) override;

        /**
         *
         * @param fd
         * @param length
         * @return
         */
        nsuVoidBuf_p get_buffer(nsuMemory_p fd, nsuStreamLen_t length) override;

        /**
         *
         * @param chnl
         * @param fd
         * @param length
         * @param offset
         * @return
         */
        nsukitStatus_t
        open_send(nsuChnlNum_t chnl, nsuMemory_p fd, nsuStreamLen_t length, nsuStreamLen_t offset = 0) override;

        /**
         *
         * @param chnl
         * @param fd
         * @param length
         * @param offset
         * @return
         */
        nsukitStatus_t
        open_recv(nsuChnlNum_t chnl, nsuMemory_p fd, nsuStreamLen_t length, nsuStreamLen_t offset = 0) override;

        /**
         *
         * @param fd
         * @param timeout
         * @return
         */
        nsukitStatus_t wait_stream(nsuMemory_p fd, float timeout = 1.) override;
        /**
         *
         * @param fd
         * @return
         */
        nsukitStatus_t break_stream(nsuMemory_p fd) override;

        /**
         *
         * @param chnl
         * @param fd
         * @param length
         * @param offset
         * @param stop_event
         * @param timeout
         * @param flag
         * @return
         */
        nsukitStatus_t
        stream_recv(nsuChnlNum_t chnl, nsuMemory_p fd, nsuStreamLen_t length, nsuStreamLen_t offset = 0,
                    bool(*stop_event) () = nullptr, float timeout = 5., int flag = 1) override;

        /**
         *
         * @param chnl
         * @param fd
         * @param length
         * @param offset
         * @param stop_event
         * @param timeout
         * @param flag
         * @return
         */
        nsukitStatus_t
        stream_send(nsuChnlNum_t chnl, nsuMemory_p fd, nsuStreamLen_t length, nsuStreamLen_t offset = 0,
                    bool(*stop_event) () = nullptr, float timeout = 5., int flag = 1) override;

         /**
         * @brief 直接写入目标FPGA的bypass空间
         * @param addr 地址（8字节对齐）
         * @param data 数据
         * @return 状态码
         */
        nsukitStatus_t direct_write(unsigned int addr, unsigned int data);

        /**
         * @brief 直接读取目标FPGA的bypass空间
         * @param addr 地址（8字节对齐）
         * @return 读取的数据
         */
        unsigned int direct_read(unsigned int addr);
    };
}

#endif //NSUKIT_FPGAPEER_INTERFACE_H
