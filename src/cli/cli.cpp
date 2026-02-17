#include <iostream>

namespace iris {
    namespace cli {
        void helpmenu(){
            std::cout << "you can read more (too lazy to put git hre)\n";
            std::cout << "made by sol and claddy\n";
            std::cout << "----------------------\n";
            std::cout << "iris config (to check options for iris)\n";
            std::cout << "iris out (build args here, like buildtype etc)\n";
            std::cout << "iris -Disable-(subproject) to not add bloat\n";
            std::cout << "iris optimize (do in build directory, ontop of iris out sets preconfigued -march=native stuff for cpu and yeah makes better\n";
            std::cout << "----------------------\n";

        }
        
        void inputfile(){
            std::cout << "test\n"; 
        }
    }
}