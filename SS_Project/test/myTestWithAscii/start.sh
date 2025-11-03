ASSEMBLER=./asembler
LINKER=./linker
EMULATOR=./emulator

${ASSEMBLER} -o main.o main.s

${LINKER} -hex \
  -place=textt@0x40000000 -place=data@0x70000000\
  -o program.hex \
  main.o
${EMULATOR} program.hex