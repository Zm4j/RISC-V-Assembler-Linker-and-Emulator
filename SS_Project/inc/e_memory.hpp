#include <bits/types.h>
#include <string>

class Memory{
public:
  void writeByte(__uint32_t addr, __uint8_t data);

  void writeWord(__uint32_t addr, __uint16_t data);

  void writeDoubleWord(__uint32_t addr, __uint32_t data);

  __uint8_t readByte(__uint32_t addr);

  __uint16_t readWord(__uint32_t addr);

  __uint32_t readDoubleWord(__uint32_t addr);

  void loadMemFromFile(std::string fileName);

  void peekMemFromAddr(__uint32_t addr, __uint32_t cnt);

private:
  __uint8_t* PVT[0x1000] = {nullptr};
};