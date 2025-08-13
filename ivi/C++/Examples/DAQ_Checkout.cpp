//
// Created by sn06129 on 2025/8/13.
//

# include "IviSUATools.h"
# include "IviDigitizer.h"
# include "tool_config.h"

int main(int argc, char *argv[]){

    auto iviSUATools_vi = new iviSUATools_ViSession;
    auto s = IviSUATools_Initialize(iviSUATools_vi);

    std::cout << "\n=== Creating Resource DB ===" << std::endl;
    std::string resource_db_path = IviSUATools_ScanOnlineBoards(iviSUATools_vi, "conf/config.yaml");
    std::cout << "BVC Manager initialized successfully. Resource DB: " << resource_db_path << std::endl;

    auto iviDigitizer_vi = new iviDigitizer_ViSession;
    s = IviDigitizer_Initialize("PXI::0::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviDigitizer_vi, resource_db_path);

    s = IviDigitizer_CheckAttributeViInt32 (iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_ADC_DDC_ENABLE, 15);
    std::cout << "IVIDIGITIZER_ATTR_ADC_DDC_ENABLE: " << error_message(s) << std::endl;
    s = IviDigitizer_CheckAttributeViUInt32 (iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET, 1000000000);
    std::cout << "IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET: " << error_message(s) << std::endl;
    s = IviDigitizer_CheckAttributeViReal64 (iviDigitizer_vi, "0", IVIFGEN_ATTR_DAC_NCO_FREQUENCY, -700.000000);
    std::cout << "IVIFGEN_ATTR_DAC_NCO_FREQUENCY: " << error_message(s) << std::endl;

    s = IviDigitizer_Close(iviDigitizer_vi);
    s = IviSUATools_Close(iviSUATools_vi);

    delete iviSUATools_vi;
    delete iviDigitizer_vi;
    return 0;
}