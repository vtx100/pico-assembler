# pico-assembler

**Pico Assembler** is a lightweight assembler written in C for a custom instruction set *similar* to the 8bit PicoBlaze set. It parses pseudoinstructions and outputs the corresponding machine code.



## 🧱 Build 
```bash
git clone https://github.com/vtx100/pico-assembler.git
cd pico-assembler
mkdir build && cd build
cmake ..
cmake --build.
```
## ✅ Run
```bash
./pico-assembler -i <in_file> -o <out_file> -f <format>
```
## ❓ Help
```bash
./pico-assembler -h 
```
## 📘 Supported instructions
```
| Instruction     | arg_type     |
|-----------------|--------------|
| `JMP`           | `ADDR`       |
| `JZ`            | `ADDR`       |
| `JNZ`           | `ADDR`       |
| `JC`            | `ADDR`       |
| `JNC`           | `ADDR`       |
| `CALL`          | `ADDR`       |
| `CALLZ`         | `ADDR`       |
| `CALLNZ`        | `ADDR`       |
| `CALLC`         | `ADDR`       |
| `CALLNC`        | `ADDR`       |
| `RET`           | `NO_ARG`     |
| `RETZ`          | `NO_ARG`     |
| `RETNZ`         | `NO_ARG`     |
| `RETC`          | `NO_ARG`     |
| `RETNC`         | `NO_ARG`     |
| `LOAD`          | `REG_ANY`    |
| `AND`           | `REG_ANY`    |
| `OR`            | `REG_ANY`    |
| `XOR`           | `REG_ANY`    |
| `ADD`           | `REG_ANY`    |
| `ADDCY`         | `REG_ANY`    |
| `SUB`           | `REG_ANY`    |
| `SUBCY`         | `REG_ANY`    |
| `SR0`           | `REG`        |
| `SR1`           | `REG`        |
| `SRX`           | `REG`        |
| `SRA`           | `REG`        |
| `RR`            | `REG`        |
| `SL0`           | `REG`        |
| `SL1`           | `REG`        |
| `SLX`           | `REG`        |
| `SLA`           | `REG`        |
| `RL`            | `REG`        |
| `INPUT`         | `REG_REG`    |
| `INPUTP`        | `REG_IMM`    |
| `OUTPUT`        | `REG_REG`    |
| `OUTPUTP`       | `REG_IMM`    |
| `RETE`          | `NO_ARG`     |
| `RETD`          | `NO_ARG`     |
| `INTE`          | `NO_ARG`     |
| `INTD`          | `NO_ARG`     |
```

### Meaning of argument types
- ADDR : Symbol/Label (Defined using **#** can be either before or after where it is used)
- REG_ANY: allows arguments to be either register/register or register/immediate
- REG: 4 bit unsigned. Passed using: **%r** and an index in the range [0-15] (Decimal representation only)
- REG_REG: 2 register arguments
- REG_IMM: 1 register argument and 1 immediate value bounded by the maximum unsigned representation on 8 bits.Can be passed as:
  - Decimal : using the prefix **!d** followed by a decimal number
  - Binary : using the prefix **!b** followed by a binary number  
## Output formatters
Currently allows outputting in the following formats:
- **vhdlbin** : ``` "<line_idx>" => b"<binary_instruction>",```
- **vhdlhex** : ```  "<line_idx>" => x"<hex_instruction>",```
- **debug** 
## 🖊️ How to use
Example : *in.txt*
```
ADD %1, %13
ADD %1, !d5
JMP jump
ADD %2, %2
#jump
SUB %2 , !b10100010
INTE
```
*Build using* : 
```
./pico-assembler -i in.txt -o out.txt -f debug
```
Console contents:
```
[OK]: I/O + TOKEN
[OK]: PARSE
[OK]: LINKING
[OK]: WRITE TO FILE
[pico-assembler] Successfully assembled 'in.txt'. Wrote to 'out.txt'.
```
Means assembly was successful. Output is in *out.txt*
