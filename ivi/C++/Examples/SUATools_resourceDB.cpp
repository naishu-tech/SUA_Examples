//
// Created by sn06129 on 2025/7/25.
//

#include <iostream>
#include <string>
#include "IviSUATools.h"

int main() {
    std::cout << "=== SUA Tools NSWave Python Integration Test ===" << std::endl;
    
    try {
        std::cout << "\n Initializing BVC Manager..." << std::endl;

        auto iviSUATools_vi = new iviSUATools_ViSession;
        auto s = IviSUATools_Initialize(iviSUATools_vi);

        std::cout << "\nInitializing BVC Manager..." << std::endl;
        std::string resource_db_path = IviSUATools_ScanOnlineBoards(iviSUATools_vi, "conf/config.yaml");
        std::cout << "\nInitializing BVC Manager..." << std::endl;
        if (resource_db_path.empty()) {
            std::cerr << "Failed to initialize BVC Manager" << std::endl;
            return -1;
        }
        std::cout << "BVC Manager initialized successfully. Resource DB: " << resource_db_path << std::endl;

        delete iviSUATools_vi;

        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown exception in main" << std::endl;
        return -1;
    }
} 