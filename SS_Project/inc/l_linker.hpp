#include "a_symbol_table.hpp"
#include "a_code_generator.hpp"

#define Node SymbolTable::Node
#define EQU_MAP std::unordered_map<std::string, std::vector<std::string>>
#define SYMBOL_TABLE std::unordered_map<std::string, Node>
#define CODE_SECTION std::vector<std::vector<__uint8_t>>

class Linker{
public:
  std::vector<EQU_MAP> equ_map_vector;
  std::vector<SYMBOL_TABLE> st_nodes_vector;
  std::vector<CODE_SECTION> code_section_vector;

  SYMBOL_TABLE globalTable;

  std::vector<std::pair<std::string, int>> sctn_addr;

  std::unordered_map<std::string, std::vector<int>> section_has_files;
  std::vector<std::string> order_of_sections;
  std::unordered_map<std::string, std::vector<int>> global_variables_used_in_files;

  std::vector<std::string> inputFiles;
  std::string outputFile="";

  void importAssemblyOutput(std::string inputFile);

  void printLoadedData();
  void printLinkerData();

  void linkAssemblyFiles();

  void relocateAssemblyFiles(std::string outputFile);

  void loadMemory();

  void resolveSymbol(int fileID, std::string symbol);

  void addSectionPlace(std::pair<std::string, int> sectionPlace);

  void updateSectionAddr(std::vector<std::pair<std::string, int>> sctn_addr);

  __uint32_t loadLineFromFileSection(int file, int section, int addr);
};