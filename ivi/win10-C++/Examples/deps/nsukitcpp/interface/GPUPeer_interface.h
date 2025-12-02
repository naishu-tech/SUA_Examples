//
// Created by sn06129 on 2024/12/5.
//

#ifndef NSUKIT_GPUPEER_INTERFACE_H
#define NSUKIT_GPUPEER_INTERFACE_H

#include "base_itf.h"
#include "xdma_api.h"

namespace nsukit {
    class GPU_P2P_XDMAOperation_t {
    protected:
        std::map<int, int> openedBoard = {};
        std::mutex OpLock{};

    public:
        /**
         * 开启板卡，并记录开启次数，只有第一次真正开启板卡
         * @param board xdma板卡号
         * @return
         */
        nsukitStatus_t GPU_P2P_open_xdma_board(int board);

        /**
         * 关闭板卡并记录关闭次数，只有最后一次真正关闭
         * @param board xdma板卡号
         * @return
         */
        nsukitStatus_t GPU_P2P_close_xdma_board(int board);
    };

    class NSU_DLLEXPORT GPUPeerStreamUItf : public I_BaseStreamUItf {
    private:
        struct StreamProcess {
            nsuBoardNum_t FPGA_board; // FPGA 号
            nsuBoardNum_t GPU_board; // GPU 号
            nsuStreamLen_t current = 0;
            nsuStreamLen_t total = 0;
            void* gpu_device_ptr = nullptr; // GPU设备内存指针
            unsigned long long gpu_phys_addr = 0; // GPU物理地址
            bool gpu_allocated = false; // 是否由我们分配的GPU内存
            HANDLE dma; // DMA句柄
            int fdLength; //记录申请的fd长度
        };

        nsuBoardNum_t pciBoard = 0;
        nsuBoardNum_t gpuBoard = 0;

#ifdef CUDA_ENABLED
        CUdevice cuDevice;
        CUcontext cuContext = nullptr;
#endif


        const uint8_t byteWidth = 4;
        const uint8_t maxChnl = 4;
    protected:
                struct StreamProcess {
            nsuBoardNum_t FPGA_board; // FPGA 号
            nsuBoardNum_t GPU_board; // GPU 号
            nsuStreamLen_t current = 0;
            nsuStreamLen_t total = 0;
            int* cuda_buffer; // cuda缓存
            HANDLE dma; // DMA句柄
            int fdLength; //记录申请的fd长度
        };

        nsuBoardNum_t pciBoard = 0;

        std::map<nsuMemory_p, StreamProcess> uploadProcess{};

        std::map<nsuMemory_p, StreamProcess> downloadProcess{};
    public:
        /**
         *
         */
        GPUPeerStreamUItf();

        ~GPUPeerStreamUItf() override = default;

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
    };
}

#endif //NSUKIT_GPUPEER_INTERFACE_H