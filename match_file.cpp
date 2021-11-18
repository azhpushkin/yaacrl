#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

#include "yaacrl.h"

namespace fs = std::filesystem;


// Simplest possible logger
void custom_logger(yaacrl::LogLevel lvl, std::string msg) {
    std::cout << "[" << static_cast<int>(lvl) << "] " << msg << std::endl;
}



int main(int argc, char* argv[]) {
    yaacrl::set_logger(custom_logger);

    if (argc < 2) {
        std::cout << "Please specify path to the yaacrl database and the test fragment" << std::endl;
        return 1;
    }
    
    const fs::path library = fs::path(argv[1]);
    const fs::path test_path = fs::path(argv[2]);

    yaacrl::Storage storage(library);

    std::cout << "Check " << test_path.filename() << std::endl;
    auto fprint = yaacrl::Fingerprint(yaacrl::WAVFile(test_path));
    auto matches = storage.get_matches(fprint);
    std::cout << "  -> " << " Match results: " <<  std::endl;
    for (auto& pair: matches) {
        std::cout << "    * " << std::get<0>(pair).name << ": " << std::get<1>(pair) * 100 << "%" << std::endl;
    }

    return 0;
    
}