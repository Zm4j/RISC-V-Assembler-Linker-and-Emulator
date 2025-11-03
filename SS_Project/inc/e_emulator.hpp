#include "e_memory.hpp"
#include <thread>
#include <mutex>

class Emulator{
public:

  union GenReg32{
    __uint32_t dword;
    __uint16_t word[2];
    __uint8_t  byte[4];
  };

  typedef struct CPU_Reg_struct{
    GenReg32 reg[16], csr[3];
    __uint32_t flags; // ???
  }CPU_Reg_struct;

  CPU_Reg_struct CPU_Reg;
  __uint32_t tim_cfg_addr = 0xFFFFFF10;
  __uint32_t ter_out_addr = 0xFFFFFF00;
  __uint32_t ter_in_addr  = 0xFFFFFF04;

  #define pc CPU_Reg.reg[15]
  #define sp CPU_Reg.reg[14]
  #define status CPU_Reg.csr[0]
  #define handler CPU_Reg.csr[1]
  #define cause CPU_Reg.csr[2]

  #define status_Tr (status.dword & 0x01)
  #define status_Tl (status.dword & 0x02)
  #define status_I (status.dword & 0x04)

  Memory mem;

  Emulator();

  void Execute();

  void peekRegisters();

  void push(__uint32_t val);

  __uint32_t  pop();

  void printStack();

private:
  std::thread* timerThread = nullptr;
  std::thread* inputThread = nullptr;

  std::mutex mutex;
  bool flag_terminal = 0;
  bool flag_timer = 0;
  __uint32_t stack_size_before_interupt;

  __uint32_t FetchNext(__uint32_t instr);
  void onTick();
  void inputRead();
};