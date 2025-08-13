//
// Created by sn06129 on 2025/8/13.
//

# include "IviSUATools.h"
# include "IviFgen.h"
# include "tool_config.h"

int main(int argc, char *argv[]){

    auto iviSUATools_vi = new iviSUATools_ViSession;
    auto s = IviSUATools_Initialize(iviSUATools_vi);

    std::cout << "\n=== Creating Resource DB ===" << std::endl;
    std::string resource_db_path = IviSUATools_ScanOnlineBoards(iviSUATools_vi, "conf/config.yaml");
    std::cout << "BVC Manager initialized successfully. Resource DB: " << resource_db_path << std::endl;

    auto iviFgen_vi = new iviFgen_ViSession;
    s = IviFgen_Initialize("PXI::0::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviFgen_vi, resource_db_path);

    s = IviFgen_CheckAttributeViInt32 (iviFgen_vi, "0", IVIFGEN_ATTR_DAC_DUC_ENABLE, 15);
    std::cout << "IVIFGEN_ATTR_DAC_DUC_ENABLE: " << error_message(s) << std::endl;
    s = IviFgen_CheckAttributeViUInt32 (iviFgen_vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET, 1000000000);
    std::cout << "IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET: " << error_message(s) << std::endl;
    s = IviFgen_CheckAttributeViReal64 (iviFgen_vi, "0", IVIFGEN_ATTR_DAC_NCO_FREQUENCY, -700.000000);
    std::cout << "IVIFGEN_ATTR_DAC_NCO_FREQUENCY: " << error_message(s) << std::endl;

    s = IviFgen_Close(iviFgen_vi);
    s = IviSUATools_Close(iviSUATools_vi);

    delete iviSUATools_vi;
    delete iviFgen_vi;
    return 0;
}