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

    std::list<iviFgen_ViSession *> iviFgen_vi_list;
    std::list<iviDigitizer_ViSession *> iviDigitizer_vi_list;

    iviFgen_vi_list.push_back(iviFgen_vi);
    iviDigitizer_vi_list.push_back(iviDigitizer_vi);

    s = IviSUATools_Sync(iviSUATools_vi, iviSyncATrig_vi, iviFgen_vi_list, iviDigitizer_vi_list);

    isFAIL(IviFgen_Close(iviFgen_vi));
    isFAIL(IviDigitizer_Close(iviDigitizer_vi));
    isFAIL(IviSyncATrig_Close(iviSyncATrig_vi));
    isFAIL(IviSUATools_Close(iviSUATools_vi));

    delete iviSUATools_vi;
    delete iviFgen_vi;
    delete iviDigitizer_vi;
    delete iviSyncATrig_vi;

    return 0;
} 