#include <memory>
#include <string>
#include <stdexcept>
#include <iostream>
#include "Micro.h"

int main(int argc, char** argv)
{
    std::cout << "main: Program started." << std::endl; 
    try {
        std::cout << "main: Creating Micro..." << std::endl; 
        std::unique_ptr<Micro> micro = std::make_unique<Micro>();
        std::cout << "main: Micro created. Starting app..." << std::endl;
        micro->startApp(argc, argv);
        std::cout << "main: startApp finished." << std::endl; 
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "main: Program finished." << std::endl; 
    return EXIT_SUCCESS;
}