#include <iostream>
#include <termios.h>
#include <unistd.h>
#include "../inc/e_emulator.hpp"

int main(int argc, char* argv[]){
  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  if(argc < 2){
    std::cerr<<"add file in line";
    return -1;
  }

  std::string inputFile = argv[1];

  std::cout<<"Hi\n";
  Emulator e = Emulator();
  e.mem.loadMemFromFile(inputFile);
  e.Execute();
  
  e.peekRegisters();
  e.mem.peekMemFromAddr(0xFFFFFF00, 6);
  e.mem.peekMemFromAddr(0x40000000, 6);
  e.printStack();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

//chmod +x start.sh
//./start.sh