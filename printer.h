#pragma once

#include <iostream>
#include "constants.h"
#include <fstream>
#include <string>

class Printer {
public:
    explicit Printer(const std::string& filename) {
        fileStream.open(filename);
    }

    ~Printer() {
        fileStream.close();
    }

    void print(const std::string& text) {
        fileStream << text ;
    }

    void print_ln(const std::string& text) {
        fileStream << text <<std::endl ;
    }

private:
    std::ofstream fileStream;
};