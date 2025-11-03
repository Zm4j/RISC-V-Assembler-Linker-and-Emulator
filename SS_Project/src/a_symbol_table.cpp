#include "../inc/a_symbol_table.hpp"
#include <iostream>

SymbolTable::SymbolTable(){
    addNode("text", 0, 's', 0);
}

void SymbolTable::addNode(std::string name, __uint32_t val, __uint8_t type, int section) {
    Node n;

    n.name = name;
    n.val = val;
    n.type = type;
    n.dependent_on_cnt = -1;
    n.sectionID = section;
    nodes[name] = n;
}

void SymbolTable::addNode(SymbolTable::Node node){
    nodes[node.name]=node;
}

SymbolTable::Node* SymbolTable::findNode(std::string name) {
  auto node = nodes.find(name);
  if(node != nodes.end()){
    return &(node->second);
  }
  return nullptr;
}

void SymbolTable::equ_solve_unresolved_equ(Node* nodeSrc){
    if(equ_unresolved_map.find(nodeSrc->name) == equ_unresolved_map.end()) return;
    
    std::vector<std::string> equation = equ_unresolved_map[nodeSrc->name];
    std::vector<int> stack;
    for(auto elem:equation){
        if(elem=="+"){
            int op2 = stack.back();
            stack.pop_back();
            int op1 = stack.back();
            stack.pop_back();
            stack.push_back(op1+op2);
        }
        else if(elem=="-"){
            int op2 = stack.back();
            stack.pop_back();
            int op1 = stack.back();
            stack.pop_back();
            stack.push_back(op1-op2);
        }
        else if(elem=="*"){
            int op2 = stack.back();
            stack.pop_back();
            int op1 = stack.back();
            stack.pop_back();
            stack.push_back(op1*op2);
        }
        else if(elem=="/"){
            int op2 = stack.back();
            stack.pop_back();
            int op1 = stack.back();
            stack.pop_back();
            stack.push_back(op1/op2);
        }
        else if(elem=="~"){
            int op = stack.back();
            stack.pop_back();
            stack.push_back(-op);
        }
        else if((isdigit(elem[0]) && elem.find('_') == std::string::npos) || (elem[0]=='-' && isdigit(elem[1]))){ //literal
            if((elem[1]=='x' || elem[1]=='X') || (elem[0]=='-' && (elem[1]=='x' || elem[1]=='X'))){
                stack.push_back(stol(elem, nullptr, 16));
            }
            else{
                stack.push_back(stol(elem, nullptr, 10));
            }
        }
        else { //symbol
            stack.push_back(findNode(elem)->val);
        }
    }
    std::cout<<nodeSrc->name << " RESOLVED\n";
    nodeSrc->val = stack.back();
}

void SymbolTable::fixup_addresses(Node * nodeSrc, std::vector<std::vector<__uint8_t>>* sectionAddresses){
    for(auto elem: nodeSrc->fix_section_addr){
        sectionAddresses->at(elem.first)[elem.second+0] = ((nodeSrc->val) & 0xFF000000) >> 24;
        sectionAddresses->at(elem.first)[elem.second+1] = ((nodeSrc->val) & 0x00FF0000) >> 16;
        sectionAddresses->at(elem.first)[elem.second+2] = ((nodeSrc->val) & 0x0000FF00) >> 8;
        sectionAddresses->at(elem.first)[elem.second+3] = ((nodeSrc->val) & 0x000000FF);
    }
}

void SymbolTable::update_nodes_depending_on_me(Node* nodeSrc, std::vector<std::vector<__uint8_t>>* sectionAddresses){
    equ_solve_unresolved_equ(nodeSrc);
    fixup_addresses(nodeSrc, sectionAddresses);
    for(auto node: nodeSrc->depends_on_me_vector){
        if(--(node->dependent_on_cnt) == 0){
            update_nodes_depending_on_me(node, sectionAddresses);
            
        }
    }
}

void SymbolTable::printSymbolTable(){
    std::cout << "Name\t\tValue\t\tType\t\tScope\t\tisAlive\t\tSection\n";
    for(auto n: nodes){
        std::cout << n.second.name << ((n.second.name.size()>7)?"\t":"\t\t") << n.second.val << ((n.second.val>9999999)?"\t":"\t\t") << (char)n.second.type << "\t\t" << n.second.scope << "\t\t" << n.second.dependent_on_cnt << "\t\t" << n.second.sectionID << '\n';
    }
    /*std::cout << "--------EQUATIONS FOR EACH ELEMENT--------\n";
    for(auto it : equ_unresolved_map){
        std::cout << it.first << ": ";
        for(auto elem: it.second){
            std::cout << elem << ", ";
        }
        std::cout << "\n";
    }
    */
    /*
    std::cout << "-------DEPENDING ELEMENTS--------\n";
    for(auto n: nodes){
        std::cout << n.first << ": ";
        for(auto elem: n.second.depends_on_me_vector){
            std::cout << elem->name << ' ';
        }
        std::cout << '\n';
    }
    */
}