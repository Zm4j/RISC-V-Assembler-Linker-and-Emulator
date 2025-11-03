#include "../inc/e_emulator.hpp"
#include <iostream>
#include <termios.h>
#include <unistd.h>

Emulator::Emulator(){
  pc.dword=0x40000000;
  sp.dword=0xFFFFFF00;

  mem.writeDoubleWord(tim_cfg_addr, 0x00000000);
  mem.writeDoubleWord(ter_in_addr, 0x00000000);
  mem.writeDoubleWord(ter_out_addr, 0x00000000);

  timerThread = new std::thread(&Emulator::onTick, this);
  inputThread = new std::thread(&Emulator::inputRead, this);
}

void Emulator::Execute(){
  int res;
  do{
    //sem.wait()
    mutex.lock();
    uint32_t instr = mem.readDoubleWord(pc.dword);
    pc.dword+=0x04;
    res = FetchNext(instr);

    if(status.dword!= ~(0x1)){
      if(flag_terminal){
        flag_terminal = 0;
        stack_size_before_interupt = sp.dword;
        push(status.dword);
        push(pc.dword);
        cause.dword = 0x03;
        status.dword = ~(0x1);
        pc.dword = handler.dword;
      }
      else if(flag_timer){
        flag_timer = 0;
        stack_size_before_interupt = sp.dword;
        push(status.dword);
        push(pc.dword);
        cause.dword = 0x02;
        status.dword = ~(0x1);
        pc.dword = handler.dword;
      }
    }
    

    mutex.unlock();
    //sem.signal()

  }while(res);
}

__uint32_t Emulator::FetchNext(__uint32_t instr){
  //   4b    4b    4b     4b     4b     4b     4b     4b 
  // ------------------------------------------------------
  // | OC | MOD | RegA | RegB | RegC | Disp | Disp | Disp |
  // ------------------------------------------------------
  
  __uint8_t OC, MOD, RegA, RegB, RegC;
  __int16_t Disp;
  
  OC = (instr & ((0xF)<<28)) >> 28;
  MOD = (instr & ((0xF)<<24)) >> 24;
  RegA = (instr & ((0xF)<<20)) >> 20;
  RegB = (instr & ((0xF)<<16)) >> 16;
  RegC = (instr & ((0xF)<<12)) >> 12;
  Disp = instr & (0xFFF);
  if(Disp & 0x800) Disp |= 0xF000; // for negative values

  //std::cout << "instr - 0x" << std::hex << pc.dword-0x04 << ": " << instr << '\n';

  switch (OC)
  {
  case 0x00:{
    break;
  }
  case 0x01:{
    push(status.dword);
    push(pc.dword);
    cause.dword = 0x04;
    status.dword = status.dword&(~0x1);
    pc.dword = handler.dword;
    break;
  }
  case 0x02:{
    switch (MOD)
    {
    case 0x00:{
      push(pc.dword);
      pc.dword = CPU_Reg.reg[RegA].dword + CPU_Reg.reg[RegB].dword + Disp;
      break;
    }
    case 0x01:{
      push(pc.dword);
      pc.dword = mem.readDoubleWord(CPU_Reg.reg[RegA].dword + CPU_Reg.reg[RegB].dword + Disp);
      break;
    }
    default:{
      break;
    }
    }
    break;
  }
  case 0x03:{
    switch (MOD)
    {
    case 0x00:{
      pc.dword = CPU_Reg.reg[RegA].dword + Disp;
      break;
    }
    case 0x01:{
      if(CPU_Reg.reg[RegB].dword == CPU_Reg.reg[RegC].dword){
        pc.dword = CPU_Reg.reg[RegA].dword + Disp;
      }
      break;
    }
    case 0x02:{
      if(CPU_Reg.reg[RegB].dword != CPU_Reg.reg[RegC].dword){
        pc.dword = CPU_Reg.reg[RegA].dword + Disp;
      }
      break;
    }
    case 0x03:{
      if((int)CPU_Reg.reg[RegB].dword > (int)CPU_Reg.reg[RegC].dword){
        pc.dword = CPU_Reg.reg[RegA].dword + Disp;
      }
      break;
    }
    case 0x08:{
      pc.dword = mem.readDoubleWord(CPU_Reg.reg[RegA].dword + Disp);
      break;
    }
    case 0x09:{
      if(CPU_Reg.reg[RegB].dword == CPU_Reg.reg[RegC].dword){
        pc.dword = mem.readDoubleWord(CPU_Reg.reg[RegA].dword + Disp);
      }
      break;
    }
    case 0x0a:{
      if(CPU_Reg.reg[RegB].dword != CPU_Reg.reg[RegC].dword){
        pc.dword = mem.readDoubleWord(CPU_Reg.reg[RegA].dword + Disp);
      }
      break;
    }
    case 0x0b:{
      if((int)CPU_Reg.reg[RegB].dword > (int)CPU_Reg.reg[RegC].dword){
        pc.dword = mem.readDoubleWord(CPU_Reg.reg[RegA].dword + Disp);
      }
      break;
    }
    default:{
      break;
    }
    }
    break;
  }
  case 0x04:{
    __uint32_t temp = CPU_Reg.reg[RegB].dword;
    CPU_Reg.reg[RegB].dword = CPU_Reg.reg[RegC].dword;
    CPU_Reg.reg[RegC].dword = temp;
    break;
    }
  case 0x05:{
    switch (MOD)
    {
    case 0x00:{
      CPU_Reg.reg[RegA].dword = CPU_Reg.reg[RegB].dword + CPU_Reg.reg[RegC].dword; 
      break;
    }
    case 0x01:{
      CPU_Reg.reg[RegA].dword = CPU_Reg.reg[RegB].dword - CPU_Reg.reg[RegC].dword;
      break;
    }
    case 0x02:{
      CPU_Reg.reg[RegA].dword = CPU_Reg.reg[RegB].dword * CPU_Reg.reg[RegC].dword;
      break;
    }
    case 0x03:{
      CPU_Reg.reg[RegA].dword = CPU_Reg.reg[RegB].dword / CPU_Reg.reg[RegC].dword;
      break;
    }
    default:{
      break;
    }
    }
    break;
  }
  case 0x06:{
    switch (MOD)
    {
    case 0x00:{
      CPU_Reg.reg[RegA].dword = ~CPU_Reg.reg[RegB].dword; 
      break;
    }
    case 0x01:{
      CPU_Reg.reg[RegA].dword = CPU_Reg.reg[RegB].dword & CPU_Reg.reg[RegC].dword;
      break;
    }
    case 0x02:{
      CPU_Reg.reg[RegA].dword = CPU_Reg.reg[RegB].dword | CPU_Reg.reg[RegC].dword;
      break;
    }
    case 0x03:{
      CPU_Reg.reg[RegA].dword = CPU_Reg.reg[RegB].dword ^ CPU_Reg.reg[RegC].dword;
      break;
    }
    default:{
      break;
    }
    }
    break;
  }
  case 0x07:{
    switch (MOD)
    {
    case 0x00:{
      CPU_Reg.reg[RegA].dword = CPU_Reg.reg[RegB].dword << CPU_Reg.reg[RegC].dword;
      break;
    }
    case 0x01:{
      CPU_Reg.reg[RegA].dword = CPU_Reg.reg[RegB].dword >> CPU_Reg.reg[RegC].dword;
      break;
    }
    default:{
      break;
    }
    }
    break;
  }
  case 0x08:{
    switch (MOD)
    {
    case 0x00:{
      __uint32_t addr = CPU_Reg.reg[RegA].dword+CPU_Reg.reg[RegB].dword+Disp;
      if(addr == ter_out_addr){
        mem.writeDoubleWord(ter_out_addr, CPU_Reg.reg[RegC].dword);
        std::cout << CPU_Reg.reg[RegC].byte[3] << std::flush;
      }
      else{
        mem.writeDoubleWord(addr, CPU_Reg.reg[RegC].dword);
      }
      break;
    }
    case 0x02:{
      
      __uint32_t addr = mem.readDoubleWord(CPU_Reg.reg[RegA].dword+CPU_Reg.reg[RegB].dword+Disp);
      if(addr == ter_out_addr){
        //std::cout << "GAS2\n" << CPU_Reg.reg[RegC].byte[0] << '\n';
        mem.writeDoubleWord(ter_out_addr, CPU_Reg.reg[RegC].dword);
        for(int i=0;i<4;i++){
          if(CPU_Reg.reg[RegC].byte[3-i]!=0){
            std::cout << CPU_Reg.reg[RegC].byte[3-i] << std::flush;
            break;
          }
        }
        
      }
      else{
        mem.writeDoubleWord(addr, CPU_Reg.reg[RegC].dword);
      }
      break;
    }
    case 0x01:{
      CPU_Reg.reg[RegA].dword = CPU_Reg.reg[RegA].dword+Disp;
      __uint32_t addr = CPU_Reg.reg[RegA].dword;
      if(addr == ter_out_addr){
        mem.writeDoubleWord(ter_out_addr, CPU_Reg.reg[RegC].dword);
        std::cout << CPU_Reg.reg[RegC].byte[3] << std::flush;
      }
      else{
        mem.writeDoubleWord(addr, CPU_Reg.reg[RegC].dword);
      }
      break;
    } 
    
    default:{
      break;
    }
    }
    break;
  }
  case 0x09:{
    switch(MOD){
      case 0x00:{
        CPU_Reg.reg[RegA].dword = CPU_Reg.csr[RegB].dword;
        break;
      }
      case 0x01:{
        CPU_Reg.reg[RegA].dword = CPU_Reg.reg[RegB].dword + Disp;
        break;
      }
      case 0x02:{
        CPU_Reg.reg[RegA].dword = mem.readDoubleWord(CPU_Reg.reg[RegB].dword + CPU_Reg.reg[RegC].dword + Disp);
        break;
      }
      case 0x03:{
        CPU_Reg.reg[RegA].dword = mem.readDoubleWord(CPU_Reg.reg[RegB].dword);
        CPU_Reg.reg[RegB].dword = CPU_Reg.reg[RegB].dword + Disp;
        break;
      }
      case 0x04:{
        CPU_Reg.csr[RegA].dword = CPU_Reg.reg[RegB].dword;
        break;
      }
      case 0x05:{
        CPU_Reg.csr[RegA].dword = CPU_Reg.csr[RegB].dword | Disp;
        break;
      }
      case 0x06:{
        CPU_Reg.csr[RegA].dword = mem.readDoubleWord(CPU_Reg.reg[RegB].dword + CPU_Reg.reg[RegC].dword + Disp);
        break;
      }
      case 0x07:{
        CPU_Reg.csr[RegA].dword = mem.readDoubleWord(CPU_Reg.reg[RegB].dword);
        CPU_Reg.reg[RegB].dword = CPU_Reg.reg[RegB].dword + Disp;
        break;
      }
      default:{
        break;
      }
    }
    break;
  }
  default:{
    break;
  }
  }
  return OC;
}

void Emulator::push(__uint32_t val){
  sp.dword = sp.dword - 0x04;
  mem.writeDoubleWord(sp.dword, val);
}

__uint32_t Emulator::pop(){
  __uint32_t val = mem.readDoubleWord(sp.dword);
  sp.dword = sp.dword + 0x04;
  return val;
}

void Emulator::printStack(){
  std::cout << "------- STACK -------\n";
  for(__uint32_t i=sp.dword;i<0xFFFFFF00;i+=0x04){
    std::cout << std::hex << i << ": " << mem.readDoubleWord(i) << '\n';
  }
}

void Emulator::peekRegisters(){
  std::cout << "------- REGISTERS -------\n";
  for(int i=0;i<14;i++){
    std::cout << "gpr[" << i << "] = 0x" << std::hex << CPU_Reg.reg[i].dword << '\n';
  }
  std::cout << "sp = 0x" << std::hex << sp.dword << '\n';
  std::cout << "pc = 0x" << std::hex << pc.dword << '\n';
  std::cout << "status = 0x" << std::hex << status.dword << '\n';
  std::cout << "handler = 0x" << std::hex << handler.dword << '\n';
  std::cout << "cause = 0x" << std::hex << cause.dword << '\n';
}

void Emulator::onTick(){
  while(1){
    __uint32_t intervalMs = mem.readDoubleWord(tim_cfg_addr);
    switch (intervalMs)
    {
    case 0x0:{
      intervalMs = 500;
      break;
    }
    case 0x1:{
      intervalMs = 1000;
      break;
    }
    case 0x2:{
      intervalMs = 1500;
      break;
    }
    case 0x3:{
      intervalMs = 2000;
      break;
    }
    case 0x4:{
      intervalMs = 5000;
      break;
    }
    case 0x5:{
      intervalMs = 10000;
      break;
    }
    case 0x6:{
      intervalMs = 30000;
      break;
    }
    case 0x7:{
      intervalMs = 60000;
      break;
    }
    default:{
      intervalMs = -1;
      break;
    }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    //sem.wait()
    mutex.lock();
    
    flag_timer = 1;

    mutex.unlock();
    //sem.signal() - needs to be added when iret is active, idk how to do it
  }
}

void Emulator::inputRead(){
  while(1){
    char c;
    if(read(STDIN_FILENO, &c, 1) > 0){
      //sem.wait()
      mutex.lock();
      
      mem.writeDoubleWord(ter_in_addr, c);
      flag_terminal = 1;

      mutex.unlock();
      //sem.signal() - needs to be added when iret is active, idk how to do it
    }
    
  }
}