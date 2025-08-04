/**
 * @file cuda_signal_processing_simple.cu
 * @brief CUDA IVI signal processing example
 * 
 * This example demonstrates how to use CUDA for basic signal generation, 
 * combined with IVI interfaces to implement signal download, playback, and acquisition.
 * Simplified features:
 * - CUDA GPU parallel generation of real waveform
 * - IviFgen signal generator control and waveform download
 * - IviDigitizer digitizer data acquisition
 * - Acquired data saved directly to file
 * 
 * @author: jilianyi@rigol.com
 * @date: 2025
 */

// ===============================================================================
// system and standard library headers
// ===============================================================================
#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <fstream>
#include <cmath>

// ===============================================================================
// CUDA related headers
// ===============================================================================
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <curand_kernel.h>

// ===============================================================================
// IVI interface headers
// ===============================================================================
#include "IviFgen.h"
#include "IviDigitizer.h"
#include "IviSUATools.h"
#include "IviSyncATrig.h"

// ===============================================================================
// error check macro definitions
// ===============================================================================
#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            std::cerr << "CUDA error at " << __FILE__ << ":" << __LINE__ << " - " << cudaGetErrorString(err) << std::endl; \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

#define IVI_CHECK(call) \
    do { \
        ViStatus status = call; \
        if (status != VI_STATE_SUCCESS) { \
            std::cerr << "IVI error at " << __FILE__ << ":" << __LINE__ << " - Status: " << status << std::endl; \
            return status; \
        } \
    } while(0)


#define DIGITIZER_RESOURCE_NAME "PXI::0::INSTR"
#define FGEN_RESOURCE_NAME "PXI::0::INSTR"
#define SYNC_ATRIG_RESOURCE_NAME "PXI::1::INSTR"

// ===============================================================================
// constant definitions
// ===============================================================================
constexpr int DEFAULT_WAVEFORM_SIZE = 8192000;  // 8M samples
constexpr double DEFAULT_SAMPLE_RATE = 4000000000.0;  // 4 GSPS
constexpr double DEFAULT_FREQUENCY = 100000000.0;  // 100 MHz基础频率
constexpr int THREADS_PER_BLOCK = 256;

// ===============================================================================
// CUDA device functions - simplified signal generation kernel functions
// ===============================================================================

/**
 * @brief CUDA kernel function: directly generate real waveform signal
 * @param d_waveform_data output waveform data buffer
 * @param num_samples number of samples
 * @param sample_rate sample rate
 * @param base_freq base frequency
 * @param amplitude signal amplitude
 * @param noise_power noise power
 */
__global__ void generate_real_waveform(float* d_waveform_data, 
                                      int num_samples,
                                      double sample_rate,
                                      double base_freq,
                                      float amplitude,
                                      float noise_power) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_samples) return;
    
    // time axis
    double t = (double)idx / sample_rate;
    
    // generate multi-tone signal
    float signal = 0.0f;
    signal += 0.6f * sinf(2.0f * M_PI * base_freq * t);           // base frequency
    signal += 0.3f * sinf(2.0f * M_PI * (base_freq * 2.0) * t);   // double frequency
    signal += 0.1f * sinf(2.0f * M_PI * (base_freq * 0.5) * t);   // half frequency
    
    // add Gaussian white noise
    curandState state;
    curand_init(idx + blockIdx.x, 0, 0, &state);
    float noise = curand_normal(&state) * noise_power;
    
    // output signal = amplitude * (signal + noise)
    d_waveform_data[idx] = amplitude * (signal + noise);
}

/**
 * @brief CUDA kernel function: convert float to 16-bit integer (DAC format)
 * @param d_float_waveform input float waveform
 * @param d_int16_waveform output 16-bit integer waveform
 * @param num_samples number of samples
 * @param scale_factor scale factor
 */
__global__ void float_to_int16_waveform(const float* d_float_waveform,
                                       ViInt16* d_int16_waveform,
                                       int num_samples,
                                       float scale_factor) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_samples) return;
    
    // scale to 16-bit integer range [-32767, 32767]
    float scaled_value = d_float_waveform[idx] * scale_factor;
    scaled_value = fmaxf(-32767.0f, fminf(32767.0f, scaled_value));
    d_int16_waveform[idx] = (ViInt16)scaled_value;
}

// ===============================================================================
// C++ class definition - simplified CUDA signal processor
// ===============================================================================

/**
 * @brief simplified CUDA signal processor class
 * 
 * Encapsulate basic CUDA signal generation functionality:
 * - GPU memory management
 * - Real waveform generation
 * - Data format conversion
 */
class SimpleCudaSignalProcessor {
private:
    // GPU memory pointer
    float* d_float_waveform_;
    ViInt16* d_int16_waveform_;
    
    // CPU memory pointer
    ViInt16* h_waveform_data_;
    
    // configuration parameters
    int waveform_size_;
    double sample_rate_;
    double base_freq_;
    
public:
    /**
     * @param waveform_size waveform size
     * @param sample_rate sample rate
     * @param base_freq base frequency
     */
    SimpleCudaSignalProcessor(int waveform_size = DEFAULT_WAVEFORM_SIZE,
                             double sample_rate = DEFAULT_SAMPLE_RATE,
                             double base_freq = DEFAULT_FREQUENCY)
        : waveform_size_(waveform_size), sample_rate_(sample_rate), base_freq_(base_freq) {
        
        // allocate GPU memory
        CUDA_CHECK(cudaMalloc(&d_float_waveform_, waveform_size_ * sizeof(float)));
        CUDA_CHECK(cudaMalloc(&d_int16_waveform_, waveform_size_ * sizeof(ViInt16)));
        
        // allocate CPU memory
        h_waveform_data_ = new ViInt16[waveform_size_];
        
        std::cout << "Simple CUDA Signal Processor initialized with " << waveform_size_ << " samples" << std::endl;
    }
    

    ~SimpleCudaSignalProcessor() {
        cudaFree(d_float_waveform_);
        cudaFree(d_int16_waveform_);
        
        delete[] h_waveform_data_;
        
        std::cout << "Simple CUDA Signal Processor destroyed" << std::endl;
    }
    
    /**
     * @brief generate real waveform signal
     * @param amplitude signal amplitude
     * @param noise_power noise power
     * @return 0 if successful, -1 if failed
     */
    int generateRealWaveform(float amplitude = 0.8f, float noise_power = 0.01f) {
        int num_blocks = (waveform_size_ + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
        
        // start CUDA kernel function to generate real waveform
        generate_real_waveform<<<num_blocks, THREADS_PER_BLOCK>>>(
            d_float_waveform_, waveform_size_, sample_rate_, base_freq_,
            amplitude, noise_power);
        
        CUDA_CHECK(cudaGetLastError());
        CUDA_CHECK(cudaDeviceSynchronize());
        
        std::cout << "Real waveform generated successfully" << std::endl;
        return 0;
    }
    
    /**
     * @brief convert to 16-bit integer waveform and get data
     * @param scale_factor scale factor
     * @return 16-bit integer waveform data pointer
     */
    ViInt16* convertToInt16Waveform(float scale_factor = 32767.0f) {
        int num_blocks = (waveform_size_ + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
        
        // convert float to 16-bit integer
        float_to_int16_waveform<<<num_blocks, THREADS_PER_BLOCK>>>(
            d_float_waveform_, d_int16_waveform_, waveform_size_, scale_factor);
        
        CUDA_CHECK(cudaGetLastError());
        CUDA_CHECK(cudaDeviceSynchronize());
        
        // copy to CPU memory
        CUDA_CHECK(cudaMemcpy(h_waveform_data_, d_int16_waveform_, 
                             waveform_size_ * sizeof(ViInt16), cudaMemcpyDeviceToHost));
        
        std::cout << "Signal converted to 16-bit integer format" << std::endl;
        return h_waveform_data_;
    }
    
    int getWaveformSize() const { return waveform_size_; }
    
    double getSampleRate() const { return sample_rate_; }
    
    double getBaseFreq() const { return base_freq_; }
};

// ===============================================================================
// IVI device manager class
// ===============================================================================

/**
 * @brief IVI device manager class
 * 
 * Encapsulate basic IVI device initialization, configuration, and control functionality
 */
class SimpleIviDeviceManager {
private:
    // device session handles
    iviSUATools_ViSession* sua_tools_session_;
    iviFgen_ViSession* fgen_session_;
    iviDigitizer_ViSession* digitizer_session_;
    iviSyncATrig_ViSession* sync_a_trig_session_;
    
    // resource path
    std::string resource_db_path_;
    bool initialized_;
    
public:
    SimpleIviDeviceManager() : initialized_(false) {
        sua_tools_session_ = new iviSUATools_ViSession;
        fgen_session_ = new iviFgen_ViSession;
        digitizer_session_ = new iviDigitizer_ViSession;
    }
    
    ~SimpleIviDeviceManager() {
        if (initialized_) {
            cleanup();
        }
        delete sua_tools_session_;
        delete fgen_session_;
        delete digitizer_session_;
    }
    
    /**
     * @brief initialize all IVI devices
     * @param config_path configuration file path
     * @return VI_STATE_SUCCESS if successful, VI_STATE_FAIL if failed
     */
    ViStatus initialize(const std::string& config_path = "conf/config.yaml") {
        try {
            IVI_CHECK(IviSUATools_Initialize(sua_tools_session_));
            std::cout << "SUA Tools initialized successfully" << std::endl;
            
            resource_db_path_ = IviSUATools_ScanOnlineBoards(sua_tools_session_, config_path.c_str());
            std::cout << "Resource DB path: " << resource_db_path_ << std::endl;
            
            IVI_CHECK(IviFgen_Initialize(FGEN_RESOURCE_NAME, VI_STATE_FALSE, VI_STATE_TRUE, 
                                        fgen_session_, resource_db_path_));
            std::cout << "Fgen initialized successfully" << std::endl;
            
            IVI_CHECK(IviDigitizer_Initialize(DIGITIZER_RESOURCE_NAME, VI_STATE_FALSE, VI_STATE_TRUE, 
                                             digitizer_session_, resource_db_path_));
            std::cout << "Digitizer initialized successfully" << std::endl;
            
            IVI_CHECK(IviSyncATrig_Initialize(SYNC_ATRIG_RESOURCE_NAME, VI_STATE_FALSE, VI_STATE_TRUE, 
                                             sync_a_trig_session_, resource_db_path_));
            std::cout << "Sync ATrig initialized successfully" << std::endl;
            
            initialized_ = true;
            return VI_STATE_SUCCESS;
            
        } catch (const std::exception& e) {
            std::cerr << "Initialization failed: " << e.what() << std::endl;
            return VI_STATE_FAIL;
        }
    }

    /**
     * @brief sync sua
     * @return VI_STATE_SUCCESS if successful, VI_STATE_FAIL if failed
     */
    ViStatus syncSua() {
        IVI_CHECK(IviSUATools_Sync(sua_tools_session_, sync_a_trig_session_, fgen_session_, digitizer_session_));
        return VI_STATE_SUCCESS;
    }
    
    /**
     * @brief configure signal generator
     * @param sample_rate sample rate
     * @return VI_STATE_SUCCESS if successful, VI_STATE_FAIL if failed
     */
    ViStatus configureFgen(double sample_rate = DEFAULT_SAMPLE_RATE) {
        try {
            IVI_CHECK(IviFgen_SetAttributeViReal64(fgen_session_, "0", IVIFGEN_ATTR_ARB_SAMPLE_RATE, sample_rate));
            
            IVI_CHECK(IviFgen_SetAttributeViUInt32(fgen_session_, "0", IVIFGEN_ATTR_TRIGGER_SOURCE, 
                                                  IVIFGEN_VAL_TRIGGER_SOURCE_EXTERNAL));     
            std::cout << "Fgen configured successfully" << std::endl;
            return VI_STATE_SUCCESS;
            
        } catch (const std::exception& e) {
            std::cerr << "Fgen configuration failed: " << e.what() << std::endl;
            return VI_STATE_FAIL;
        }
    }
    
    /**
     * @brief configure digitizer
     * @param sample_rate sample rate
     * @param data_depth data depth
     * @return VI_STATE_SUCCESS if successful, VI_STATE_FAIL if failed
     */
    ViStatus configureDigitizer(double sample_rate = DEFAULT_SAMPLE_RATE, 
                               ViUInt32 data_depth = DEFAULT_WAVEFORM_SIZE) {
        try {
            IVI_CHECK(IviDigitizer_SetAttributeViReal64(digitizer_session_, "0", IVIDIGITIZER_ATTR_SAMPLE_STATE, sample_rate));
            IVI_CHECK(IviDigitizer_SetAttributeViUInt32(digitizer_session_, "0", IVIDIGITIZER_ATTR_RECV_SIZE, data_depth));
            
            IVI_CHECK(IviDigitizer_SetAttributeViUInt32(digitizer_session_, "0", IVIDIGITIZER_ATTR_SAMPLE_ENABLE, 1));
            
            IVI_CHECK(IviDigitizer_ConfigureChannelDataDepthInt16(digitizer_session_, "0", data_depth));
            
            std::cout << "Digitizer configured successfully" << std::endl;
            return VI_STATE_SUCCESS;
            
        } catch (const std::exception& e) {
            std::cerr << "Digitizer configuration failed: " << e.what() << std::endl;
            return VI_STATE_FAIL;
        }
    }
    
    /**
     * @brief download waveform to signal generator
     * @param waveform_data waveform data
     * @param waveform_size waveform size
     * @param channel channel number
     * @return VI_STATE_SUCCESS if successful, VI_STATE_FAIL if failed
     */
    ViStatus downloadWaveform(ViInt16* waveform_data, int waveform_size, const std::string& channel = "0") {
        try {
            // convert to Real64 format
            std::vector<ViReal64> real64_waveform(waveform_size);
            for (int i = 0; i < waveform_size; ++i) {
                real64_waveform[i] = static_cast<ViReal64>(waveform_data[i]) / 32768.0;
            }
            
            // create waveform handle
            auto wfm_handle = std::make_unique<waveformHandle>();
            IVI_CHECK(IviFgen_CreateArbWaveformViReal64(fgen_session_, waveform_size, 
                                                       real64_waveform.data(), wfm_handle.get()));
            
            IVI_CHECK(IviFgen_ConfigureArbWfmMemOffset(fgen_session_, wfm_handle.get(), 0));
            
            IVI_CHECK(IviFgen_ConfigureArbWaveformViReal64(fgen_session_, channel.c_str(), 
                                                          wfm_handle.get(), 1.0, 0.0));
            
            std::cout << "Waveform downloaded to channel " << channel << " successfully" << std::endl;
            return VI_STATE_SUCCESS;
            
        } catch (const std::exception& e) {
            std::cerr << "Waveform download failed: " << e.what() << std::endl;
            return VI_STATE_FAIL;
        }
    }
    
    /**
     * @brief start signal generation
     * @return VI_STATE_SUCCESS if successful, VI_STATE_FAIL if failed
     */
    ViStatus startGeneration() {
        try {
            IVI_CHECK(IviFgen_InitiateGeneration(fgen_session_));
            std::cout << "Signal generation started" << std::endl;
            return VI_STATE_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Failed to start generation: " << e.what() << std::endl;
            return VI_STATE_FAIL;
        }
    }
    
    /**
     * @brief stop signal generation
     * @return VI_STATE_SUCCESS if successful, VI_STATE_FAIL if failed
     */
    ViStatus stopGeneration() {
        try {
            IVI_CHECK(IviFgen_AbortGeneration(fgen_session_));
            std::cout << "Signal generation stopped" << std::endl;
            return VI_STATE_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Failed to stop generation: " << e.what() << std::endl;
            return VI_STATE_FAIL;
        }
    }
    
    /**
     * @brief start data acquisition
     * @return VI_STATE_SUCCESS if successful, VI_STATE_FAIL if failed
     */
    ViStatus startAcquisition() {
        try {
            IVI_CHECK(IviDigitizer_InitiateAcquisition(digitizer_session_));
            std::cout << "Data acquisition started" << std::endl;
            return VI_STATE_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Failed to start acquisition: " << e.what() << std::endl;
            return VI_STATE_FAIL;
        }
    }
    
    /**
     * @brief wait for acquisition to complete and get data
     * @param channel channel number
     * @param acquired_data output acquired data buffer
     * @param data_size data size
     * @param timeout timeout (seconds)
     * @return VI_STATE_SUCCESS if successful, VI_STATE_FAIL if failed
     */
    ViStatus fetchWaveformData(const std::string& channel, ViInt16* acquired_data, 
                              ViUInt32 data_size, ViReal64 timeout = 10.0) {
        try {
            // wait for acquisition to complete
            IVI_CHECK(IviDigitizer_WaitForAcquisitionComplete(digitizer_session_, channel.c_str(), timeout));
            
            // get waveform data
            IVI_CHECK(IviDigitizer_FetchWaveformInt16(digitizer_session_, channel.c_str(), 
                                                     data_size, acquired_data));
            
            std::cout << "Waveform data fetched from channel " << channel << std::endl;
            return VI_STATE_SUCCESS;
            
        } catch (const std::exception& e) {
            std::cerr << "Failed to fetch waveform data: " << e.what() << std::endl;
            return VI_STATE_FAIL;
        }
    }
    
    /**
     * @brief clean up resources
     */
    void cleanup() {
        if (initialized_) {
            IviFgen_Close(fgen_session_);
            IviDigitizer_Close(digitizer_session_);
            IviSUATools_Close(sua_tools_session_);
            initialized_ = false;
            std::cout << "IVI devices cleaned up successfully" << std::endl;
        }
    }

    /**
     * @brief generate trigger
     * @return VI_STATE_SUCCESS if successful, VI_STATE_FAIL if failed
     */
    ViStatus generateSystemTrigger() {
        IVI_CHECK(IviSyncATrig_SetAttributeViInt32(
            sync_a_trig_session_, "0", 
            IVISYNCATRIG_ATTR_TRIGGER_SOURCE, 
            IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_INTERNAL));
        IVI_CHECK(IviSyncATrig_SetAttributeViInt32(
            sync_a_trig_session_, "0", 
            IVISYNCATRIG_ATTR_TRIGGER_NUM, 
            0xFFFFFFFF));
        IVI_CHECK(IviSyncATrig_SetAttributeViInt32(
            sync_a_trig_session_, "0", 
            IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_PERIOD, 
            1e6));
        return VI_STATE_SUCCESS;
    }
};

// ===============================================================================
// utility functions - file saving
// ===============================================================================

/**
 * @brief save 16-bit integer data to binary file
 * @param data data pointer
 * @param size data size
 * @param filename file name
 */
void saveDataToFile(const ViInt16* data, int size, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(data), size * sizeof(ViInt16));
        file.close();
        std::cout << "Data saved to " << filename << " (" << size * sizeof(ViInt16) << " bytes)" << std::endl;
    } else {
        std::cerr << "Failed to open file " << filename << std::endl;
    }
}

/**
 * @brief generate timestamp filename
 * @param prefix file name prefix
 * @param extension file extension
 * @return timestamp filename
 */
std::string generateTimestampFilename(const std::string& prefix, const std::string& extension) {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    
    std::tm* localTime = std::localtime(&currentTime);
    char timeStr[32];
    std::strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S", localTime);
    
    return prefix + "_" + std::string(timeStr) + "." + extension;
}



int main() {
    std::cout << "=== Simple CUDA IVI Signal Processing Example ===" << std::endl;
    
    try {
        // 1. initialize CUDA device
        int device_count;
        CUDA_CHECK(cudaGetDeviceCount(&device_count));
        if (device_count == 0) {
            std::cerr << "No CUDA devices found!" << std::endl;
            return -1;
        }
        
        CUDA_CHECK(cudaSetDevice(0));
        cudaDeviceProp device_prop;
        CUDA_CHECK(cudaGetDeviceProperties(&device_prop, 0));
        std::cout << "Using CUDA device: " << device_prop.name << std::endl;
        
        // 2. create simple CUDA signal processor
        auto signal_processor = std::make_unique<SimpleCudaSignalProcessor>(
            DEFAULT_WAVEFORM_SIZE, DEFAULT_SAMPLE_RATE, DEFAULT_FREQUENCY);
        
        // 3. create simple IVI device manager
        auto device_manager = std::make_unique<SimpleIviDeviceManager>();
        
        // 4. initialize IVI devices
        if (device_manager->initialize() != VI_STATE_SUCCESS) {
            std::cerr << "Failed to initialize IVI devices" << std::endl;
            return -1;
        }
        
        // 5. configure devices
        if (device_manager->configureFgen() != VI_STATE_SUCCESS ||
            device_manager->configureDigitizer() != VI_STATE_SUCCESS) {
            std::cerr << "Failed to configure devices" << std::endl;
            return -1;
        }

        // 5.1 sync sua
        if (device_manager->syncSua() != VI_STATE_SUCCESS) {
            std::cerr << "Failed to sync sua" << std::endl;
            return -1;
        }


        
        // 6. generate real waveform signal
        std::cout << "\n=== Generating Real Waveform Signal ===" << std::endl;
        signal_processor->generateRealWaveform(0.8f, 0.005f);
        
        // 7. convert to 16-bit integer waveform
        ViInt16* waveform_data = signal_processor->convertToInt16Waveform();
        
        // 8. download waveform to signal generator
        std::cout << "\n=== Downloading Waveform to Generator ===" << std::endl;
        if (device_manager->downloadWaveform(waveform_data, signal_processor->getWaveformSize(), "0") != VI_STATE_SUCCESS) {
            std::cerr << "Failed to download waveform" << std::endl;
            return -1;
        }
        
        // 9. start signal generation
        std::cout << "\n=== Starting Signal Generation ===" << std::endl;
        if (device_manager->startGeneration() != VI_STATE_SUCCESS) {
            std::cerr << "Failed to start generation" << std::endl;
            return -1;
        }
        
        // 10. start data acquisition
        std::cout << "\n=== Starting Data Acquisition ===" << std::endl;
        if (device_manager->startAcquisition() != VI_STATE_SUCCESS) {
            std::cerr << "Failed to start acquisition" << std::endl;
            return -1;
        }
        
        // 11. wait and fetch acquired data
        std::vector<ViInt16> acquired_data(signal_processor->getWaveformSize());
        std::cout << "\n=== Fetching Acquired Data ===" << std::endl;
        if (device_manager->fetchWaveformData("0", acquired_data.data(), 
                                             signal_processor->getWaveformSize(), 10.0) != VI_STATE_SUCCESS) {
            std::cerr << "Failed to fetch waveform data" << std::endl;
            return -1;
        }
        
        // 12. save data to file
        std::cout << "\n=== Saving Data to Files ===" << std::endl;
        
        // generate timestamp filename
        std::string generated_filename = generateTimestampFilename("generated_waveform", "bin");
        std::string acquired_filename = generateTimestampFilename("acquired_waveform", "bin");
        
        // save generated waveform and acquired data
        saveDataToFile(waveform_data, signal_processor->getWaveformSize(), generated_filename);
        saveDataToFile(acquired_data.data(), signal_processor->getWaveformSize(), acquired_filename);
        
        // 13. stop signal generation
        std::cout << "\n=== Stopping Signal Generation ===" << std::endl;
        device_manager->stopGeneration();
        
        // 14. show statistics
        std::cout << "\n=== Statistics ===" << std::endl;
        std::cout << "Waveform size: " << signal_processor->getWaveformSize() << " samples" << std::endl;
        std::cout << "Sample rate: " << signal_processor->getSampleRate() / 1e9 << " GSPS" << std::endl;
        std::cout << "Base frequency: " << signal_processor->getBaseFreq() / 1e6 << " MHz" << std::endl;
        std::cout << "Signal duration: " << signal_processor->getWaveformSize() / signal_processor->getSampleRate() * 1e6 << " μs" << std::endl;
        std::cout << "Generated file: " << generated_filename << std::endl;
        std::cout << "Acquired file: " << acquired_filename << std::endl;
        
        std::cout << "\n=== Simple Example Completed Successfully ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return -1;
    }
}