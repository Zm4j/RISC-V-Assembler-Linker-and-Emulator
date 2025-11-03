#include "../inc/l_linker.hpp"
#include <iostream>

int main(int argc, char*argv[]){
  if(argc < 2){
    std::cerr<<"linker must have at least one input file";
    return -1;
  }

  Linker linker;
  int mode=-1;

  for(int i=1;i<argc;i++){
    std::cout << argv[i] << "\n";
    std::string str_argv = argv[i];
    if(str_argv=="-o"){
      i++;
      linker.outputFile = argv[i];
    }
    else if(str_argv=="-hex"){
      if(mode==-1)
        mode = 1;
      else{
        std::cerr << "multiple definition of -hex and/or -relocatable";
        return -1;
      }
    }
    else if(str_argv=="-relocatable"){
      if(mode==-1)
        mode = 2;
      else{
        std::cerr << "multiple definition of -hex and/or -relocatable";
        return -1;
      }
    }
    else if(str_argv[0]=='-'){ // -place
      std::string entry = str_argv.substr(7);
      size_t at_pos = entry.find('@');
      
      std::string section = entry.substr(0, at_pos);
      std::string addr_str = entry.substr(at_pos + 1);

      int address = 0;
      if (addr_str.size() > 2 && (addr_str[0] == '0') && (addr_str[1] == 'x' || addr_str[1] == 'X')) {
        address = std::stoul(addr_str, nullptr, 16);
      } 
      else {
        address = std::stoul(addr_str, nullptr, 10);
      }
      
      linker.addSectionPlace(std::make_pair(section, address));
    }
    else{
      linker.importAssemblyOutput(str_argv);
      if(linker.outputFile==""){
        linker.outputFile = "linker_"+str_argv;
      }
    }
  }

  //linker.importAssemblyOutput("linker/input_files/linker_test1.txt");
  //linker.importAssemblyOutput("linker/input_files/linker_test2.txt");
  //linker.importAssemblyOutput("linker/input_files/linker_test_merged.txt");

  //linker.addSectionPlace(std::make_pair("s3", 0x40000000));
  //linker.addSectionPlace(std::make_pair("s2", 0x00000000));
  //linker.linkAssemblyFiles();
  //linker.relocateAssemblyFiles("linker/input_files/linker_test_merged.txt");
  if(mode==-1){
    std::cerr<<"needed -hex or --relocatable";
    return -1;
  }
  else if(mode==1){
    linker.linkAssemblyFiles();
    linker.loadMemory();
  }
  else if(mode==2){
    linker.relocateAssemblyFiles(linker.outputFile);
  }

  linker.printLoadedData();
  linker.printLinkerData();
}