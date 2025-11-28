# include "iostream"
# include "IviFgen.h"
# include "IviDigitizer.h"
# include "jsoncpp/json/json.h"
# include "IviBase.h"
# include "IviSyncATrig.h"

int main(int argc, char *argv[]){

//    if (argc != 2) {
//        std::cout << "Unsupported parameter passing method" << std::endl;
//        // sudo ./RF Sim
//        // sudo ./RF XDMA
//        std::cout << argv[0] << " {logicalName}" << std::endl;
//        return 1;
//    }
//
//    std::string logicalName = argv[1];
    std::string logicalName = "Sim";

    auto vi = new iviBase_ViSession;
    std::cout << "logicalName: " << logicalName << std::endl;
    auto s = IviBase_Initialize(logicalName, VI_STATE_FALSE, VI_STATE_TRUE, vi);
    std::cout << "IviBase_Initialize: " << s << std::endl;

    s = IviBase_SetAttributeViUInt32(vi, "0", IVIBASE_ATTR_OFFLINE_WORK, 1);
    s = IviBase_SetAttributeViInt32(vi, "0", IVIBASE_ATTR_RF_CONFIG, 0);
    ViString m;
    s = IviBase_GetAttributeViString(vi, "0", IVIBASE_ATTR_MODULE_VENDOR_ID_STR, &m);
    std::cout <<"IVIBASE_ATTR_MODULE_VENDOR_ID_STR: "<< m << std::endl;
    s = IviBase_GetAttributeViString(vi, "0", IVIBASE_ATTR_MODULE_DEVICE_ID_STR, &m);
    std::cout <<"IVIBASE_ATTR_MODULE_DEVICE_ID_STR: "<< m << std::endl;
    s = IviBase_GetAttributeViString(vi, "0", IVIBASE_ATTR_FPGA_VERSION_STR, &m);
    std::cout <<"IVIBASE_ATTR_FPGA_VERSION_STR: "<< m << std::endl;

    s = IviBase_GetAttributeViString(vi, "0", IVIBASE_ATTR_COMPILE_DATE_INFO, &m);
    std::cout <<"IVIBASE_ATTR_COMPILE_DATE_INFO: "<< m << std::endl;
    s = IviBase_GetAttributeViString(vi, "0", IVIBASE_ATTR_COMPILE_TIME_INFO, &m);
    std::cout <<"IVIBASE_ATTR_COMPILE_TIME_INFO: "<< m << std::endl;

//    IviBase_CompileTimeInfo();

    s = IviBase_Close(vi);
    return 0;

}