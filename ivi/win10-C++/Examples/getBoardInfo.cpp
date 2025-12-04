# include "iostream"
# include "IviBase.h"
# include "tool_config.h"
# include "../deps/jsoncpp/json/json.h"


int main(int argc, char *argv[]){

//    if (argc != 2) {
//        std::cout << "Unsupported parameter passing method" << std::endl;
//        // sudo ./RF Sim
//        // sudo ./RF XDMA
//        std::cout << argv[0] << " {logicalName}" << std::endl;
//        return 1;
//    }

    std::string resource_db_path = "./resourceDB.json";
    std::string logicalName = "PXI::0::INSTR";

    auto vi = new iviBase_ViSession;
    std::cout << "logicalName: " << logicalName << std::endl;
    auto s = IviBase_Initialize(logicalName, VI_STATE_FALSE, VI_STATE_TRUE, vi, resource_db_path);
    std::cout << "IviBase_Initialize: " << s << std::endl;

    ViUInt32 m;
    s = IviBase_GetAttributeViUInt32(vi, "0", IVIBASE_ATTR_FPGA_FUNCTION_IDENTIFICATION, &m);
    std::cout <<"IVIBASE_ATTR_FPGA_FUNCTION_IDENTIFICATION: "<< m << std::endl;
    s = IviBase_GetAttributeViUInt32(vi, "0", IVIBASE_ATTR_MODULE_DEVICE_ID, &m);
    std::cout <<"IVIBASE_ATTR_MODULE_DEVICE_ID: "<< m << std::endl;
    s = IviBase_GetAttributeViUInt32(vi, "0", IVIBASE_ATTR_FPGA_FIRMWARE_VERSION, &m);
    std::cout <<"IVIBASE_ATTR_FPGA_FIRMWARE_VERSION: "<< m << std::endl;
    s = IviBase_GetAttributeViUInt32(vi, "0", IVIBASE_ATTR_BASIC_CAPABILITY_IDENTIFICATION, &m);
    std::cout <<"IVIBASE_ATTR_BASIC_CAPABILITY_IDENTIFICATION: "<< m << std::endl;

    s = IviBase_Close(vi);

    return 0;

}