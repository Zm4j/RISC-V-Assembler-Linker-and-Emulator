#include <vector>
#include <bits/types.h>
#include <string>

class CodeGenerator{
public:
  CodeGenerator();
  int current_section = 0;
  int flag_loaded_first_symbol = 0;
  std::vector<std::vector<__uint8_t>> BytesInSection;

  void addByte(__uint8_t byte, int cnt);

  void add4Bytes(__uint32_t dword);

  int currentByte();

  void printSections();
  
  void loadStoreOperand(std::string operand, int reg, std::string mode);

  void jumpOperand(std::string operand, int reg1, int reg2, std::string mode);
};