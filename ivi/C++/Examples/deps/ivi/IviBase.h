#ifndef IVI_IVIBASE_H
#define IVI_IVIBASE_H

#include "../exprtk/exprtk.hpp"
#include "type.h"
#include "configureNSUKit.h"
#include "IviATTR.h"
#include "IviVAL.h"

class AttrParam {
public:
    std::string attrName;
    std::string attrType;
    std::list<void *> value{};
    size_t data_length_{};
    std::string data_range_;
    ViInt32 channelSize{};

    AttrParam(std::string attrName, std::string attrType, const std::list<void *> &value, const size_t &data_length_, std::string data_range_, const ViInt32 &channelSize):
            attrName(std::move(attrName)), attrType(std::move(attrType)), value(value), data_length_(data_length_), data_range_(std::move(data_range_)), channelSize(channelSize){}
    ~AttrParam() = default;
};

struct actionGroup {
    std::string operation;
    std::string name;
    int channelNum;

    actionGroup(std::string operation, std::string name, int channelNum) : operation(std::move(operation)), name(std::move(name)), channelNum(channelNum) {}
    ~actionGroup() = default;
};

struct channelXDMAInfor{
    ViUInt32 channelOpenNum = 2;
    ViUInt32 channelNum = 2;
    channelXDMAInfor(ViUInt32 channelOpenNum, ViUInt32 channelNum): channelOpenNum(channelOpenNum), channelNum(channelNum) {}
    ~channelXDMAInfor() = default;
};

struct channelInfor {
    ViBoolean channelEnable = true;
    ViInt32 channelXDMA = 1;
    ViUInt32 channelLen = 2147483648;
    ViReal64 channelMAXSampleRate = 4000000000.0;
    channelInfor(ViBoolean channelEnable, ViInt32 channelXDMA, ViUInt32 channelLen, ViReal64 channelMAXSampleRate):
        channelEnable(channelEnable), channelXDMA(channelXDMA), channelLen(channelLen), channelMAXSampleRate(channelMAXSampleRate){}
    ~channelInfor() = default;
};

struct iviBase_ViSession{
    nsukit::BaseKit* kit{}; //
    nsuInitParam_t param{};
    ViString logicalName = "Sim";
    ViString name = "sim;sim;sim";
    ViString resourceDescriptr = "sim://0;sim://0;sim://0";
    ViString iviPath = "./ivi.json";
    ViString fpgaFunctionIdentification = "0";
    ViString functionIdentification = "0";
    ViString basicCapabilityIdentification = "0";
    ViString moduleModel = "0";

    std::string nameDS;
    std::map<std::string, std::string> DSItf_t{};
    std::string nameCR;
    std::map<std::string, std::string> CRItf_t{};
    std::string nameCS;
    std::map<std::string, std::string> CSItf_t{};

    std::map<ViAttr, AttrParam*> attrID_AttrParam{};
    std::map<ViString, ViAttr> attrName_attrID{};

    std::map<ViString, std::list<ViString>> setSeq_Action{};
    std::map<ViString, std::list<ViString>> getSeq_Action{};
    std::map<ViString, std::list<actionGroup>> action_Source{};
    std::map<ViString, ViString> action_Func{};
    std::map<ViString, actionGroup> action_Target{};

    ViBoolean isSim = false;
    std::map<ViString, ViString> attrName_SimAttr{};
    std::map<ViAttr, std::list<void *>> attrID_simAttrParam{};

    ViBoolean isRFConfigSuccess = true;

    ViUInt32 workModeAWG = 3;
    ViUInt32 workModeDAQ = 0;

    Json::Reader ICDReader;
    Json::Value icdRoot{};
    Json::Value *icdAttr;
    Json::Value *icdAction;
    Json::Value *icdSequence;
};


std::string extractName(const std::string& str);
ViStatus get_module(iviBase_ViSession* vi, const ViString& m_name, const ViString& path);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_Initialize (const ViString& logicalName, ViBoolean IDQuery, ViBoolean resetDevice, iviBase_ViSession *vi, const ViString& resourceDBPath = "./resourceDB.json");
ViStatus InitializeJSON (iviBase_ViSession *vi);
ViStatus InitializeAttrSim (iviBase_ViSession *vi);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_Reset (iviBase_ViSession *vi);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_Close (iviBase_ViSession *vi);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_RFConfig (iviBase_ViSession *vi);
DLLEXTERN RIGOLLIB_API ViAttr IviBase_queryAttrID (iviBase_ViSession *vi, const ViString& attrName);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SaveJson (iviBase_ViSession *vi, const ViString& iviSavePath="./ivi.json", const ViString& icdSavePath="./icd.json");

void* get_nth_element(const std::list<void*>& lst, int n);

void set_nth_element_ViInt32(std::list<void *>& lst, int n, ViInt32 value);
void set_nth_element_ViUInt32(std::list<void *>& lst, int n, ViUInt32 value);
void set_nth_element_ViReal64(std::list<void *>& lst, int n, ViReal64 value);
void set_nth_element_ViBoolean(std::list<void *>& lst, int n, ViBoolean value);
void set_nth_element_ViString(std::list<void *>& lst, int n, const ViString& value);

int InitJSON(std::string path, iviBase_ViSession *vi);
ViStatus ivi_attr2map(Json::Value* icdAttr, iviBase_ViSession *vi);
ViStatus ivi_action2map(Json::Value* icdAction, iviBase_ViSession *vi);
ViStatus ivi_sequence2map(Json::Value* icdSequence, iviBase_ViSession *vi);

typedef double eType; // numeric type (float, double, mpfr etc...)
ViStatus get_value(iviBase_ViSession *vi, const ViString& configurationName, const ViString& configurationValue, ViInt32 channel, eType *x);
ViStatus set_value(iviBase_ViSession *vi, const ViString& configurationName, const ViString& configurationValue, ViInt32 channel, eType result);
ViStatus iviAction(iviBase_ViSession *vi, ViInt32 channel, std::list<std::string> *sequence);

ViStatus judgmentChannel(ViAttr attributeID, AttrParam* attrParam, ViInt32 numChannel);

DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeViInt32(iviBase_ViSession *vi,  ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeViUInt32(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeViReal64(iviBase_ViSession *vi,  ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeViBoolean(iviBase_ViSession *vi,  ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeViSession(iviBase_ViSession *vi,  ViConstString channel, ViAttr attributeID, iviBase_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeViString(iviBase_ViSession *vi,  ViConstString channel, ViAttr attributeID, const ViString& attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeViInt32(iviBase_ViSession *vi,  ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeViUInt32(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeViReal64(iviBase_ViSession *vi,  ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeViBoolean(iviBase_ViSession *vi,  ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeViSession(iviBase_ViSession *vi,  ViConstString channel, ViAttr attributeID, iviBase_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeViString(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViString *attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviBase_CheckAttributeViInt32(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_CheckAttributeViUInt32(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_CheckAttributeViReal64(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_CheckAttributeViBoolean(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_CheckAttributeViSession(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, iviBase_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_CheckAttributeViString(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);

DLLEXTERN RIGOLLIB_API ViString error_message (ViStatus errCode);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_CompileTimeInfo();

DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeRangeMaxValueViInt32(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeRangeMaxValueViUInt32(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeRangeMaxValueViReal64(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeRangeMaxValueViBoolean(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeRangeMaxValueViSession(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, iviBase_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeRangeMaxValueViString(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeRangeMinValueViInt32(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeRangeMinValueViUInt32(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeRangeMinValueViReal64(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeRangeMinValueViBoolean(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeRangeMinValueViSession(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, iviBase_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetAttributeRangeMinValueViString(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString *attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeRangeMaxValueViInt32(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeRangeMaxValueViUInt32(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeRangeMaxValueViReal64(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeRangeMaxValueViBoolean(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeRangeMaxValueViSession(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, iviBase_ViSession* attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeRangeMaxValueViString(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeRangeMinValueViInt32(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeRangeMinValueViUInt32(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeRangeMinValueViReal64(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeRangeMinValueViBoolean(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeRangeMinValueViSession(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, iviBase_ViSession* attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetAttributeRangeMinValueViString(iviBase_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviBase_Sync(iviBase_ViSession *vi,  iviBase_ViSession **vis, ViInt32 slave_num, ViCharMem cmd_str);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SendStream(iviBase_ViSession *vi, ViConstString channel, ViChar *streamValue, ViInt32 streamLength, ViInt32 streamOffset);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_RecvStream(iviBase_ViSession *vi, ViConstString channel, ViChar *streamValue, ViInt32 streamLength, ViInt32 streamOffset);

DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetParamViUInt8(iviBase_ViSession *vi, ViString param_name,  ViUInt8 value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetParamViInt8(iviBase_ViSession *vi, ViString param_name,  ViInt8 value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetParamViUInt16(iviBase_ViSession *vi, ViString param_name,  ViUInt16 value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetParamViInt16(iviBase_ViSession *vi, ViString param_name,  ViInt16 value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetParamViUInt32(iviBase_ViSession *vi, ViString param_name,  ViUInt32 value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetParamViInt32(iviBase_ViSession *vi, ViString param_name,  ViInt32 value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetParamViReal32(iviBase_ViSession *vi, ViString param_name,  ViReal32 value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetParamViReal64(iviBase_ViSession *vi, ViString param_name,  ViReal64 value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_SetParamViString(iviBase_ViSession *vi, ViString param_name,  ViString value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetParamViUInt8(iviBase_ViSession *vi, ViString param_name,  ViUInt8* value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetParamViInt8(iviBase_ViSession *vi, ViString param_name,  ViInt8* value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetParamViUInt16(iviBase_ViSession *vi, ViString param_name,  ViUInt16* value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetParamViInt16(iviBase_ViSession *vi, ViString param_name,  ViInt16* value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetParamViUInt32(iviBase_ViSession *vi, ViString param_name,  ViUInt32* value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetParamViInt32(iviBase_ViSession *vi, ViString param_name,  ViInt32* value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetParamViReal32(iviBase_ViSession *vi, ViString param_name,  ViReal32* value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetParamViReal64(iviBase_ViSession *vi, ViString param_name,  ViReal64* value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_GetParamViString(iviBase_ViSession *vi, ViString param_name,  ViString* value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_Execute(iviBase_ViSession *vi, ViString cname);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_Write(iviBase_ViSession *vi, ViAttr addr, ViUInt32 value);
DLLEXTERN RIGOLLIB_API ViStatus IviBase_Read(iviBase_ViSession *vi, ViAttr addr, ViUInt32* value);

#endif //IVI_IVIBASE_H
