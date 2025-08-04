#ifndef IVI_IVIPXIMAINBOARD_H
#define IVI_IVIPXIMAINBOARD_H

#include "IviBase.h"

struct iviPXIMainBoard_ViSession{
    iviBase_ViSession* vi{};
};

DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_Initialize (const ViString& logicalName, ViBoolean IDQuery, ViBoolean resetDevice, iviPXIMainBoard_ViSession *vi, const ViString& resourceDBPath = "./resourceDB.json");
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_Reset (iviPXIMainBoard_ViSession *vi);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_Close (iviPXIMainBoard_ViSession *vi);

DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeViInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeViUInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeViReal64 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeViBoolean (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeViSession (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, iviPXIMainBoard_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeViString (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViString attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeViInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeViUInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeViReal64 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeViBoolean (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeViSession (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, iviPXIMainBoard_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeViString (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViString *attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_CheckAttributeViInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_CheckAttributeViUInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_CheckAttributeViReal64 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_CheckAttributeViBoolean (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_CheckAttributeViSession (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, iviPXIMainBoard_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_CheckAttributeViString (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeRangeMaxValueViInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeRangeMaxValueViUInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeRangeMaxValueViReal64 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeRangeMaxValueViBoolean (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeRangeMaxValueViSession (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, iviPXIMainBoard_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeRangeMaxValueViString (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeRangeMinValueViInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeRangeMinValueViUInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeRangeMinValueViReal64 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeRangeMinValueViBoolean (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeRangeMinValueViSession (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, iviPXIMainBoard_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_GetAttributeRangeMinValueViString (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString *attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeRangeMaxValueViInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeRangeMaxValueViUInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeRangeMaxValueViReal64 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeRangeMaxValueViBoolean (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeRangeMaxValueViSession (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, iviPXIMainBoard_ViSession* attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeRangeMaxValueViString (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeRangeMinValueViInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeRangeMinValueViUInt32 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeRangeMinValueViReal64 (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeRangeMinValueViBoolean (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeRangeMinValueViSession (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, iviPXIMainBoard_ViSession* attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviPXIMainBoard_SetAttributeRangeMinValueViString (iviPXIMainBoard_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);

#endif //IVI_IVIPXIMAINBOARD_H
