#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>

class SymbolTable{
public:
  SymbolTable ();
  enum TYPE{};
  struct Node {
      std::string name;
      long val;
      __uint8_t type = 'x';
      __uint8_t scope = 'l';
      std::vector<std::pair<__uint32_t,__uint32_t>> fix_section_addr;
      int dependent_on_cnt;
      std::vector<Node*> depends_on_me_vector; // serialize as names
      int sectionID;
  };

  std::string equ_current_symbol;
  std::unordered_map<std::string, std::vector<std::string>> equ_unresolved_map;
  void equ_solve_unresolved_equ(Node* nodeSrc);
  void fixup_addresses(Node * nodeSrc, std::vector<std::vector<__uint8_t>>* sectionAddresses);
  void update_nodes_depending_on_me(Node* nodeSrc, std::vector<std::vector<__uint8_t>>* sectionAddresses);

  std::unordered_map<std::string,Node> nodes;
  void addNode(std::string name, __uint32_t val, __uint8_t type, int section);
  void addNode(Node n);
  Node* findNode(std::string name);

  void printSymbolTable();
};