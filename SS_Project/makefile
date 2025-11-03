# ========================================
# ============ Global Config =============
# ========================================

CXX := g++
FLEX := flex
BISON := bison
CXXFLAGS := -Wall -Wextra -std=c++17 -pthread

INCLUDE_DIR := include
SRC_DIR := src
MISC_DIR := misc
BUILD_DIR := build

INCLUDES := -I$(INCLUDE_DIR)

# ========================================
# ============ Assembler (a_*.cpp) =======
# ========================================

ASSEMBLER_SRC := $(wildcard src/a_*.cpp)
ASSEMBLER_OBJS := $(patsubst src/%.cpp, build/assembler/%.o, $(ASSEMBLER_SRC))
ASSEMBLER_TARGET := asembler

LEX_FILE := misc/flex/lexer.l
PARSER_FILE := misc/bison/parser.y

LEX_BUILD := build/assembler/lexer
PARSER_BUILD := build/assembler/parser

LEX_CPP := $(LEX_BUILD)/lex.yy.c
LEX_OBJ := $(LEX_BUILD)/lex.yy.o
PARSER_CPP := $(PARSER_BUILD)/parser.tab.cpp
PARSER_HPP := $(PARSER_BUILD)/parser.tab.hpp
PARSER_OBJ := $(PARSER_BUILD)/parser.tab.o

ASSEMBLER_INCLUDES := -Iinclude -I$(LEX_BUILD) -I$(PARSER_BUILD)

$(LEX_BUILD) $(PARSER_BUILD):
	mkdir -p $@

$(PARSER_CPP) $(PARSER_HPP): $(PARSER_FILE) | $(PARSER_BUILD)
	$(BISON) -d -o $(PARSER_CPP) $(PARSER_FILE)

$(PARSER_OBJ): $(PARSER_CPP)
	$(CXX) $(CXXFLAGS) $(ASSEMBLER_INCLUDES) -c $< -o $@

$(LEX_CPP): $(LEX_FILE) | $(LEX_BUILD)
	$(FLEX) -o $@ $<

$(LEX_OBJ): $(LEX_CPP)
	$(CXX) $(CXXFLAGS) $(ASSEMBLER_INCLUDES) -c $< -o $@

build/assembler/%.o: src/%.cpp | $(PARSER_HPP) $(LEX_CPP)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(ASSEMBLER_INCLUDES) -c $< -o $@

build_assembler: $(ASSEMBLER_OBJS) $(LEX_OBJ) $(PARSER_OBJ)
	$(CXX) $(CXXFLAGS) $(ASSEMBLER_INCLUDES) $^ -o $(ASSEMBLER_TARGET)

# ========================================
# ============ Linker ====================
# ========================================

LINKER_BUILD := $(BUILD_DIR)/linker
LINKER_SRC := $(wildcard $(SRC_DIR)/l_*.cpp)
LINKER_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(LINKER_BUILD)/%.o,$(LINKER_SRC))
LINKER_TARGET := ./linker

$(LINKER_BUILD):
	mkdir -p $@

$(LINKER_BUILD)/%.o: $(SRC_DIR)/%.cpp | $(LINKER_BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

build_linker: $(LINKER_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $(LINKER_TARGET)

# ========================================
# ============ Emulator ==================
# ========================================

EMULATOR_BUILD := $(BUILD_DIR)/emulator
EMULATOR_SRC := $(wildcard $(SRC_DIR)/e_*.cpp)
EMULATOR_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(EMULATOR_BUILD)/%.o,$(EMULATOR_SRC))
EMULATOR_TARGET := ./emulator

$(EMULATOR_BUILD):
	mkdir -p $@

$(EMULATOR_BUILD)/%.o: $(SRC_DIR)/%.cpp | $(EMULATOR_BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

build_emulator: $(EMULATOR_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $(EMULATOR_TARGET)

# ========================================
# ============ Meta Targets ==============
# ========================================

build_all: build_assembler build_linker build_emulator
	@echo "✅ All projects built successfully!"

clean:
	rm -rf $(BUILD_DIR) ./asembler ./linker ./emulator
	@echo "🧹 Cleaned all build artifacts."
