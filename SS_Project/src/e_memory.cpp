#include "../inc/e_memory.hpp"
#include <fstream>
#include <iostream>

void Memory::writeByte(__uint32_t addr, __uint8_t data){
  __uint32_t PVT_entry = (addr & 0xFFF00000) >> 20;
  if(PVT[PVT_entry]==nullptr){
    PVT[PVT_entry] = new __uint8_t[1024*1024];
  }
  __uint32_t PVT_addr = (addr & 0xFFFFF);

  PVT[PVT_entry][PVT_addr]=data;
}

void Memory::writeWord(__uint32_t addr, __uint16_t data){
  __uint32_t PVT_entry = (addr & 0xFFF00000) >> 20;
  if(PVT[PVT_entry]==nullptr){
    PVT[PVT_entry] = new __uint8_t[1024*1024];
  }
  __uint32_t PVT_addr = (addr & 0xFFFFF);

  PVT[PVT_entry][PVT_addr+1]=(data >> 8) & 0xFF;
  PVT[PVT_entry][PVT_addr+0]=data & 0xFF;
}

void Memory::writeDoubleWord(__uint32_t addr, __uint32_t data){
  __uint32_t PVT_entry = (addr & 0xFFF00000) >> 20;
  if(PVT[PVT_entry]==nullptr){
    PVT[PVT_entry] = new __uint8_t[1024*1024];
  }
  __uint32_t PVT_addr = (addr & 0xFFFFF);

  PVT[PVT_entry][PVT_addr+3]=(data >> 24) & 0xFF;
  PVT[PVT_entry][PVT_addr+2]=(data >> 16) & 0xFF;
  PVT[PVT_entry][PVT_addr+1]=(data >> 8) & 0xFF;
  PVT[PVT_entry][PVT_addr+0]=data & 0xFF;
}

__uint8_t Memory::readByte(__uint32_t addr){
  __uint32_t PVT_entry = (addr & 0xFFF00000) >> 20;
  if(PVT[PVT_entry]==nullptr){
    PVT[PVT_entry] = new __uint8_t[1024*1024];
  }
  __uint32_t PVT_addr = (addr & 0xFFFFF);

  return PVT[PVT_entry][PVT_addr];
}

__uint16_t Memory::readWord(__uint32_t addr){
  __uint32_t PVT_entry = (addr & 0xFFF00000) >> 20;
  if(PVT[PVT_entry]==nullptr){
    PVT[PVT_entry] = new __uint8_t[1024*1024];
  }
  __uint32_t PVT_addr = (addr & 0xFFFFF);

  __uint16_t res = (PVT[PVT_entry][PVT_addr+1]<<8)+PVT[PVT_entry][PVT_addr+0];
  return res;
}

__uint32_t Memory::readDoubleWord(__uint32_t addr){
  if(addr%4==0){
    __uint32_t PVT_entry = (addr & 0xFFF00000) >> 20;
    if(PVT[PVT_entry]==nullptr){
      PVT[PVT_entry] = new __uint8_t[1024*1024];
    }
    __uint32_t PVT_addr = (addr & 0xFFFFF);

    __uint32_t res = (((((PVT[PVT_entry][PVT_addr+3]<<8)+PVT[PVT_entry][PVT_addr+2])<<8)+PVT[PVT_entry][PVT_addr+1])<<8)+PVT[PVT_entry][PVT_addr+0];
    
    return res;
  }
  else{
    __uint32_t res = 0;
    __uint32_t temp_addr = addr-(addr%4);
    if(addr%4==1){
      res |= readByte(temp_addr+0) << 8;
      res |= readByte(temp_addr+1) << 16;
      res |= readByte(temp_addr+2) << 24;
      res |= readByte(temp_addr+7);
    }
    if(addr%4==2){
      res |= readByte(temp_addr+0) << 16;
      res |= readByte(temp_addr+1) << 24;
      res |= readByte(temp_addr+6);
      res |= readByte(temp_addr+7) << 8;
    }
    if(addr%4==3){
      res |= readByte(temp_addr+0) << 24;
      res |= readByte(temp_addr+5);
      res |= readByte(temp_addr+6) << 8;
      res |= readByte(temp_addr+7) << 16;
    }

    return res;
  }
}

void Memory::loadMemFromFile(std::string fileName){
  std::ifstream file(fileName);

  if(!file.is_open()){
    std::cerr << "Error: can't open file\n";
    return;
  }

  __uint32_t addr, val;
  while(file >> std::hex >> addr >> val){
    __uint32_t PVT_entry = (addr & 0xFFF00000) >> 20;
    if(PVT[PVT_entry]==nullptr){
      PVT[PVT_entry] = new __uint8_t[1024*1024];
    }
    __uint32_t PVT_addr = (addr & 0xFFFFF);
    
    PVT[PVT_entry][PVT_addr+0]=(val >> 24) & 0xFF;
    PVT[PVT_entry][PVT_addr+1]=(val >> 16) & 0xFF;
    PVT[PVT_entry][PVT_addr+2]=(val >> 8) & 0xFF;
    PVT[PVT_entry][PVT_addr+3]=val & 0xFF;
  }

  if(file.bad()){
    std::cerr << "Error: error reading file\n";
  }

  file.close();
  return;
}

void Memory::peekMemFromAddr(__uint32_t addr, __uint32_t cnt){
  __uint32_t PVT_entry = (addr & 0xFFF00000) >> 20;
  if(PVT[PVT_entry]==nullptr){
    PVT[PVT_entry] = new __uint8_t[1024*1024];
  }
  __uint32_t PVT_addr = (addr & 0xFFFFF);

  std::cout << "------- MEMORY -------\n";
  for(__uint32_t i=0;i<cnt;i++){
    std::cout << "0x" << std::hex << (addr+i*4) << ": ";
    //std::cout << std::hex << (__uint32_t)mem[addr+0+i*4] << " " << (__uint32_t)mem[addr+1+i*4] << " " << (__uint32_t)mem[addr+2+i*4] << " " << (__uint32_t)mem[addr+3+i*4] << '\n';
    std::cout << std::hex << (__uint32_t)PVT[PVT_entry][PVT_addr+0+i*4] << " " << (__uint32_t)PVT[PVT_entry][PVT_addr+1+i*4] << " " << (__uint32_t)PVT[PVT_entry][PVT_addr+2+i*4] << " " << (__uint32_t)PVT[PVT_entry][PVT_addr+3+i*4] << '\n';

  }

}