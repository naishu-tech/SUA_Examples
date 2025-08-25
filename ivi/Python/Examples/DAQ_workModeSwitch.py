//
// Created by sn06129 on 2025/7/25.
//

# include "iostream"
# include "IviSUATools.h"
# include "IviDigitizer.h"
# include "tool_config.h"

int main(int argc, char *argv[]){

    std::map<ViString, ViUInt32> workModeSwitch = {
            {"Stream", 287449856}, //"0x11222300"
            {"RingBuffer", 287441664} //"0x11220300"
    };

    auto iviSUATools_vi = new iviSUATools_ViSession;
    auto s = IviSUATools_Initialize(iviSUATools_vi);

    std::cout << "\n=== Creating Resource DB ===" << std::endl;
    std::string resource_db_path = IviSUATools_ScanOnlineBoards(iviSUATools_vi, "conf/config.yaml");
    std::cout << "BVC Manager initialized successfully. Resource DB: " << resource_db_path << std::endl;

    auto iviDigitizer_vi = new iviDigitizer_ViSession;
    s = IviDigitizer_Initialize("PXI::0::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviDigitizer_vi, resource_db_path);

    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_BIT_SWITCH_FILE_NAME, workModeSwitch["Stream"]);
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_BIT_SWITCH_EXE, 0);

    ViUInt32 result;
    s = IviDigitizer_GetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_BIT_SWITCH_RESULT, &result);
    std::cout << "workModeSwitch result is " << result << std::endl;

    if (result){
        return VI_STATE_FAIL;
    } else {
        std::cout << "Work Mode Switch Successfully, Please modprobe XDMA" << std::endl;
        return VI_STATE_SUCCESS;
    }

    s = IviDigitizer_Close(iviDigitizer_vi);
    s = IviSUATools_Close(iviSUATools_vi);

    delete iviSUATools_vi;
    delete iviDigitizer_vi;
    return 0;

}