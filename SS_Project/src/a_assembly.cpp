#include "parser.tab.hpp"
#include "../inc/a_assembly.hpp"

extern FILE* yyin;

Assembly::Assembly(std::string s){
    inputPath = s;
}

int Assembly::execute(){
    yyin = fopen(inputPath.c_str(), "r");
    if (!yyin) {
        std::cerr << "Cannot open input.txt\n";
        return 1;
    }

    int result = yyparse();
    if (result == 0)
        std::cout << "Parsing completed successfully!\n";
    else
        std::cout << "Parsing failed!\n";

    fclose(yyin);
    return 0;
}

void Assembly::exportAssemblyOutput(std::string outputPath){
    std::ofstream out(outputPath);
    if (!out.is_open()) {
        std::cerr << "Failed to open file: " << outputPath << "\n";
        return;
    }
    // number of .o files in this file
    out << 1 << "\n";

    // equ_unresolved_map
    out << st.equ_unresolved_map.size() << "\n";
    for (auto &pair : st.equ_unresolved_map) {
        out << pair.first << " " << pair.second.size();
        for (auto &val : pair.second)
            out << " " << val;
        out << "\n";
    }

    // nodes
    out << st.nodes.size() << "\n";
    for (auto &pair : st.nodes) {
        out << pair.first << "\n";       // key
        out << pair.second.name << ' ' << pair.second.val << ' ' << pair.second.type << ' ' << pair.second.sectionID << ' ' << pair.second.dependent_on_cnt << ' ' << pair.second.scope << '\n';
        int cnt = pair.second.fix_section_addr.size(); 
        out << cnt << ' ';
        for(int i=0;i<cnt;i++){
            out << pair.second.fix_section_addr[i].first << ',' << pair.second.fix_section_addr[i].second << ' ';
        }
        cnt = pair.second.depends_on_me_vector.size();
        out << '\n' << cnt << ' ';
        for(int i=0;i<cnt;i++){
            out << pair.second.depends_on_me_vector[i]->name << ' ';
        }
        out << '\n';
    }

    // BytesInSection
    out << cg.BytesInSection.size() << "\n";
    for (auto &vec : cg.BytesInSection) {
        out << vec.size();
        for (auto &b : vec)
            out << " " << static_cast<int>(b);
        out << "\n";
    }

    out.close();
}