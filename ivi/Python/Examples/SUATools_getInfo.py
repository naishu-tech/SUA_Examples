//
// Created by sn06129 on 2025/7/25.
//

#include <iostream>
#include <string>
#include "IviSUATools.h"

int main() {

    auto iviSUATools_vi = new iviSUATools_ViSession;
    auto s = IviSUATools_Initialize(iviSUATools_vi);

    std::string resource_db_path = IviSUATools_ScanOnlineBoards(iviSUATools_vi, "conf/config.yaml");
    std::cout << "BVC Manager initialized successfully. Resource DB: " << resource_db_path << std::endl;

    auto iviFgen_vi = new iviFgen_ViSession;
    s = IviFgen_Initialize("PXI::0::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviFgen_vi, resource_db_path);

    auto iviDigitizer_vi = new iviDigitizer_ViSession;
    s = IviDigitizer_Initialize("PXI::0::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviDigitizer_vi, resource_db_path);

    auto iviSyncATrig_vi = new iviSyncATrig_ViSession;
    s = IviSyncATrig_Initialize("PXI::1::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviSyncATrig_vi, resource_db_path);

    auto iviPXIMainBoard_vi = new iviPXIMainBoard_ViSession;
    s = IviPXIMainBoard_Initialize("PXI::1::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviPXIMainBoard_vi, resource_db_path);

    s = IviSUATools_GetInfoFgen(iviSUATools_vi, iviFgen_vi);

    std::cout << "\n\nIviFgen info:"
          <<"\nfpga_version: " << iviSUATools_vi->infoFgen.fpga_version
          <<"\nfpga_hardware_version: " << iviSUATools_vi->infoFgen.fpga_hardware_version
          <<"\nfpga_temper: " << iviSUATools_vi->infoFgen.fpga_temper
          <<"\ncpu_temper: " << iviSUATools_vi->infoFgen.cpu_temper
          <<"\nstatus: " << iviSUATools_vi->infoFgen.status
          <<"\nusage: " << iviSUATools_vi->infoFgen.usage
          <<"\ncontroler_memory_total: " << iviSUATools_vi->infoFgen.controler_memory_total
          <<"\ncontroler_memory_usage: " << iviSUATools_vi->infoFgen.controler_memory_usage
          <<"\nivi_version: " << iviSUATools_vi->infoFgen.ivi_version
    << std::endl;

    s = IviSUATools_GetInfoDigitizer(iviSUATools_vi, iviDigitizer_vi);

    std::cout << "\n\nIviDigitizer_vi info:"
          <<"\nfpga_version: " << iviSUATools_vi->infoDigitizer.fpga_version
          <<"\nfpga_hardware_version: " << iviSUATools_vi->infoDigitizer.fpga_hardware_version
          <<"\nfpga_temper: " << iviSUATools_vi->infoDigitizer.fpga_temper
          <<"\ncpu_temper: " << iviSUATools_vi->infoDigitizer.cpu_temper
          <<"\nstatus: " << iviSUATools_vi->infoDigitizer.status
          <<"\nusage: " << iviSUATools_vi->infoDigitizer.usage
          <<"\ncontroler_memory_total: " << iviSUATools_vi->infoDigitizer.controler_memory_total
          <<"\ncontroler_memory_usage: " << iviSUATools_vi->infoDigitizer.controler_memory_usage
          <<"\nivi_version: " << iviSUATools_vi->infoDigitizer.ivi_version
    << std::endl;

    s = IviSUATools_GetInfoSyncATrig(iviSUATools_vi, iviSyncATrig_vi);
    std::cout << "\n\nIviSyncATrig info:"
          <<"\nfpga_version: " << iviSUATools_vi->infoSyncATrig.fpga_version
          <<"\nfpga_hardware_version: " << iviSUATools_vi->infoSyncATrig.fpga_hardware_version
          <<"\nfpga_temper: " << iviSUATools_vi->infoSyncATrig.fpga_temper
          <<"\ncpu_temper: " << iviSUATools_vi->infoSyncATrig.cpu_temper
          <<"\nstatus: " << iviSUATools_vi->infoSyncATrig.status
          <<"\nusage: " << iviSUATools_vi->infoSyncATrig.usage
          <<"\ncontroler_memory_total: " << iviSUATools_vi->infoSyncATrig.controler_memory_total
          <<"\ncontroler_memory_usage: " << iviSUATools_vi->infoSyncATrig.controler_memory_usage
          <<"\nivi_version: " << iviSUATools_vi->infoSyncATrig.ivi_version
    << std::endl;


    s = IviSUATools_GetInfoPXIMainBoard(iviSUATools_vi, iviPXIMainBoard_vi);
    std::cout << "\n\nIviPXIMainBoard info:"
          <<"\nfpga" << iviSUATools_vi->infoPXIMainBoard.fpga_version
          <<"\nfpga_hardware_version: " << iviSUATools_vi->infoPXIMainBoard.fpga_hardware_version
          <<"\nfpga_temper: " << iviSUATools_vi->infoPXIMainBoard.fpga_temper
          <<"\ncpu_temper: " << iviSUATools_vi->infoPXIMainBoard.cpu_temper
          <<"\nstatus: " << iviSUATools_vi->infoPXIMainBoard.status
          <<"\nusage: " << iviSUATools_vi->infoPXIMainBoard.usage
          <<"\ncontroler_memory_total: " << iviSUATools_vi->infoPXIMainBoard.controler_memory_total
          <<"\ncontroler_memory_usage: " << iviSUATools_vi->infoPXIMainBoard.controler_memory_usage
          <<"\nivi_version: " << iviSUATools_vi->infoPXIMainBoard.ivi_version
    << std::endl;

    isFAIL(IviFgen_Close(iviFgen_vi));
    isFAIL(IviDigitizer_Close(iviDigitizer_vi));
    isFAIL(IviSyncATrig_Close(iviSyncATrig_vi));
    isFAIL(IviPXIMainBoard_Close(iviPXIMainBoard_vi));
    isFAIL(IviSUATools_Close(iviSUATools_vi));

    delete iviSUATools_vi;
    delete iviFgen_vi;
    delete iviDigitizer_vi;
    delete iviSyncATrig_vi;
    delete iviPXIMainBoard_vi;

    return 0;
} 