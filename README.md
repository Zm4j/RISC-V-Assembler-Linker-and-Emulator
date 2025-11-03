# RISC-V-Assembler-Linker-and-Emulator

This project implements a complete RISC-V software toolchain in C++, consisting of an assembler, linker, and emulator.

The assembler translates RISC-V assembly code into relocatable object files using Flex and Bison for lexical and syntax analysis.

The linker combines multiple object files, resolves symbols, performs relocations, and produces a final executable image.

The emulator simulates a simplified RISC-V processor capable of executing the generated machine code with support for registers, memory, and interrupts.

The system provides a modular and educational platform for understanding the RISC-V architecture and toolchain design.