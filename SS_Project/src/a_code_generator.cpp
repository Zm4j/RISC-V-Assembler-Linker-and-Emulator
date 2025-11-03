#include "../inc/a_code_generator.hpp"
#include <iostream>

CodeGenerator::CodeGenerator(){
    std::vector<__uint8_t> vec;
    BytesInSection.push_back(vec);
    current_section = 0;
}

void CodeGenerator::addByte(__uint8_t byte, int cnt=1){
  for(int i=0;i<cnt;i++){
    BytesInSection[current_section].push_back(byte);
  }
}

void CodeGenerator::add4Bytes(__uint32_t dword){
  addByte((dword & 0xFF000000) >> 24);
  addByte((dword & 0x00FF0000) >> 16);
  addByte((dword & 0x0000FF00) >> 8);
  addByte((dword & 0x000000FF));
}

int CodeGenerator::currentByte(){
  return BytesInSection[current_section].size();
}

void CodeGenerator::printSections(){
  for(int i=0;i<BytesInSection.size();i++){
    std::cout << "section" << i << ":";
    for(int j=0;j<BytesInSection[i].size();j++){
      if(j%4==0){
        std::cout << "\n0x" << std::hex << j << ": ";
        std::cout << std::hex << (int)BytesInSection[i][j]/16 << (int)BytesInSection[i][j]%16 << " ";
      }
      else {
        std::cout << std::hex << (int)BytesInSection[i][j]/16 << (int)BytesInSection[i][j]%16 << " ";
      }
    } 
    std::cout << "\n";
  }
}

void CodeGenerator::loadStoreOperand(std::string operand_string, int reg, std::string mode){
  
  if(mode=="load"){
    if(operand_string[0]=='a'){
      int operand = std::stoi(operand_string.substr(2));
      add4Bytes(0x91ff0004);    // pc = pc + 4
      add4Bytes(operand);       // op
      add4Bytes(0x920f0ff8 | (reg << 20) );   // r = [pc - 8]
    }
    else if(operand_string[0]=='b'){
      int operand = std::stoi(operand_string.substr(2));
      add4Bytes(0x91ff0004);    // pc = pc + 4
      add4Bytes(operand);       // op
      add4Bytes(0x920f0ff8);    // r0 = [pc - 8]
      add4Bytes(0x93000000);    // r0 = [r0]
      add4Bytes(0x81e00ffc);    // push r0
      add4Bytes(0x63000000);    // r0 = 0
      add4Bytes(0x930e0004 | (reg << 20) );   // pop r
    }
    else if(operand_string[0]=='c'){
      if(operand_string[2]=='r'){
        int operand = std::stoi(operand_string.substr(3));
        add4Bytes(0x91000000 | (reg << 20) | (operand << 16));  // r = pgr[op]
      }
      else{
        int operand = std::stoi(operand_string.substr(3));
        add4Bytes(0x90000000 | (reg << 20) | (operand << 16));  // r = csr[op]
      }
    }
    else if(operand_string[0]=='d'){
      if(operand_string[2]=='r'){
        int operand = std::stoi(operand_string.substr(3));
        add4Bytes(0x92000000 | (reg << 20) | (operand << 16));  // r = [gpr[op]]
      }
      else{
        int operand = std::stoi(operand_string.substr(3));
        add4Bytes(0x90000000 | (operand << 16));  // r0 = csr[op]
        add4Bytes(0x93000000);  // r0 = [r0]
        add4Bytes(0x81e00ffc);  // push r0
        add4Bytes(0x63000000);  // r0 = 0
        add4Bytes(0x930e0004 | (reg << 20));  // pop r
      }
    }
    else if(operand_string[0]=='e'){
      size_t plusPos = operand_string.find('+');

      int reg_op = std::stoi(operand_string.substr(3, plusPos - 3));
      int operand = std::stoi(operand_string.substr(plusPos + 1));
      
      if(operand_string[2]=='r'){
        add4Bytes(0x92000000 | (reg << 20) | (reg_op << 16) | ((operand) & 0x00000FFF));
      }
      else{
        add4Bytes(0x90000000 | (reg_op << 16) );  // r0 = csr[op] 
        add4Bytes(0x91000000 | ((operand) & 0x00000FFF)); // r0 = r0 + reg_op
        add4Bytes(0x93000000);  // r0 = [r0]
        add4Bytes(0x81e00ffc);  // push r0
        add4Bytes(0x63000000);  // r0 = 0
        add4Bytes(0x930e0004 | (reg << 20));  // pop r
      }
    }
  }
  if(mode=="store"){
    if(operand_string[0]=='b'){
      int operand = std::stoi(operand_string.substr(2));
      add4Bytes(0x91ff0004);    // pc = pc + 4
      add4Bytes(operand);       // op
      add4Bytes(0x82f00ff8 | (reg << 12) );   // mem[pc - 8] = r
    }
    else if(operand_string[0]=='c'){
      if(operand_string[2]=='r'){
        int operand = std::stoi(operand_string.substr(3));
        add4Bytes(0x91000000 | (operand << 20) | (reg << 16));  // pgr[op] = r
      }
      else{
        int operand = std::stoi(operand_string.substr(3));
        add4Bytes(0x94000000 | (operand << 20) | (reg << 16));  // csr[op] = r
      }
    }
    else if(operand_string[0]=='d'){
      if(operand_string[2]=='r'){
        int operand = std::stoi(operand_string.substr(3));
        add4Bytes(0x80000000 | (reg << 12) | (operand << 20));  // [gpr[op]] = r
      }
      else{
        int operand = std::stoi(operand_string.substr(3));
        add4Bytes(0x81e01ffc);  // push r1
        add4Bytes(0x90100000 | (operand << 16));  // r1 = csr[op]
        add4Bytes(0x80100000 | (reg << 12));  // [r1] = r
        add4Bytes(0x931e0004);  // pop r1
      }
    }
    else if(operand_string[0]=='e'){
      size_t plusPos = operand_string.find('+');

      int reg_op = std::stoi(operand_string.substr(3, plusPos - 3));
      int operand = std::stoi(operand_string.substr(plusPos + 1));
      
      if(operand_string[2]=='r'){
        add4Bytes(0x80000000 | (reg << 12) | (reg_op << 20) | ((operand) & 0x00000FFF));
      }
      else{
        add4Bytes(0x81e01ffc);  // push r1
        add4Bytes(0x90100000 | (reg_op << 16));  // r1 = csr[op]
        add4Bytes(0x91110000 | ((operand) & 0x00000FFF)); // r1 = r1 + op2
        add4Bytes(0x80100000 | (reg << 12));  // [r1] = r
        add4Bytes(0x931e0004);  // pop r1
      }
    }
  }
}

void CodeGenerator::jumpOperand(std::string operand, int reg1, int reg2, std::string mode){
  if(mode=="jump"){
    loadStoreOperand(operand, 15, "load"); // pc = load(operand)
  }
  if(mode=="call"){
    add4Bytes(0x81e01ffc);  // push r1
    add4Bytes(0x81e01ffc);  // push r1
    add4Bytes(0x911f0014);  // r1 = pc + 20(0x14)
    add4Bytes(0x80e01004);  // mem[sp+4] = r1
    add4Bytes(0x931e0004);  // pop r1
    loadStoreOperand(operand, 15, "load"); // pc = load(operand)
  }
  if(mode=="beq"){
    add4Bytes(0x32f00000 | (reg1 << 16) | (reg2 << 12) | (12));
    loadStoreOperand(operand, 15, "load"); // pc = load(operand)
  }
  if(mode=="bne"){
    add4Bytes(0x31f00000 | (reg1 << 16) | (reg2 << 12) | (12));
    loadStoreOperand(operand, 15, "load"); // pc = load(operand)
  }
  if(mode=="bgt"){
    add4Bytes(0x33f00000 | (reg1 << 12) | (reg2 << 16) | (12));
    //add4Bytes(0x32f00000 | (reg1 << 16) | (reg2 << 12) | (12));
    loadStoreOperand(operand, 15, "load"); // pc = load(operand)
  }
}