# simple-pipeline-with-an-instruction-decoder-with-stalling
 
Model simple pipeline with the following three stages.  

```
 1 stage for fetch (IF) 
 1 stage for decode (ID) 
 1 stage for instruction and dependency analyze (IA) 
 1 stage for branch (BR) 
 1 stages for addition and subtraction (Execution unit 1: EX1) 
 1 stages for multiplication and division (Execution unit 2: EX2) 
 2 stages for memory operation (Memory unit: Mem1 and Mem2) 
 1 stage for write back (WB) 
 16 x 4B registers 
 64KB memory (code for 0‐999, data for 1000 – 65535) 
memory.map is the memory map file for this project.  
 Instruction formats 
```

The pipeline supports 4B fixed‐length instructions, which have 1B for opcode, 1B for destination, and 2B 
for two operands. The destination and the left operand are always registers. The right operand can be 
either register or an immediate value.  

```
Opcode (1B)  Destination (1B)  Left Operand (1B)  Right Operand (1B) 
Opcode (1B)  Operand (1B)  Immediate value (2B) 
```
The supported instructions have 19 different types as follows. The pipeline only supports integer 
arithmetic operations with 16 integer registers (R0 ‐ R15), where each has 4B. All numbers between 0 
and 1 will be discarded (floor). 

```
Mnemonic  Opcode 
```
```
Description 
Destination (1B)  Left Operand (1B)  Right Operand (1B) 
Operand (1B)  Immediate value (2B) 
```
```
set  0x00 
```
```
set Rx, #Imm (Set an immediate value to register Rx) 
Register Rx  Immediate value 
```
```
add  0x10 
```
```
add  Rx, Ry, Rz (Compute Rx = Ry + Rz) 
Register Rx  Register Ry  Register Rz 
```
```
add  0x11 
```
```
add  Rx, Ry, #Imm (Compute Rx = Ry + an immediate valve) 
Register Rx  Register Ry  Immediate value 
```
```
sub  0x20 
```
```
sub  Rx, Ry, Rz (Compute Rx = Ry – Rz) 
Register Rx  Register Ry  Register Rz 
```
```
sub  0x21 
```
```
sub  Rx, Ry, #Imm (Compute Rx = Ry ‐ an immediate valve) 
Register Rx  Register Ry  Immediate value 
```
```
mul  0x30 
```
```
mul  Rx, Ry, Rz (Compute Rx = Ry * Rz) 
Register Rx  Register Ry  Register Rz 
```
```
mul  0x31 
```
```
mul  Rx, Ry, #Imm (Compute Rx = Ry * an immediate valve) 
Register Rx  Register Ry  Immediate value 
```
```
div  0x40 
```
```
div  Rx, Ry, Rz (Compute Rx = Ry / Rz) 
Register Rx  Register Ry  Register Rz 
```
```
div  0x41 
```
```
div  Rx, Ry, #Imm (Compute Rx = Ry / an immediate valve) 
Register Rx  Register Ry  Immediate value 
```
```
ld  0x50 
```
```
ld  Rx, #Addr  (load the data stored in #Addr into register Rx)
Register Rx  Immediate value 
```
```
ld  0x51 
```
```
ld  Rx, Rz  (load into register Rx the data stored in the address at Rz) 
Register Rx  Register Rz 
```
```
st  0x60 
```
```
st  Rx, #Addr  (store the content of register Rx into the address #Addr. E.g.) 
Register Rx  Immediate value 
```
```
st  0x61 
```
```
st  Rx, Rz  (store the content of register Rx into the address at Rz) 
Register Rx  Register Rz 
```

```
bez  0x70 
```
```
bez Rx, #Imm  (branch to #Imm if Rx==0) 
Register Rx  Immediate value 
```
```
bgez  0x71 
```
```
bgez Rx, #Imm (branch to #imm if Rx >= 0) 
Register Rx  Immediate value 
```
```
blez  0x72 
```
```
blez Rx, #Imm (branch to #imm if Rx <= 0) 
Register Rx  Immediate value 
```
```
bgtz  0x73 
```
```
bgtz Rx, #Imm (branch to #imm if Rx > 0) 
Register Rx  Immediate value 
```
```
bltz  0x74 
```
```
bltz Rx, #Imm (branch to #imm if Rx < 0) 
Register Rx  Immediate value 
```
```
ret  0xFF 
```
```
ret (exit the current program) 
```
All pipeline stages require 1 cycle to complete. A data dependency check and register reads are done at 
the IA stage. If there is dependency, the instruction must wait at the IA stage until the result becomes 
available. Once the register is ready, the instruction can read and proceed to the next stage.  

There are four different execution stages, Ex1, Ex2, Mem1, and Mem2. The Ex1 is for addition and 
subtraction instructions, and the Ex2 stage is for multiplication and division. A memory instruction's 
effective address is prepared at the Ex1 stage, and the memory instruction is completed at the end of 
the Mem2 stage. 

The branch target address is computed at the EX1 stage. The branch condition is checked at the Br 
stage, and the PC is updated at the same cycle. After the PC update, a new instruction is fetched at the 
next cycle (i.e., cycle 1: calculate target address, cycle 2: condition check + pc update, cycle 3: fetch a 
new instruction). To handle branch instruction easily, you can assume that you detect the branch 
instruction at the fetch stage and stall the instruction fetch.

The memory map file contains the snapshot of the system's main memory. The file position 0 to 65535 is 
mapped to the main memory address 0 to 65535. The data at the file position presents the data in the 
corresponding location of the main memory. Although the programs are in separate files, they are 
mapped to the memory address 0 to 999. You do not need to copy the programs to the memory map 
file. 

Your register file has two read ports and one write port, supporting up to two register reads and one 
register write per cycle. The initial memory values are set in the given memory map file (memory.map). 

