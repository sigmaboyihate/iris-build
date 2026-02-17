#include "cli.hpp"
#include <iostream>


int main(int argc, char* argv[]){
    if (argc < 2) {
        std::cout << "incorrect usage, for help do: 'iris --help'\n";
        return 1;
    }
    std::string command = argv[1];


    if (command == "help" | command == "--help"){
        iris::cli::helpmenu(); // you can find the thingies for args in src/cli 
    }
}