#ifndef IVI_IVISYNCATRIG_H
#define IVI_IVISYNCATRIG_H

#include "IviBase.h"
#include "IviFgen.h"
#include "IviDigitizer.h"

struct iviSyncATrig_ViSession{
    iviBase_ViSession* vi{};
};

DLLEXTERN MYLIB_API ViStatus IviSyncATrig_Initialize (const ViString& logicalName, ViBoolean IDQuery, ViBoolean resetDevice, iviSyncATrig_ViSession *vi, const ViString& resourceDBPath = "./resourceDB.json");
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_Reset (iviSyncATrig_ViSession *vi);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_Close (iviSyncATrig_ViSession *vi);

DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeViInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeViUInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeViReal64 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeViBoolean (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeViSession (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, iviSyncATrig_ViSession *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeViString (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViString attributeValue);
//MYLIB_API ViStatus IviSync_SetAttribute (ViSession *vi, ViConstString channel, ViAttr attributeID, waveformHandle *attributeValue);

DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeViInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeViUInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeViReal64 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeViBoolean (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeViSession (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, iviSyncATrig_ViSession *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeViString (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViString *attributeValue);

DLLEXTERN MYLIB_API ViStatus IviSyncATrig_CheckAttributeViInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_CheckAttributeViUInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_CheckAttributeViReal64 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_CheckAttributeViBoolean (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_CheckAttributeViSession (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, iviSyncATrig_ViSession *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_CheckAttributeViString (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);

DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeRangeMaxValueViInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeRangeMaxValueViUInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeRangeMaxValueViReal64 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeRangeMaxValueViBoolean (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeRangeMaxValueViSession (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, iviSyncATrig_ViSession *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeRangeMaxValueViString (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeRangeMinValueViInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeRangeMinValueViUInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeRangeMinValueViReal64 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeRangeMinValueViBoolean (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeRangeMinValueViSession (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, iviSyncATrig_ViSession *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_GetAttributeRangeMinValueViString (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString *attributeValue);

DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeRangeMaxValueViInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeRangeMaxValueViUInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeRangeMaxValueViReal64 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeRangeMaxValueViBoolean (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeRangeMaxValueViSession (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, iviSyncATrig_ViSession* attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeRangeMaxValueViString (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeRangeMinValueViInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeRangeMinValueViUInt32 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeRangeMinValueViReal64 (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeRangeMinValueViBoolean (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeRangeMinValueViSession (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, iviSyncATrig_ViSession* attributeValue);
DLLEXTERN MYLIB_API ViStatus IviSyncATrig_SetAttributeRangeMinValueViString (iviSyncATrig_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);

#endif //IVI_IVISYNCATRIG_H