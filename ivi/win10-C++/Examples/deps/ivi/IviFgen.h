#ifndef IVI_IVIFGEN_H
#define IVI_IVIFGEN_H

#include "IviBase.h"
#include "jsoncpp/json/json.h"

struct waveformHandle {
    void* dataHandle = nullptr;
    ViUInt32 idx = 0;
    ViConstString channelName = "0";
    ViUInt32 wfmSize = 0;
    ViUInt32 memOffset = 0;
    ViUInt32 writtenSize = 0;
    ViUInt32 wfmHandleNum = 0;
    ViBoolean isCreated = false;
};

class LRUCache {
private:
    std::unordered_map<uintptr_t, waveformHandle *> cacheMap;
public:
    waveformHandle * get( uintptr_t key);
    void put(uintptr_t key, waveformHandle * item);
    void remove(uintptr_t key);
    void update(uintptr_t key, waveformHandle * item);
    void clearCacheMap();
};

struct iviFgen_ViSession{
    iviBase_ViSession* vi{};
    std::mutex mtx;
    std::mutex mtx_config;
    ViUInt32 channelDataSize = 2147483648;
    ViUInt32 maxDataSize = 4294967295;
    ViUInt32 channelNumber = 2;
    bool is_locked = false;
    ViReal64 maxSampleRate = 10000000000.0;
    ViReal64 maxChannelSampleRate = 10000000000.0;
    std::map<uintptr_t, waveformHandle*> wfmHandles{};
    ViUInt32 channelEnabled = 0b11111111;
    std::map<ViUInt32, channelInfor> channelInforMap{};
    std::map<ViUInt32, channelXDMAInfor> channelXDMAInforMap{};
    ViUInt32 chanlNum;
    LRUCache cache;

    ViReal64 timeOut = 1.0;
    ViInt32 waitTimes = 3;
};

DLLEXTERN RIGOLLIB_API ViStatus IviFgen_Initialize (const ViString& logicalName, ViBoolean IDQuery, ViBoolean resetDevice, iviFgen_ViSession *vi, const ViString& resourceDBPath = "./resourceDB.json");
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_Reset (iviFgen_ViSession *vi);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_Close (iviFgen_ViSession *vi);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_RFConfig (iviFgen_ViSession *vi);

DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeViInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeViUInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeViReal64 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeViBoolean (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeViSession (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, iviFgen_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeViString (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, const ViString& attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeViInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeViUInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeViReal64 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeViBoolean (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeViSession (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, iviFgen_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeViString (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViString *attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviFgen_CheckAttributeViInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_CheckAttributeViUInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_CheckAttributeViReal64 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_CheckAttributeViBoolean (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_CheckAttributeViSession (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, iviFgen_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_CheckAttributeViString (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeRangeMaxValueViInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeRangeMaxValueViUInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeRangeMaxValueViReal64 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeRangeMaxValueViBoolean (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeRangeMaxValueViSession (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, iviFgen_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeRangeMaxValueViString (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeRangeMinValueViInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeRangeMinValueViUInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeRangeMinValueViReal64 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeRangeMinValueViBoolean (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeRangeMinValueViSession (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, iviFgen_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_GetAttributeRangeMinValueViString (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString *attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeRangeMaxValueViInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeRangeMaxValueViUInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeRangeMaxValueViReal64 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeRangeMaxValueViBoolean (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeRangeMaxValueViSession (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, iviFgen_ViSession attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeRangeMaxValueViString (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeRangeMinValueViInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeRangeMinValueViUInt32 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeRangeMinValueViReal64 (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeRangeMinValueViBoolean (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeRangeMinValueViSession (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, iviFgen_ViSession attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SetAttributeRangeMinValueViString (iviFgen_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviFgen_SendNSQC(iviFgen_ViSession *vi, ViChar *NSQCValue, ViInt32 NSQCLength, ViInt32 NSQCOffset = 0);

DLLEXTERN RIGOLLIB_API ViStatus IviFgen_InitiateGeneration (iviFgen_ViSession *vi);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_AbortGeneration (iviFgen_ViSession *vi);

DLLEXTERN RIGOLLIB_API ViStatus IviFgen_CreateArbWaveformViReal64 (iviFgen_ViSession *vi, ViInt32 waveformSize, ViReal64 *waveformDataArray, waveformHandle *wfmHandle);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_CreateArbWaveformViInt16 (iviFgen_ViSession *vi, ViInt32 waveformSize, ViInt16 *waveformDataArray, waveformHandle *wfmHandle);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_QueryArbWfmCapabilities (iviFgen_ViSession *vi, ViInt32 *maximumNumberofWaveforms, ViInt32 *waveformQuantum, ViInt32 *minimumWaveformSize, ViInt32 *maximumWaveformSize);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_ConfigureArbWfmMemOffset (iviFgen_ViSession *vi, waveformHandle *wfmHandle, ViUInt32 memOffset);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_ConfigureArbWaveformViReal64 (iviFgen_ViSession *vi, ViConstString channelName, waveformHandle *wfHandle, ViReal64 gain, ViReal64 offset);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_ConfigureArbWaveformViInt16 (iviFgen_ViSession *vi, ViConstString channelName, waveformHandle *wfHandle, ViReal64 gain, ViReal64 offset);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_ClearArbWaveform (iviFgen_ViSession *vi, waveformHandle *wfHandle);
DLLEXTERN RIGOLLIB_API ViStatus IviFgen_ClearAllArbWaveform (iviFgen_ViSession *vi);

#endif //IVI_IVIFGEN_H
