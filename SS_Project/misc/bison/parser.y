%{
#include <iostream>
#include <unordered_map>

#include "../../../inc/a_symbol_table.hpp"
#include "../../../inc/a_code_generator.hpp"
extern SymbolTable* st;
extern CodeGenerator* cg;

void yyerror(const char *s);
int yylex();
%}

%code requires {
    #include <string>
}

%union {
    int int_val;
    std::string* str_val;
}

%token DIR_GLOBAL DIR_EXTERN DIR_SECTION DIR_WORD DIR_SKIP DIR_ASCII DIR_EQU DIR_END
%token HALT INT IRET CALL RET JMP BEQ BNE BGT PUSH POP XCHG
%token ADD SUB MUL DIV NOT AND OR XOR SHL SHR LD ST CSRRD CSRWR
%token DOLAR PERCENT COMMA COLON
%token LBRACKET RBRACKET LBRACKET2 RBRACKET2
%token PLUS MINUS STAR SLASH
%token <str_val> TOKEN_ID TOKEN_STRING
%token <int_val> GPR CSR TOKEN_NUMBER HEX_NUMBER

%type <str_val> symbol section_name string
%type <str_val> global_list_symbol extern_list_symbol list_symbol_literal symbol_literal
%type <str_val> reg
%type <str_val> operand operand_2 operand_3 operand_4
%type <int_val> literal
%type <int_val> expr term factor

%%

program:
    dir_stmt_list
    ;

dir_stmt_list:
      directive dir_stmt_list
    | statement dir_stmt_list
    | directive
    | statement
    ;

directive:
      DIR_GLOBAL global_list_symbol
    | DIR_EXTERN extern_list_symbol
    | DIR_SECTION section_name 
    | DIR_WORD list_symbol_literal
    | DIR_SKIP literal { 
        cg->addByte(0, $2);
      }
    | DIR_ASCII string { 
        for(auto c: *($2)){
          cg->addByte((__uint8_t)c, 1);
        }
        cg->addByte(0x00,1);
     }
    | DIR_EQU symbol1 COMMA expr {
        SymbolTable::Node* node = st->findNode(st->equ_current_symbol);
        node->sectionID = cg->current_section;
        if(node->dependent_on_cnt==0){
          st->update_nodes_depending_on_me(node, &(cg->BytesInSection));
        }
      }
    | DIR_END 
    ;

global_list_symbol:
      symbol COMMA global_list_symbol { 
          SymbolTable::Node* node = st->findNode(*($1));
          node->scope = 'g';
        }
    | symbol { 
        SymbolTable::Node* node = st->findNode(*($1));
        node->scope = 'g';
      }
    ;

extern_list_symbol:
      symbol COMMA extern_list_symbol { 
          SymbolTable::Node* node = st->findNode(*($1));
          node->scope = 'e';
        }
    | symbol { 
          SymbolTable::Node* node = st->findNode(*($1));
          node->scope = 'e';
        } 
    ;

symbol:
      TOKEN_ID { 
          $$ = $1;
          SymbolTable::Node* node = st->findNode(*($1));
          if(node==nullptr){
            st->addNode(*($1), 0, 'x', -1);
          }
        }
    ;

symbol1:
      symbol { 
          st->equ_current_symbol = *($1); 
          std::vector<std::string> vec;
          st->equ_unresolved_map[*($1)] = vec;

          SymbolTable::Node* node = st->findNode(*($1));
          node->dependent_on_cnt=0;
        }

section_name:
      TOKEN_ID { 
          SymbolTable::Node* node = st->findNode(*($1));
          if(node==nullptr){
            std::cout << "GASSS1\n";
            if(cg->flag_loaded_first_symbol == 0 && (st->findNode("text")!=nullptr)){
              std::cout << "GASSS2\n";
              cg->current_section = 0;
              SymbolTable::Node* node = st->findNode("text");
              node->name = *($1);
              node->type = 's';
              node->dependent_on_cnt = 0;
              cg->flag_loaded_first_symbol = 1;
            }
            else{
              std::cout << "GASSS3\n";
              cg->current_section = cg->BytesInSection.size();
              st->addNode(*($1), 0, 's', cg->BytesInSection.size());
              std::vector<__uint8_t> vec;
              cg->BytesInSection.push_back(vec);
              SymbolTable::Node* node = st->findNode(*($1));
              node->dependent_on_cnt = 0;
            }
            
          }
          else{
            std::cout << "GASSS4\n";
            cg->current_section = node->sectionID;
          } 
        }
    ;

list_symbol_literal:
      symbol_literal COMMA list_symbol_literal
    | symbol_literal
    ;

symbol_literal:
      symbol {
          // this is for .word section
          SymbolTable::Node* node = st->findNode(*($1));
          if(node->dependent_on_cnt==0){
            cg->add4Bytes((__uint32_t) node->val);
          }
          else{
            node->fix_section_addr.push_back(std::make_pair(cg->current_section, cg->currentByte()));
            cg->add4Bytes(0x0);
          }

        }
    | literal{ cg->add4Bytes($1); }
    ;

literal:
      TOKEN_NUMBER { $$ = $1; }
    | HEX_NUMBER { $$ = $1; }
    ;

string:
      TOKEN_STRING { std::cout << cg->current_section << '\n'; $$ = $1; }
    ;

expr:
      expr PLUS term { st->equ_unresolved_map[st->equ_current_symbol].push_back("+"); }
    | expr MINUS term { st->equ_unresolved_map[st->equ_current_symbol].push_back("-"); }
    | term
    ;

term:
      term STAR factor { st->equ_unresolved_map[st->equ_current_symbol].push_back("*"); }
    | term SLASH factor { st->equ_unresolved_map[st->equ_current_symbol].push_back("/"); }
    | factor
    ;

factor:
      literal { st->equ_unresolved_map[st->equ_current_symbol].push_back(std::to_string(($1))); }
    | symbol { 
        st->equ_unresolved_map[st->equ_current_symbol].push_back(*($1)); 
        SymbolTable::Node* node = st->findNode(*($1));
        if(node->dependent_on_cnt!=0){ // symbol is not resolved when this is called
          SymbolTable::Node* nodeSrc = st->findNode(st->equ_current_symbol);
          nodeSrc->dependent_on_cnt++;
          node->depends_on_me_vector.push_back(nodeSrc);
        }
      }
    | LBRACKET expr RBRACKET 
    | LBRACKET MINUS factor RBRACKET {
        st->equ_unresolved_map[st->equ_current_symbol].push_back("~");
      }
    ;

statement:
      HALT                  { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x00000000); }
    | INT                   { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x10000000); }
    | IRET                  { cg->flag_loaded_first_symbol=1;
        cg->add4Bytes(0x960e0004);// pop status
        cg->add4Bytes(0x93fe0008);// pop pc
      }
    | CALL operand_4        { cg->flag_loaded_first_symbol=1;
        std::string s = *($2);
        cg->jumpOperand(s, 0, 0, "call");
      }
    | RET                   { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x93fe0004); }
    | JMP operand_2         { cg->flag_loaded_first_symbol=1; 
        std::string s = *($2);
        cg->jumpOperand(s, 0, 0, "jump");
      }
    | BEQ GPR COMMA GPR COMMA operand_3   { cg->flag_loaded_first_symbol=1;
        std::string s = *($6);
        int reg1 = $2;
        int reg2 = $4;
        cg->jumpOperand(s, reg1, reg2, "beq");
      }
    | BNE GPR COMMA GPR COMMA operand_3   { cg->flag_loaded_first_symbol=1;
        std::string s = *($6);
        int reg1 = $2;
        int reg2 = $4;
        cg->jumpOperand(s, reg1, reg2, "bne");
      }
    | BGT GPR COMMA GPR COMMA operand_3   { cg->flag_loaded_first_symbol=1;
        std::string s = *($6);
        int reg1 = $2;
        int reg2 = $4;
        cg->jumpOperand(s, reg1, reg2, "bgt");
      }
    | PUSH GPR              { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x81e00ffc | ((__uint8_t)$2)<<12); }
    | POP GPR               { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x930e0004 | ((__uint8_t)$2)<<20); }
    | XCHG GPR COMMA GPR    { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x40000000 | ((__uint8_t)$2)<<16 | ((__uint8_t)$4)<<12); }
    | ADD GPR COMMA GPR     { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x50000000 | ((__uint8_t)$4)<<20 | ((__uint8_t)$4)<<16 | ((__uint8_t)$2)<<12); }
    | SUB GPR COMMA GPR     { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x51000000 | ((__uint8_t)$4)<<20 | ((__uint8_t)$4)<<16 | ((__uint8_t)$2)<<12); }
    | MUL GPR COMMA GPR     { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x52000000 | ((__uint8_t)$4)<<20 | ((__uint8_t)$4)<<16 | ((__uint8_t)$2)<<12); }
    | DIV GPR COMMA GPR     { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x53000000 | ((__uint8_t)$4)<<20 | ((__uint8_t)$4)<<16 | ((__uint8_t)$2)<<12); }
    | NOT GPR               { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x60000000 | ((__uint8_t)$2)<<20 | ((__uint8_t)$2)<<16); } 
    | AND GPR COMMA GPR     { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x61000000 | ((__uint8_t)$4)<<20 | ((__uint8_t)$4)<<16 | ((__uint8_t)$2)<<12); }
    | OR GPR COMMA GPR      { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x62000000 | ((__uint8_t)$4)<<20 | ((__uint8_t)$4)<<16 | ((__uint8_t)$2)<<12); }
    | XOR GPR COMMA GPR     { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x63000000 | ((__uint8_t)$4)<<20 | ((__uint8_t)$4)<<16 | ((__uint8_t)$2)<<12); }
    | SHL GPR COMMA GPR     { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x70000000 | ((__uint8_t)$4)<<20 | ((__uint8_t)$4)<<16 | ((__uint8_t)$2)<<12); }
    | SHR GPR COMMA GPR     { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x71000000 | ((__uint8_t)$4)<<20 | ((__uint8_t)$4)<<16 | ((__uint8_t)$2)<<12); }
    | LD operand COMMA GPR  { cg->flag_loaded_first_symbol=1;
        
        std::string s = *($2);
        cg->loadStoreOperand(s, ($4), "load");
      }
    | ST GPR COMMA operand  { cg->flag_loaded_first_symbol=1;
        std::string s = *($4);
        cg->loadStoreOperand(s, ($2), "store");
      }
    | CSRRD CSR COMMA GPR   { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x90000000 | ((__uint8_t)$2)<<16 | ((__uint8_t)$4)<<20); }
    | CSRWR GPR COMMA CSR   { cg->flag_loaded_first_symbol=1; cg->add4Bytes(0x94000000 | ((__uint8_t)$2)<<16 | ((__uint8_t)$4)<<20); }
    | symbol COLON          { cg->flag_loaded_first_symbol=1;
        SymbolTable::Node* node = st->findNode(*($1));
        node->type = 'l';
        node->val  = cg->BytesInSection[cg->current_section].size();
        node->dependent_on_cnt = -1;
        node->sectionID = cg->current_section;
        cg->flag_loaded_first_symbol = 1;
      }
    ;

reg:
      CSR { cg->flag_loaded_first_symbol=1; $$ = new std::string("c" + std::to_string($1)); }
    | GPR { cg->flag_loaded_first_symbol=1; $$ = new std::string("r" + std::to_string($1)); }
    ;

operand:
      DOLAR literal   { cg->flag_loaded_first_symbol=1; $$ = new std::string("a:"+std::to_string($2)); }
    | DOLAR symbol    { cg->flag_loaded_first_symbol=1;
        SymbolTable::Node* node = st->findNode(*($2));
        if(node->dependent_on_cnt==0){
          $$ = new std::string("a:"+ std::to_string(node->val));
        }else{
          $$ = new std::string("a:0");
          node->fix_section_addr.push_back(std::make_pair(cg->current_section, 4 + cg->BytesInSection[cg->current_section].size()));
        }
      }               
    | literal         { cg->flag_loaded_first_symbol=1; $$ = new std::string("b:"+std::to_string($1)); }
    | symbol          { cg->flag_loaded_first_symbol=1;
        SymbolTable::Node* node = st->findNode(*($1));
        if(node->dependent_on_cnt==0){
          $$ = new std::string("b:"+ std::to_string(node->val));
        }else{
          $$ = new std::string("b:0");
          node->fix_section_addr.push_back(std::make_pair(cg->current_section, 4 + cg->BytesInSection[cg->current_section].size()));
        }
      }
    | reg             { cg->flag_loaded_first_symbol=1; $$ = new std::string("c:"+(*($1))); } 
    | LBRACKET2 reg RBRACKET2                 { cg->flag_loaded_first_symbol=1; $$ = new std::string("d:"+(*($2))); }
    | LBRACKET2 reg PLUS literal RBRACKET2    { cg->flag_loaded_first_symbol=1; $$ = new std::string("e:" + (*($2)) + "+" + std::to_string($4)); }
    | LBRACKET2 reg PLUS symbol RBRACKET2     { cg->flag_loaded_first_symbol=1;
        SymbolTable::Node* node = st->findNode(*($4));
        if(node->dependent_on_cnt==0){
          $$ = new std::string("e:" + (*($2)) + "+" + std::to_string(node->val)); 
        }
      }        
    ;

operand_2:
      literal   { cg->flag_loaded_first_symbol=1; $$ = new std::string("a:"+std::to_string($1)); }
    | symbol    { cg->flag_loaded_first_symbol=1; 
        SymbolTable::Node* node = st->findNode(*($1));
        if(node->dependent_on_cnt==0){
          $$ = new std::string("a:"+ std::to_string(node->val));
        }else{
          $$ = new std::string("a:0");
          node->fix_section_addr.push_back(std::make_pair(cg->current_section, 4 + cg->BytesInSection[cg->current_section].size()));
        }
      }  
    ;

operand_3:
      literal   { cg->flag_loaded_first_symbol=1; $$ = new std::string("a:"+std::to_string($1)); }
    | symbol    { cg->flag_loaded_first_symbol=1; 
        SymbolTable::Node* node = st->findNode(*($1));
        if(node->dependent_on_cnt==0){
          $$ = new std::string("a:"+ std::to_string(node->val));
        }else{
          $$ = new std::string("a:0");
          node->fix_section_addr.push_back(std::make_pair(cg->current_section, 8 + cg->BytesInSection[cg->current_section].size()));
        }
      }  
    ;

operand_4:
      literal   { cg->flag_loaded_first_symbol=1; $$ = new std::string("a:"+std::to_string($1)); }
    | symbol    { cg->flag_loaded_first_symbol=1; 
        SymbolTable::Node* node = st->findNode(*($1));
        if(node->dependent_on_cnt==0){
          $$ = new std::string("a:"+ std::to_string(node->val));
        }else{
          $$ = new std::string("a:0");
          node->fix_section_addr.push_back(std::make_pair(cg->current_section, 0x18 + cg->BytesInSection[cg->current_section].size()));
        }
      }  
    ;

%%

void yyerror(const char *s) {
    std::cerr << "Parse error: " << s << std::endl;
  }
