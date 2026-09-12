# The VGOD

The VGOD is a virtual graphics output device used for RivOS. This will mainly be a datasheet for the raw device, not a assembly documentation. For that look [here](rap.md)

## Why use the VGOD?

The VGOD is meant to be a high-ish level graphics output program that still runs fast and still covers modern needs.

NOTE: The VGOD is little endian

## Specsheet

### Basics of a file

Layout: 

| Offset | Type |  Use |
|:------|:------:|-------:|
| 0x00     | char[4]      | mgic="vgod"      |
| 0x04     | u32      |  virtPos     |
| 0x08     | u8[2]    | vers(major=[0], minor=[1]) |
| 0x0A     | char[8] |  reserved    |

NOTE: Current version = 0.0
NOTE: The VGOD can also be used outside a shader.

### Instructions

This will note all instructions a VGOD can perform.
| InstrByte | Operands | ASM | Description | Side effects |
|-------:|-----:|-------|-------------|----|
| 0x00 | [] | NOP | Does nothing: just waits | |
| 0x01 | [type: DoubleValType, dest, src] | MOV | Sets a valueMoveType(dest) to a valueType(src) | INT 0x03 |
| 0x02 | [loc: imd] | CALL | Calls loc | |
| 0x03 | [] | RET | Returns from a CALL | |
| 0x04 | [loc: imd] | JMP | Unconditionaly jumps to a loc | |
| 0x05 | [type: DoubleValType, x, y] | CMP | Sets OCR to comparison of x, y | OCR |
| 0x06 | [loc: imd] | BEQ | Branches to a loc if OCR.eq is 1 | |
| 0x07 | [loc: imd] | BNE | Branches to a loc if OCR.eq is 0 | |
| 0x08 | [loc: imd] | BSM | Branches to a loc if OCR.sm is 1 | |
| 0x09 | [loc: imd] | BGR | Branches to a loc if OCR.gr is 1 | |
| 0x0A | [loc: imd] | BSE | Branches to a loc if OCR.sm or OCR.eq is 1 | |
| 0x0B | [loc: imd] | BGE | Branches to a loc if OCR.gr or OCR.eq is 1 | |
| 0x0C | [loc: imd] | BOV | Branches to a loc if ACR.ov is 1 | |
| 0x0D | [type: Type, reg: Register, y: type] | ADD | Performs a ADD operation on reg and y. Output is in OCR. NOTE: Do not use any Branch instruction besides BOV as OCR is now garbage (for branching at least) | OCR, ACR.ov |
| 0x0E | [type: Type, reg: Register, y: type] | SUB | Subtracts y from reg into OCR. For further note(s), look in ADD | OCR, AVR.ov |
| 0x0F | [type: Type, reg: Register, y: type] | MUL | Performs reg * y into OCR. For further note(s), look in ADD. | OCR, AVR.ov |
| 0x10 | [type: Type, reg: Register, y: type] | DIV | Performs reg / y into OCR. if y == 0 INT 0x00 will fire. | OCR, ACR.ov, ACR, INT 0x00 |
| 0x11 | [type: Type, reg: Register, y: type] | OR | Performs reg OR y into OCR. | OCR |
| 0x12 | [type: Type, reg: Register, y: type] | AND | Performs reg & y into OCR. | OCR |
| 0x13 | [reg: Register] | NOT | Performs !reg into OCR. | OCR |
| 0x14 | [type: Type, reg: Register, y: type] | SHL | Performs reg << y into OCR. | OCR, ACR.ov |
| 0x15 | [type: Type, reg: Register, y: type] | SHR | Performs reg >> y into OCR | OCR, ACR.ov |
| 0x16 | [intNum: int, loc: imd] | SIH | Sets the interrupt handler of intNum to loc | INT_TABLE, Possible INT 0x01 if !mem.isValid() |
| 0x17 | [type: Type, v: type] | RVL | Returns the value v. | |
| 0x18 | [msh: int, loc: imd] | SFS | Sets fragment shader for msh to loc. Msh=0 means full screen (excluding meshes) | Possible 0x02 if !mem.isValid() |
| 0x19 | [str: mem] | CMS | Creates a mesh. Looks in filepath at str. Str should be null-terminated like a C-str in ASCII. | | Possible INT 0x04 if anything is invalid (file doesn't exist, unexpected format etc.)
| 0x20 | [] | GMC | Gets the mesh color at pixel coordinate current. | Loads it in | Possible INT 0x05 if no mesh exists.
| 0x21 | [char: int] | OUT | Outputs char. This will not be shown unless the parent caller decides it should be. Can be used for logs etc. | |### Types

### Types

As you might have seen, there were a lot of types. Each one will be explained here. 

* DoubleValType: 1 byte, a bitpacked byte that has a layout like:
    | Offset (In bits) | Description |
    |------|-------------|
    | 0x00 | 4 bits containing a regular Type. First operand uses this type. |
    | 0x04 | 4 bits containing a regular Type. Second operand uses this type.     |
* Type. 1 byte, this can be either
    - (0x00) Mem. 4 bytes, value at a memory location.
    - (0x01) Register. 1 byte, this is a register. A list can be found below here.
    - (0x02) Int. 4 bytes, This is just a immediate value.


### Registers 

XPOS = X position of current shader. If not inside a shader, will throw INT 0x03
YPOS = Y position of current shader. If not inside a shader, will throw INT 0x03.
FCR = Fragment Control Register
OCR = Operator Control Register.
ACR = Arithmetic control register, decides if should be signed/unsigned etc.
GCR = General Control Register. 
GP1 = General purpose 1
GP2 = General purpose 2

#### Bit layouts
Note that all registers are 32bit. 
'b' is defined as bit.
'b(n)' is defined as n bits.
Integers are shown in rust-style naming.

* XPOS: u32
* YPOS: u32
* FCR: u32
    - u8 CURR_MONITOR (read=get current monitorID, write=set monitorID) | b KILL (read=UB, write=stop fragment shader) | u8 MSH_ID (read=mesh id, write=UB) | Reserved
* OCR: u32
    - Result of BIN_OP
    - b GE | b GR | b SM | b SME | b EQ | Reserved 
* ACR: u32
    - Invalid math op (E.G. dividing by 0) will have it's first operand (x) reported here.
    - b INT_OV | b UINT_OV | Reserved
* GCR: u32
    - Reserved
    - If in interrupt contains original location interrupt was triggered in.
* GP1: u32
    - General purpose
* GP2: u32
    - General purpose

\* Any form of UB will be responded to by doing nothing in a official RivOS VGOD implementation, however this is not part of the main spec. 

###  MMIO
* 0x1000: u16[2]: M_RES_INFO: 0x1000[0] is horizontal, 0x1000[1] is vertical resolution
* 0x1004: u32: M_NAME_PTR: Pointer to the monitor name string
* 0x1008: u32: M_NAME_LEN: Length of the monitor name string
* 0x100B: u8: BITNESS: Bitness of the VGOD (should either be 32 or 64)
* 0x100C: u8[2]: VERSION: Version of the VGOD. 0x100C[0] is major, 0x100C[1] is minor.
* 0x100E: u8[2]: KINFO_VERS: Version of the kernel. Loc[0] is major, loc[1] is minor.
* 0x1010: u8: NATIVE: If this is a native VGOD implementation should be '1'
* 0x1011: u16[2] W_RES_INFO 0x1011[0] is horizontal, 0x1011[1] is vertical.
* \*-0x2000: Reserved and should not be used.

### Interrupt table
* INT 0x00: division by 0
* INT 0x01: invalid interrupt handler
* INT 0x02: Invalid (fragment) shader handler
* INT 0x03: Invalid access of XPOS or YPOS register outside shader.
* INT 0x04: Failure in CMS
* INT 0x05: Failure in GMC (No mesh exists right now)
* INT 0x06: Undefined opcode.


