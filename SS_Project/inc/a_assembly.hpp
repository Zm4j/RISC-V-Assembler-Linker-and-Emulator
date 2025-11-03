#include <string>
#include <cstdio>
#include <iostream>
#include "a_symbol_table.hpp"
#include "a_code_generator.hpp"

// Forward declaration for Bison/Flex

class Assembly {
public:
    std::string inputPath;
    SymbolTable st;
    CodeGenerator cg;

    explicit Assembly(std::string inputPath);

    int execute();

    void exportAssemblyOutput(std::string outputPath);
};
