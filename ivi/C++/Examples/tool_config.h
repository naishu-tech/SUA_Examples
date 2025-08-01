//
// Created by sn06129 on 2025/7/25.
//
#include <iostream>
#include <string>
#include <map>
#include "deps/ivi/IviFgen.h"
#include "deps/ivi/IviSUATools.h"

ViStatus DUCConfigAWG(iviFgen_ViSession *vi, ViInt32 DUC_Enable, ViConstString channelName, ViReal64 DUC_NCO_Frequency){
    auto s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_DAC_DUC_ENABLE, DUC_Enable);
    s = IviFgen_SetAttributeViReal64(vi, channelName, IVIFGEN_ATTR_DAC_NCO_FREQUENCY, DUC_NCO_Frequency);
    s = IviFgen_SetAttributeViInt32(vi, "0", IVIFGEN_ATTR_DAC_DUC_EXE, 1);
    ViUInt32 result = 0;
    s = IviFgen_GetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_DAC_DUC_RESULT, &result);
    std::cout << "IVIFGEN_ATTR_DAC_DUC_RESULT result is " << result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViStatus triggerConfigAWG(iviFgen_ViSession *vi,  ViUInt32 triggerSource){
    auto s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_TRIGGER_SOURCE, triggerSource);
    s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_TRIGGER_SOURCE_EXE, 0);

    ViUInt32 result = 0;
    s = IviFgen_GetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_TRIGGER_SOURCE_RESULT, &result);
    std::cout << "triggerConfigAWG result is " << result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViStatus internalTriggerConfigAWG(iviFgen_ViSession *vi, ViUInt32 triggerPulseWidth, ViUInt32 triggerRepeatSize, ViUInt32 triggerCycle, ViUInt32 triggerDelay){
    auto s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_INTERNAL_TRIGGER_PULSE_WIDTH, triggerPulseWidth);
    s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_INTERNAL_TRIGGER_REPEAT_SIZE, triggerRepeatSize);
    s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_INTERNAL_TRIGGER_PERIOD, triggerCycle);
    s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_INTERNAL_TRIGGER_DELAY, triggerDelay);
    s = IviFgen_SetAttributeViInt32(vi, "0", IVIFGEN_ATTR_INTERNAL_TRIGGER_EXE, 0);

    ViUInt32 result = 0;
    s = IviFgen_GetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_INTERNAL_TRIGGER_RESULT, &result);
    std::cout << "internalTriggerConfigAWG result is "<< result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }

}

ViStatus NYQ_ZONEAWG(iviFgen_ViSession *vi, ViConstString channelName, ViInt32 NYQ_ZONE){
    auto s = IviFgen_SetAttributeViUInt32(vi, channelName, IVIFGEN_ATTR_DAC_NYQ_ZONE, NYQ_ZONE);
    s = IviFgen_SetAttributeViInt32(vi, "0", IVIFGEN_ATTR_DAC_NYQ_ZONE_EXE, 0);

    ViUInt32 result = 0;
    s = IviFgen_GetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_DAC_NYQ_ZONE_RESULT, &result);
    std::cout << "NYQ_ZONEAWG result is "<< result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViStatus internalTriggerConfigSAT(iviSyncATrig_ViSession *vi, ViUInt32 triggerSource, ViUInt32 triggerPeriod = 6400, ViUInt32 triggerRepetSize=4294967295, ViUInt32 triggerPulseWidthy=1600){
    auto s = IviSyncATrig_SetAttributeViUInt32(vi, "0", IVISYNCATRIG_ATTR_TRIGGER_SOURCE, triggerSource);
    s = IviSyncATrig_SetAttributeViUInt32(vi, "0", IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_PERIOD, triggerPeriod);
    s = IviSyncATrig_SetAttributeViUInt32(vi, "0", IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_REPEAT_SIZE, triggerRepetSize);
    s = IviSyncATrig_SetAttributeViUInt32(vi, "0", IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_PULSE_WIDTH, triggerPulseWidthy);
    ViUInt32 result = 0;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViString create_nswave_code(std::map<ViString, waveformHandle *> waveformHandle_map ,ViInt32 nswaveType, ViUInt32 triggerPeriod = 6400){
    ViString nsqc = R"(@nw.kernel
def program(wlist: dict[str, np.ndarray]):
)";

    int i = 0;
    ViString seg = "seg0";

    switch (nswaveType) {
        case 1:
            for (const auto& pair : waveformHandle_map) {
                seg = pair.first;
                nsqc += "    seg" + std::to_string(i) + ": nw.ArbWave = nw.init_arbwave(wlist, '" + seg + "')\n";
                i++;
            }

            for (int j = 0; j < i; j++) {
                nsqc += "    nw.wait_for_trigger_with_source(1)  # Wait for trigger\n";
                nsqc += "    nw.play_arb(seg" + std::to_string(j) + ")  # Play segment\n";
            }
            break;
        case 2:
            for (const auto& pair : waveformHandle_map) {
                seg = pair.first;
                nsqc += "    seg" + std::to_string(i) + ": nw.ArbWave = nw.init_arbwave(wlist, '" + seg + "')\n";
                i++;
            }

            nsqc += "    while True:\n";

            for (int j = 0; j < i; j++) {
                nsqc += "        nw.wait_for_trigger_with_source(1)  # Wait for trigger\n";
                nsqc += "        nw.wait(0.001)  # Delay one-tenth of the cycle\n";
                nsqc += "        nw.play_arb(seg" + std::to_string(j) + ")  # Play segment\n";
            }
            break;
        case 3:
            for (const auto& pair : waveformHandle_map) {
                seg = pair.first;
                nsqc += "    seg" + std::to_string(i) + ": nw.ArbWave = nw.init_arbwave(wlist, '" + seg + "')\n";
                i++;
            }
            nsqc += "    i:nw.Reg\n";
            for (int j = 0; j < i; j++) {
                nsqc += "    for i in nw.loop(10):  # Loop iteration\n";
                nsqc += "        nw.wait_for_trigger_with_source(1)  # Wait for trigger\n";
                nsqc += "        nw.play_arb(seg" + std::to_string(j) + ")  # Play segment\n";
            }
            break;
        case 4:
            for (const auto& pair : waveformHandle_map) {
                seg = pair.first;
                nsqc += "    seg" + std::to_string(i) + ": nw.ArbWave = nw.init_arbwave(wlist, '" + seg + "')\n";
                i++;
            }
            nsqc += "    i:nw.Reg\n";
            nsqc += "    while True:\n";

            for (int j = 0; j < i; j++) {
                nsqc += "        for i in nw.loop(10):  # Loop iteration\n";
                nsqc += "            nw.wait_for_trigger_with_source(1)  # Wait for trigger\n";
                nsqc += "            nw.play_arb(seg" + std::to_string(j) + ")  # Play segment\n";
            }
            break;
        default:
            for (const auto& pair : waveformHandle_map) {
                seg = pair.first;
                nsqc += "    seg" + std::to_string(i) + ": nw.ArbWave = nw.init_arbwave(wlist, '" + seg + "')\n";
                i++;
            }

            nsqc += "    while True:\n";

            for (int j = 0; j < i; j++) {
                nsqc += "        nw.play_arb(seg" + std::to_string(j) + ")  # Play segment\n";
            }
            break;
    }

    nsqc += "    return nw.Kernel()\n";
    
    return nsqc;
}

ViStatus DDConfigDAQ(iviDigitizer_ViSession *vi, ViInt32 DDC_Enable, ViConstString channelName, ViReal64 DDC_NCO_Frequency){

    auto s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_ADC_DDC_ENABLE, DDC_Enable);
    s = IviDigitizer_SetAttributeViReal64(vi, channelName, IVIDIGITIZER_ATTR_ADC_NCO_FREQUENCY, DDC_NCO_Frequency);
    s = IviDigitizer_SetAttributeViInt32(vi, "0", IVIDIGITIZER_ATTR_ADC_DDC_EXE, 1);
    ViUInt32 result;
    s = IviDigitizer_GetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_ADC_DDC_RESULT, &result);
    std::cout <<"DDConfigDAQ  result is " << result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViStatus internalTriggerConfigDAQ(iviDigitizer_ViSession *vi, ViConstString channelName, ViInt32 triggerEdgeSet, ViUInt32 triggerPulseWidth=100000000, ViUInt32 triggerRepeatSize=4294967295, ViUInt32 triggerCycle=1000000000){

    auto s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_PULSE_WIDTH, triggerPulseWidth);
    s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_REPEAT_SIZE, triggerRepeatSize);
    s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_PERIOD, triggerCycle);
    s = IviDigitizer_SetAttributeViUInt32(vi, channelName, IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET, triggerEdgeSet);
    s = IviDigitizer_SetAttributeViInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_CONFIG_TRIGGER_EXECUTE, 0);

    ViUInt32 result;
    s = IviDigitizer_GetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_RESULT, &result);
    std::cout << "internalTriggerConfigDAQ result is " << result << std::endl;

    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViStatus NYQ_ZONEDAQ(iviDigitizer_ViSession *vi, ViConstString channelName, ViInt32 NYQ_ZONE){

    auto s = IviDigitizer_SetAttributeViUInt32(vi, channelName, IVIDIGITIZER_ATTR_ADC_NYQ_ZONE, NYQ_ZONE);
    s = IviDigitizer_SetAttributeViInt32(vi, "0", IVIDIGITIZER_ATTR_ADC_NYQ_ZONE_EXE, 0);

    ViUInt32 result;
    s = IviDigitizer_GetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_ADC_NYQ_ZONE_RESULT, &result);
    std::cout << "NYQ_ZONEDAQ result is" << result << std::endl;

    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViStatus sampleConfigDAQ(iviDigitizer_ViSession *vi, ViConstString sampleEnableChannel, ViInt32 sampleEnable,
                     ViConstString sampleStrageDepthChannel, ViUInt32 sampleStrageDepth,
                     ViConstString sampleLenPreChannel, ViUInt32 sampleLenPre,
                     ViConstString sampleTimesChannel, ViUInt32 sampleTimes,
                     ViConstString sampleLogicalExtractionMultipleChannel, ViUInt32 sampleLogicalExtractionMultiple,
                     ViConstString sampleCollectDataTruncationChannel, ViUInt32 sampleCollectDataTruncation,
                     ViConstString sampleCollectDataTypeChannel, ViUInt32 sampleCollectDataType){

    auto s = IviDigitizer_SetAttributeViUInt32(vi, sampleEnableChannel, IVIDIGITIZER_ATTR_SAMPLE_ENABLE, sampleEnable);
    s = IviDigitizer_SetAttributeViUInt32(vi, sampleStrageDepthChannel, IVIDIGITIZER_ATTR_SAMPLE_STRAGE_DEPTH, sampleStrageDepth);
    s = IviDigitizer_SetAttributeViUInt32(vi, sampleLenPreChannel, IVIDIGITIZER_ATTR_SAMPLE_POINT_PRE, sampleLenPre);
    s = IviDigitizer_SetAttributeViUInt32(vi, sampleTimesChannel, IVIDIGITIZER_ATTR_SAMPLE_TIMES, sampleTimes);
    s = IviDigitizer_SetAttributeViUInt32(vi, sampleLogicalExtractionMultipleChannel, IVIDIGITIZER_ATTR_SAMPLE_LOGICAL_EXTRACTION_MULTIPLE, sampleLogicalExtractionMultiple);
    s = IviDigitizer_SetAttributeViUInt32(vi, sampleCollectDataTruncationChannel, IVIDIGITIZER_ATTR_SAMPLE_COLLECT_DATA_TRUNCATION, sampleCollectDataTruncation);
    s = IviDigitizer_SetAttributeViUInt32(vi, sampleCollectDataTypeChannel, IVIDIGITIZER_ATTR_SAMPLE_COLLECT_DATA_TYPE, sampleCollectDataType);
    s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_SAMPLE_CONFIG_EXECUTE, 0);

    ViUInt32 result;
    s = IviDigitizer_GetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_SAMPLE_CONFIG_RESULT, &result);
    std::cout << "sampleConfigDAQ result is " << result << std::endl;

    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViStatus triggerConfigDAQ(iviDigitizer_ViSession *vi, ViUInt32 triggerSource){
    auto s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_TRIGGER_SOURCE, triggerSource);
    s = IviDigitizer_SetAttributeViInt32(vi, "0", IVIDIGITIZER_ATTR_TRIGGER_CONFIG_EXECUTE, 0);

    ViUInt32 result;
    s = IviDigitizer_GetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_TRIGGER_SOURCE_RESULT, &result);
    std::cout << "triggerConfigDAQ result is " << result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}