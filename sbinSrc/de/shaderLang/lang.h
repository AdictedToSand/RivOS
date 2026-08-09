#pragma once
#include <int.h>

/* Instruction format

opc1 op1 (op2?) (op3?) etc.

opcode format:
    first 6 bytes: main opcode
    first 2 bytes: variant
    For the sake of demonstrating, a full byte will be in square brackets.
    example:

    [011010 10] > This could mean reg->const and says things about the opcode.
    ^
    This is the main opcode (for this example: MOV)
    The instruction is MOV reg, const 
    The register can be a singular byte, so 
    [011010 10] [00101010]
    This could mean: MOV (reg->const) ocr 
    Now a register is 32bits, so the next four bytes need to be the number:
    [011010 10] [00101010] [0][0][0][42] (42 in binary, not gonna write that out)
    This means:
    MOV (reg->conts) ocr, 42

    Register:
        - tmp1, tmp2 are just temporary registers.
        - ocr=OperatorControlRegister. This registers value decides the result of BIN <output>
*/

#define OPCODE_FULL (u8) 0b11111100
#define SUBOPCODE_FULL (u8) 0b00000011 
// This enum will represent the first 6 bytes of an opcode
// To check (opcode == MOV), do:
// const u8 opcodeWithoutSubOpcode = inputted & OPCODE_FULL
// if (opcodeWithoutSubOpcode == (MOV << 2))) (2 == the size of a subopcode)
typedef enum Opcode {
    BIN = 0b000001,
    LOAD = 0b000010,
    SET_FRAG = 0b000011,
    PUSH = 0b000100,
    POP = 0b000101,
    RET = 0b000110,
    CALL = 0b000111,
    LOAD_X = 0b001000,
    LOAD_Y = 0b001001,
    JMP = 0b001010,
    JCOR = 0b001011,
    JNCOR = 0b001100,
    CMP = 0b001101,
    CALL_BUILTIN = 0b001110,
    CVAR = 0b001111,
    DVAR = 0b010000,
    MOV = 0b010001
} Opcode;

typedef enum SubOpcode {
    BIN_ONLY = 0b00, // (->reg)

    LOAD_CONST = 0b00,
    LOAD_REG = 0b01,
    LOAD_VAR = 0b10,

    SET_FRAG_ONLY = 0b00, // (->const)

    PUSH_CONST = 0b00,
    PUSH_REG = 0b01,
    PUSH_VAR = 0b10,
    POP_REG = 0b00,
    POP_VAR = 0b01,

    RET_ONLY = 0b00, // (none)
    CALL_ONLY = 0b00,

    LOAD_X_ONLY = 0b00, LOAD_Y_ONLY = 0b00, // (->reg)
    JMP_ONLY = 0b00, JCOR_ONLY = 0b00, JNCOR_ONLY = 0b00, // (->const)
    CMP_ONLY = 0b00, // (none)

    CALL_BUILTIN_ONLY = 0b00, // (->const)
    
    CVAR_ONLY = 0b00, // (->const->const (sizeof(Var) && expectedPlace))
    DVAR_ONLY = 0b00, // (->var)

    MOV_REGTOCONST = 0b00,
    MOV_REGTOMEM = 0b01,
    MOV_REGTOREG = 0b10,
    MOV_MEMTOREG = 0b11,
} SubOpcode;
typedef enum OcrBinMapping {
    BIN_PLUS = 0,
    BIN_MINUS = 1,
    BIN_MULT = 2,
    BIN_DIV = 3,
    BIN_MOD = 4,
    BIN_OR = 5,
    BIN_AND = 6,
    BIN_SHL = 7,
    BIN_SHR = 8,
    BIN_XOR = 9,

    BIN_GRE = 10,
    BIN_SMA = 11,
    BIN_EQ = 12,
    BIN_SMEQ = 13,
    BIN_GREQ = 14,
} OcrBinMapping;

void decompile(const u8* src);

#define MAKE_OPCODE(full, sub) ((full << 2) | sub)

void interpret(const u8* src);
