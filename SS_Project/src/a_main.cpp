#include "../inc/a_assembly.hpp"

SymbolTable *st;
CodeGenerator *cg;

int main(int argc, char* argv[]) {
    if(argc<2){
        std::cerr << "input file is mandatory\n";
        return -1;
    }

    std::string inputFile;
    std::string outputFile = "";

    for(int i=1;i<argc;i++){
        std::string argv_str = argv[i];
        if(argv_str=="-o"){
            i++;
            argv_str = argv[i];
            outputFile = argv_str;
        }
        else{
            inputFile = argv_str;
            if(outputFile==""){
                outputFile = inputFile;
                outputFile.back()='o';
            }
        }
    }

    Assembly assembler(inputFile);
    st = &assembler.st;
    cg = &assembler.cg;
    cg->flag_loaded_first_symbol=0;
    cg->current_section = 0;
    assembler.execute();
    st->printSymbolTable();
    cg->printSections();
    assembler.exportAssemblyOutput(outputFile);
    
    return 0;
}
