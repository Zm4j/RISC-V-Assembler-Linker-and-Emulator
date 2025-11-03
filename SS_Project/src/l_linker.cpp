#include "../inc/l_linker.hpp"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <queue>

void Linker::importAssemblyOutput(std::string inputFile) {
    inputFiles.push_back(inputFile);
    std::ifstream in(inputFile);
    if (!in.is_open()) {
        std::cerr << "Failed to open file: " << inputFile << "\n";
        return;
    }
    int num_of_files;
    in >> num_of_files;
    for(int jj=0;jj<num_of_files;jj++){
        EQU_MAP equ_map;
        SYMBOL_TABLE st_nodes;
        CODE_SECTION code_section;

        // ---------- READ equ_unresolved_map ----------
        size_t equ_map_size;
        in >> equ_map_size;
        in.ignore(); // skip newline

        for (size_t i = 0; i < equ_map_size; ++i) {
            std::string key;
            size_t vec_size;
            in >> key >> vec_size;

            std::vector<std::string> vals(vec_size);
            for (size_t j = 0; j < vec_size; ++j)
                in >> vals[j];

            in.ignore(); // skip newline
            equ_map[key] = vals;
        }
        equ_map_vector.push_back(std::move(equ_map));

        // ---------- READ nodes ----------
        size_t node_count;
        in >> node_count;
        in.ignore(); // skip newline
        
        for (size_t i = 0; i < node_count; ++i) {
        Node n;
        
        // --- line 1 ---
        std::getline(in, n.name);

        // --- line 2 ---
        std::string line2;
        std::getline(in, line2);
        std::istringstream ss2(line2);
        ss2 >> n.name >> n.val >> n.type >> n.sectionID >> n.dependent_on_cnt >> n.scope;

        // --- line 3 ---
        std::string line3;
        std::getline(in, line3);
        std::istringstream ss3(line3);
        size_t fix_count;
        ss3 >> fix_count;
        n.fix_section_addr.reserve(fix_count);
        for (size_t i = 0; i < fix_count; ++i) {
            std::string pairStr;
            ss3 >> pairStr; // e.g., "1,4"
            size_t comma = pairStr.find(',');
            if (comma != std::string::npos) {
                uint32_t a = std::stoul(pairStr.substr(0, comma));
                uint32_t b = std::stoul(pairStr.substr(comma + 1));
                n.fix_section_addr.emplace_back(a, b);
            }
        }

        // --- line 4 ---
        std::string line4;
        std::getline(in, line4);
        std::istringstream ss4(line4);
        size_t dep_count;
        ss4 >> dep_count;
        n.depends_on_me_vector.reserve(dep_count);
        for (size_t i = 0; i < dep_count; ++i) {
            std::string dep;
            ss4 >> dep;
            Node *newNode = new Node();
            newNode->name = dep;
            n.depends_on_me_vector.push_back(newNode);
        }
        st_nodes[n.name] = n;
        }
        st_nodes_vector.push_back(st_nodes);
        
        
        // ---------- READ BytesInSection ----------
        size_t section_count;
        in >> section_count;
        in.ignore(); // skip newline

        for (size_t i = 0; i < section_count; ++i) {
            size_t byte_count;
            in >> byte_count;
            std::vector<__uint8_t> one_code_section;
            for (size_t j = 0; j < byte_count; ++j) {
                int val;
                in >> val;
                one_code_section.push_back(static_cast<__uint8_t>(val));
            }
            in.ignore(); // skip newline
            code_section.push_back(one_code_section);
        }
        code_section_vector.push_back(std::move(code_section));
    }
    in.close();
}

void Linker::linkAssemblyFiles(){
    
    //first linker iteration, we create table for global symbols, and order of sections to be loaded in memory
    int file_cnt=0;
    std::unordered_map<std::string, int> already_visited_sections;
    for(auto table:st_nodes_vector){
        std::priority_queue<std::pair<int,std::string>> sections_in_file_queue;
        
        for(auto pair: table){
            if(pair.second.type=='s'){
                section_has_files[pair.second.name].push_back(file_cnt);

                if(already_visited_sections.find(pair.second.name)==already_visited_sections.end()){
                    already_visited_sections[pair.second.name] = 1;
                    sections_in_file_queue.push(std::make_pair((-1)*pair.second.sectionID, pair.second.name));
                }
                
            }
            if(pair.second.scope=='g'){
                globalTable[pair.second.name] = pair.second;
            }
            if(pair.second.scope=='e'){
                global_variables_used_in_files[pair.second.name].push_back(file_cnt);
            }
        }
        while(sections_in_file_queue.size()>0){
            order_of_sections.push_back(sections_in_file_queue.top().second);
            sections_in_file_queue.pop();
        }
        file_cnt++;
    }
    //second linker iteration
    
    //first we have to assign all sections starting addresses
    updateSectionAddr(sctn_addr);

    //second we will fix labels
    //this now has to create chain reaction:
    //when we define label value, we will update rest of local variables
    file_cnt = 0;
    for(auto table: st_nodes_vector){
        for(auto pair: table){
            if(pair.second.type=='l'){
                resolveSymbol(file_cnt, pair.second.name);
            }
        }
        file_cnt++;
    }

    //if we define some variable that we export than we will chain reaction local files that need that value for their local varaibles, and so on and so on
    for(auto &pair: global_variables_used_in_files){
        while(pair.second.size()>0){
            int file_val = pair.second[pair.second.size()-1];
            resolveSymbol(file_val, pair.first);
            pair.second.pop_back();
        }
    }
}

void Linker::resolveSymbol(int fileID, std::string symbol){
    Node &node = st_nodes_vector[fileID][symbol];
    
    // we calculate value of symbol
    if(node.scope=='e'){
        //give extern symbol value from global table
        node.val = globalTable[symbol].val;
    }
    else if(node.type=='l'){
        //give label value of start position of its section + offset where label is from that section
        for(auto &pair:st_nodes_vector[fileID]){
            if(pair.second.type=='s' && pair.second.sectionID==node.sectionID){
                node.val = node.val + pair.second.val;
            }
        }
    }
    else{
        //for rest of symbols that are being resolved we are calculating its value from equation
        std::vector<std::string> equation = equ_map_vector[fileID][symbol];
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
                stack.push_back(st_nodes_vector[fileID][elem].val);
            }
        }
        node.val = stack.back();
    }
    if(node.scope=='g'){
        globalTable[symbol].val = node.val;
        //we also need to fix other extern sybols that are calling this
        while(global_variables_used_in_files[symbol].size()>0){
            int file_val = global_variables_used_in_files[symbol][global_variables_used_in_files[symbol].size()-1];
            resolveSymbol(file_val, symbol);
            global_variables_used_in_files[symbol].pop_back();
        }
    }
    node.dependent_on_cnt=0;

    // here we are putting real value of symbol in code
    for(auto elem: node.fix_section_addr){
        code_section_vector[fileID][elem.first][elem.second+0] = ((node.val) & 0xFF000000) >> 24;
        code_section_vector[fileID][elem.first][elem.second+1] = ((node.val) & 0x00FF0000) >> 16;
        code_section_vector[fileID][elem.first][elem.second+2] = ((node.val) & 0x0000FF00) >> 8;
        code_section_vector[fileID][elem.first][elem.second+3] = ((node.val) & 0x000000FF);
    }

    // here we want to notify all symbols that are depending on this one
    for(auto newNode: node.depends_on_me_vector){
        std::string s = newNode->name;
        Node *newNewNode = &st_nodes_vector[fileID][s];
        if(--(newNewNode->dependent_on_cnt)==0){
            resolveSymbol(fileID, newNewNode->name);
        }
    }

}

void Linker::addSectionPlace(std::pair<std::string, int> sectionPlace){
    sctn_addr.push_back(sectionPlace);
}

void Linker::updateSectionAddr(std::vector<std::pair<std::string, int>> sctn_addr){
    uint32_t biggest_place = 0x0;
    std::unordered_map<std::string, int> sections_with_place;
    std::unordered_map<__uint32_t, std::vector<__uint8_t>> combinedSections;

    for(auto pair:sctn_addr){
        std::string symbol = pair.first;
        int startAddr = pair.second;

        int localStartAddr = startAddr;
        for(auto fileID : section_has_files[symbol]){
            st_nodes_vector[fileID][symbol].val += localStartAddr;
            if(st_nodes_vector[fileID][symbol].val%4!=0){
                st_nodes_vector[fileID][symbol].val += 4 - st_nodes_vector[fileID][symbol].val%4;
            }
            localStartAddr += code_section_vector[fileID][st_nodes_vector[fileID][symbol].sectionID].size();
        }
        if(localStartAddr > biggest_place){
            biggest_place = localStartAddr;
        }
        sections_with_place[symbol]=1;
    }

    int current_addr = biggest_place;
    for(auto section:order_of_sections){
        if(sections_with_place[section]!=1){
            for(auto file_num:section_has_files[section]){
                st_nodes_vector[file_num][section].val = current_addr;
                if(st_nodes_vector[file_num][section].val%4!=0){
                    st_nodes_vector[file_num][section].val += 4 - st_nodes_vector[file_num][section].val%4;
                }
                current_addr += code_section_vector[file_num][st_nodes_vector[file_num][section].sectionID].size();
            }
        }
    }
}

void Linker::relocateAssemblyFiles(std::string outputFile){
    
    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "Error: cannot open output file " << outputFile << "\n";
        return;
    }

    out << st_nodes_vector.size() << '\n';

    for (const auto& filename : inputFiles) {
        std::ifstream in(filename);
        if (!in.is_open()) {
            std::cerr << "Error: cannot open input file " << filename << "\n";
            continue;
        }

        std::string line;
        // Skip the first line (the integer)
        std::getline(in, line);

        // Copy remaining lines to output
        while (std::getline(in, line)) {
            out << line << '\n';
        }
    }
    
    /*
    //CEPACKA IDEJA, JBG MALO SJEBANA

    //here we have to combine all local symbol tables into one giant symbol table
    //first we have to rename all local symbols, we will add prefix "[num]_"
    for(int fileID=0; fileID<st_nodes_vector.size(); fileID++){
        std::vector<std::string> oldEqu;
        for(auto &pair:equ_map_vector[fileID]){
            for(auto &elem:pair.second){
                if((st_nodes_vector[fileID].find(elem) != st_nodes_vector[fileID].end()) &&
                   (st_nodes_vector[fileID][elem].scope=='l' && st_nodes_vector[fileID][elem].type!='s')){
                    elem = std::to_string(fileID)+"_"+elem;
                   }
            }
            std::string elem = pair.first;
            if((st_nodes_vector[fileID].find(elem) != st_nodes_vector[fileID].end()) &&
                (st_nodes_vector[fileID][elem].scope=='l' && st_nodes_vector[fileID][elem].type!='s')){
                std::vector<std::string> equ = pair.second;
                oldEqu.push_back(elem);
                equ_map_vector[fileID][std::to_string(fileID)+"_"+elem] = equ;
            }
        }
        for(auto equ:oldEqu){
            equ_map_vector[fileID].erase(equ);
        }

        std::vector<std::string> oldSymbols;
        for(auto &pair:st_nodes_vector[fileID]){
            if(pair.second.scope == 'l' && pair.second.type!='s'){
                std::cout << pair.first << " - " << pair.second.name << "\n";
                oldSymbols.push_back(pair.first);
                Node valNode = pair.second;
                valNode.name = std::to_string(fileID)+"_"+valNode.name;
                st_nodes_vector[fileID][valNode.name] = valNode;
            }
        }
        for(auto sym: oldSymbols){
            st_nodes_vector[fileID].erase(sym);
        }
    }

    //second we have merge
    SYMBOL_TABLE st = st_nodes_vector[0];
    CODE_SECTION cg = code_section_vector[0];
    std::unordered_map<std::string, int> section_size;
    
    for(auto pair:st){
        if(pair.second.type=='s'){
            section_size[pair.second.name] = cg[pair.second.sectionID].size();
        }
    }

    for(int fileID=1;fileID<st_nodes_vector.size();fileID++){
        std::priority_queue<std::pair<int, std::string>> newSections;
        std::unordered_map<int, std::string> sectionsWithSameName;
        
        //section
        for(auto pair:st_nodes_vector[fileID]){
            if(pair.second.type=='s'){
                if(st.find(pair.first)==st.end()){
                    newSections.push(std::make_pair(pair.second.sectionID, pair.first));
                }
                else{
                    sectionsWithSameName[pair.second.sectionID]=pair.second.name;
                }
            }
        }
        //rest of symbols
        for(auto pair:st_nodes_vector[fileID]){
            //fix label that is in same section as previous file that is loaded
            if(pair.second.type=='l'){
                if(sectionsWithSameName.find(pair.second.sectionID)!=sectionsWithSameName.end()){
                    pair.second.val+=section_size[sectionsWithSameName[pair.second.sectionID]];
                }
            }
            if(pair.second.scope=='e'){
                
            }
        }

    }*/
}

void Linker::loadMemory() {
    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "Failed to open file: " << outputFile << " \n";
        return;
    }

    for(auto section: order_of_sections){
        for(auto file: section_has_files[section]){
            int sectionID = st_nodes_vector[file][section].sectionID;
            __uint32_t pc = st_nodes_vector[file][section].val;
            // if(code_section_vector[file][sectionID].size()%4!=0){
            //     for(int i=0;i<3-code_section_vector[file][sectionID].size()%4;i++){
            //         code_section_vector[file][sectionID].push_back(0);
            //     }
            // }
            for(int i=0;i<code_section_vector[file][sectionID].size();i+=4){
                out << "0x" << std::hex << pc << ' ' << loadLineFromFileSection(file,sectionID,i) << '\n';
                pc+=4;
            }
        }
    }
}

__uint32_t Linker::loadLineFromFileSection(int file, int section, int addr){
  __uint32_t fourBytes = 0;
  fourBytes |= code_section_vector[file][section][addr+0];
  fourBytes |= code_section_vector[file][section][addr+1] << 8;
  fourBytes |= code_section_vector[file][section][addr+2] << 16;
  fourBytes |= code_section_vector[file][section][addr+3] << 24;
  return fourBytes;
}

void Linker::printLoadedData() {
    for (size_t fileIndex = 0; fileIndex < equ_map_vector.size(); ++fileIndex) {
        std::cout << "========== FILE INDEX " << fileIndex << " ==========\n\n";

        // Print EQU_MAP
        std::cout << "[EQU_MAP]\n";
        const auto &equ = equ_map_vector[fileIndex];
        if (equ.empty()) {
            std::cout << "  (empty)\n";
        } else {
            for (const auto &pair : equ) {
                std::cout << "  " << pair.first << " : [ ";
                for (const auto &val : pair.second)
                    std::cout << val << " ";
                std::cout << "]\n";
            }
        }
        std::cout << "\n";

        // Print SYMBOL_TABLE
        std::cout << "[SYMBOL_TABLE]\n";
        const auto &symtab = st_nodes_vector[fileIndex];
        if (symtab.empty()) {
            std::cout << "  (empty)\n";
        } else {
            for (const auto &pair : symtab) {
                const Node &n = pair.second;
                std::cout << "SYMBOL: " << pair.first << "\n";
                std::cout << "  Node: " << n.name << ",";
                std::cout << "  val: " << n.val << ",";
                std::cout << "  scope: " << (char)n.scope << ",";
                std::cout << "  type: " << (char)n.type << ",";
                std::cout << "  dependent_on_cnt: " << n.dependent_on_cnt << ",";
                std::cout << "  sectionID: " << n.sectionID << "\n";

                // fix_section_addr
                std::cout << "    fix_section_addr (" << n.fix_section_addr.size() << "): ";
                for (auto &p : n.fix_section_addr)
                    std::cout << "(" << p.first << "," << p.second << ") ";
                std::cout << "\n";

                // depends_on_me_vector
                std::cout << "    depends_on_me_vector (" << n.depends_on_me_vector.size() << "): ";
                for (auto ptr: n.depends_on_me_vector)
                   std::cout << ptr->name << " ";
                std::cout << "\n\n";
            }
        }

        // Print CODE_SECTION
        std::cout << "[CODE_SECTION]\n";
        const auto &code = code_section_vector[fileIndex];
        if (code.empty()) {
            std::cout << "  (empty)\n";
        } else {
            for (size_t secIndex = 0; secIndex < code.size(); ++secIndex) {
                std::cout << "  Section " << secIndex << " (" << code[secIndex].size() << " bytes): ";
                for (auto b : code[secIndex])
                    std::cout << std::hex << std::setw(2) << std::setfill('0')
                              << static_cast<int>(b) << " ";
                std::cout << std::dec << "\n";
            }
        }

        std::cout << "\n";
        std::cout << "DONE\n";
    }
}

void Linker::printLinkerData() {
    std::cout << "==============================" << std::endl;
    std::cout << "       GLOBAL SYMBOL TABLE     " << std::endl;
    std::cout << "==============================" << std::endl;
    for (auto pair : globalTable) {
        std::cout << "Symbol: " << pair.first << " = " << (uint32_t)pair.second.val << std::endl;
    }

    std::cout << "\n==============================" << std::endl;
    std::cout << "     EXTERN SYMBOLS USED      " << std::endl;
    std::cout << "==============================" << std::endl;
    for (const auto& [section, files] : global_variables_used_in_files) {
        std::cout << section << ": ";
        for (size_t i = 0; i < files.size(); ++i) {
            std::cout << files[i];
            if (i + 1 < files.size()) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    std::cout << "\n==============================" << std::endl;
    std::cout << "       SECTION -> FILES        " << std::endl;
    std::cout << "==============================" << std::endl;
    for (const auto& [section, files] : section_has_files) {
        std::cout << section << ": ";
        for (size_t i = 0; i < files.size(); ++i) {
            std::cout << files[i];
            if (i + 1 < files.size()) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    std::cout << "\n==============================" << std::endl;
    std::cout << "       ORDER OF SECTIONS       " << std::endl;
    std::cout << "==============================" << std::endl;
    for (size_t i = 0; i < order_of_sections.size(); ++i) {
        std::cout << i << ": " << order_of_sections[i] << std::endl;
    }

    std::cout << std::endl;
    std::cout << "\n==============================" << std::endl;
    std::cout << "       LOCAL SYMBOL TABLE       " << std::endl;
    std::cout << "==============================" << std::endl;
    for (int i=0; i<st_nodes_vector.size(); i++){
        std::cout << "file" << i << "\n";
        for(auto pair:st_nodes_vector[i]){
            std::cout << pair.first << ": ";
            std::cout <<  std::hex << (uint32_t)pair.second.val << ", " << pair.second.scope << ", " << pair.second.type << "\n";
        }
        std::cout << "\n";
    }
}
