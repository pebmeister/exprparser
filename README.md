
**Version 1.0**

---

## Table of Contents

1. [Introduction](#introduction)
2. [Command Line Usage](#command-line-usage)
3. [Source File Format](#source-file-format)
4. [Numbers and Literals](#numbers-and-literals)
5. [Expressions](#expressions)
6. [Labels and Symbols](#labels-and-symbols)
7. [Addressing Modes](#addressing-modes)
8. [Assembler Directives](#assembler-directives)
9. [Macros](#macros)
10. [Conditional Assembly](#conditional-assembly)
11. [Loops](#loops)
12. [Complete Instruction Reference](#complete-instruction-reference)

---

## Introduction

PASM is a modern, feature-rich assembler for the MOS 6502 and WDC 65C02 microprocessors. It supports the full instruction set of both processors, including illegal/undocumented 6502 opcodes, and provides powerful macro and conditional assembly capabilities.

### Features

- Full 6502 instruction set support
- Extended 65C02 instruction set support
- Illegal/undocumented 6502 opcode support (optional)
- Powerful expression evaluation with C-style operators
- Macro definitions with parameter substitution
- Conditional assembly (`.if`, `.ifdef`, `.ifndef`)
- Loop constructs (`.do`/`.while`)
- Local and anonymous labels
- Multiple input file support
- Include file directive
- Commodore 64 PRG file output format

---

## Command Line Usage

```
pasm inputfile1 [inputfile2 ...] [options]
```

### Options

| Option | Description |
|--------|-------------|
| `-h` | Print help. |
| `-ast` | Print the Abstract Syntax Tree. |
| `-v` | Enable verbose mode. (displays symbol table) |
| `-all` | Show all symbols in listing. By default only used symbols are listed.|
| `-c64` | Commodore64 program mode. Set first 2 bytes as load address.|
| `-o <file>` | Specify output filename |
| `-65c02` | Enable 65C02 extended instruction set |
| `-il` | Allow illegal/undocumented 6502 instructions |
| `-nowarn` | Suppress warning messages |
| `-li` | List all valid instructions with addressing modes and cycle counts |

### Examples

```bash
# Basic assembly
pasm program.asm -o program.bin

# 65C02 assembly with verbose output
pasm program.asm -65c02 -v -o program.bin

# Commodore 64 PRG file
pasm game.asm -c64 -o game.prg

# Multiple source files
pasm main.asm utils.asm data.asm -o program.bin
```

---

## Source File Format

Source files are plain text files with one statement per line. The general format of a line is:

```
[label[:]] [instruction/directive [operands]] [; comment]
```

All components are optional. Blank lines and comment-only lines are permitted.

### Case Sensitivity

- **Instructions and directives**: Case-insensitive (`LDA`, `lda`, and `Lda` are equivalent)
- **Symbols and labels**: Case-insensitive (`MYVAR`, `MyVar`, and `myvar` refer to the same symbol)

### Comments

Comments begin with a semicolon (`;`) and extend to the end of the line:

```asm
LDA #$00        ; Load accumulator with zero
; This entire line is a comment
```

---

## Numbers and Literals

### Numeric Formats

| Format | Prefix | Example | Value |
|--------|--------|---------|-------|
| Decimal | (none) | `255` | 255 |
| Hexadecimal | `$` | `$FF` | 255 |
| Binary | `%` | `%11111111` | 255 |
| Character | `'c'` or `"c"` | `'A'` | 65 |

### Hexadecimal with Spaces

For readability, hexadecimal and binary numbers may contain spaces between digit groups:

```asm
.BYTE $01 02 03 04      ; Equivalent to $01, $02, $03, $04
MASK = %1111 0000       ; Equivalent to %11110000
```

### Text Strings

Text strings are enclosed in single or double quotes:

```asm
.BYTE "Hello, World!"
.BYTE 'SCORE: '
```

---

## Expressions

PASM supports a rich expression syntax with C-style operators. Expressions are evaluated as signed integers.

### Operators (by precedence, highest first)

| Precedence | Operators | Description |
|------------|-----------|-------------|
| 1 | `( )` | Parentheses |
| 2 | `-` `+` `~` | Unary minus, plus, one's complement |
| 3 | `*` `/` `%` | Multiply, divide, modulo |
| 4 | `+` `-` | Add, subtract |
| 5 | `<<` `>>` | Shift left, shift right |
| 6 | `<` `>` `<=` `>=` | Relational comparison |
| 7 | `==` `!=` `<>` | Equality, inequality |
| 8 | `&` | Bitwise AND |
| 9 | `^` | Bitwise XOR |
| 10 | `\|` | Bitwise OR |
| 11 | `&&` | Logical AND |
| 12 | `\|\|` | Logical OR |

### Special Symbols in Expressions

| Symbol | Meaning |
|--------|---------|
| `*` | Current program counter (when not an operator) |

### Expression Examples

```asm
VALUE = 10 * 5 + 3          ; VALUE = 53
MASK = $FF & %11110000      ; MASK = $F0
OFFSET = LABEL - *          ; Distance from current PC to LABEL
HIBYTE = ADDRESS >> 8       ; Extract high byte
LOBYTE = ADDRESS & $FF      ; Extract low byte
FLAG = VALUE >= 100         ; FLAG = 0 (false) or 1 (true)
```

---

## Labels and Symbols

### Standard Labels

Labels mark locations in code and can be defined with or without a trailing colon:

```asm
START:  LDA #$00        ; Label with colon
LOOP    DEX             ; Label without colon
        BNE LOOP
```

### Symbol Equates

Symbols can be assigned constant values using the `=` operator:

```asm
SCREEN = $0400
BORDER_COLOR = $D020
MAX_LIVES = 3
```

### Program Counter Assignment

The program counter can be set directly:

```asm
* = $C000               ; Set PC to $C000
```

### Local Labels

Local labels begin with `@` and are scoped between two global labels:

```asm
ROUTINE1:
        LDX #$10
@LOOP:  DEX
        BNE @LOOP       ; References local @LOOP
        RTS

ROUTINE2:
        LDX #$20
@LOOP:  DEX             ; Different @LOOP, local to ROUTINE2
        BNE @LOOP
        RTS
```

When a new global label is defined, all previous local labels go out of scope.

### Anonymous Labels

Anonymous labels provide a lightweight way to create short-range branch targets:

**Definition:**
- `+:` defines a forward anonymous label
- `-:` defines a backward anonymous label

**Reference:**
- `+` references the next forward anonymous label
- `++` references the second forward anonymous label
- `-` references the previous backward anonymous label
- `--` references the second-to-last backward anonymous label

```asm
-:      LDA DATA,X
        BEQ +           ; Branch to next + label
        JSR PROCESS
+:      INX
        CPX #$10
        BNE -           ; Branch to previous - label
```

Multiple levels of anonymous references:

```asm
-:      ; First backward label
-:      ; Second backward label
        BEQ --          ; Branch to first backward label
        BNE -           ; Branch to second backward label
```

---

## Addressing Modes

PASM automatically selects zero-page addressing when operands fit in a single byte and the instruction supports it.

| Mode | Syntax | Example | Description |
|------|--------|---------|-------------|
| Implied | `OPC` | `NOP` | No operand |
| Accumulator | `OPC A` | `ASL A` | Operates on accumulator |
| Immediate | `OPC #val` | `LDA #$FF` | 8-bit immediate value |
| Absolute | `OPC addr` | `LDA $1234` | 16-bit address |
| Zero Page | `OPC zp` | `LDA $80` | 8-bit zero page address |
| Absolute,X | `OPC addr,X` | `LDA $1234,X` | Absolute indexed by X |
| Absolute,Y | `OPC addr,Y` | `LDA $1234,Y` | Absolute indexed by Y |
| Zero Page,X | `OPC zp,X` | `LDA $80,X` | Zero page indexed by X |
| Zero Page,Y | `OPC zp,Y` | `LDX $80,Y` | Zero page indexed by Y |
| Indirect | `OPC (addr)` | `JMP ($FFFC)` | Indirect addressing |
| Indexed Indirect | `OPC (zp,X)` | `LDA ($80,X)` | Indexed indirect (X) |
| Indirect Indexed | `OPC (zp),Y` | `LDA ($80),Y` | Indirect indexed (Y) |
| Relative | `OPC label` | `BNE LOOP` | Branch relative |
| ZP Relative | `OPC zp,label` | `BBR0 $10,DONE` | 65C02 bit branch |

---

## Assembler Directives

### `.ORG` - Set Origin

Sets the program counter to a specific address:

```asm
.ORG $C000              ; Code starts at $C000
```

### `.BYTE` / `.BYT` - Define Bytes

Stores one or more bytes in memory:

```asm
.BYTE $00, $01, $02, $03
.BYTE "Hello", 0        ; String with null terminator
.BYT 'A', 'B', 'C'
```

### `.WORD` / `.WRD` - Define Words

Stores one or more 16-bit words (little-endian):

```asm
.WORD $1234             ; Stores $34, $12
.WORD LABEL1, LABEL2    ; Address table
```

### `.DS` - Define Storage

Reserves a block of bytes (advances PC without generating output):

```asm
BUFFER: .DS 256         ; Reserve 256 bytes
```

### `.FILL` - Fill Memory

Fills a region with a repeated byte value:

```asm
.FILL $EA, 16           ; 16 bytes of NOPs ($EA)
.FILL 0, 256            ; 256 zero bytes
```

### `.INCLUDE` / `.INC` - Include File

Includes another source file:

```asm
.INCLUDE "macros.asm"
.INC "data.asm"
```

---

## Macros

Macros allow you to define reusable code sequences with parameter substitution.

### Defining Macros

```asm
.MACRO NAME
    ; Macro body
    ; Use \1, \2, \3... for parameters
.ENDM
```

Or using the short forms:

```asm
.MAC NAME
    ; body
.ENDMACRO
```

### Macro Parameters

Parameters are referenced using `\1`, `\2`, `\3`, etc.:

```asm
.MACRO LOAD_XY
        LDX #\1
        LDY #\2
.ENDM
```

### Calling Macros

Macros are called by name, optionally followed by arguments:

```asm
LOAD_XY 10, 20          ; Expands to LDX #10 / LDY #20
LOAD_XY $FF, $00
```

For macros without arguments:

```asm
.MACRO SAVE_REGS
        PHA
        TXA
        PHA
        TYA
        PHA
.ENDM

SAVE_REGS               ; Call with no arguments
SAVE_REGS()             ; Alternative syntax
```

### Macro Example

```asm
; 16-bit increment macro
.MACRO INC16
        INC \1
        BNE +
        INC \1+1
+:
.ENDM

COUNTER: .WORD 0

        INC16 COUNTER   ; Increment 16-bit counter
```

---

## Conditional Assembly

### `.IF` / `.ELSE` / `.ENDIF`

Conditional assembly based on expression evaluation:

```asm
DEBUG = 1

.IF DEBUG
        JSR PRINT_STATUS    ; Only assembled if DEBUG != 0
.ELSE
        NOP                 ; Assembled if DEBUG == 0
.ENDIF
```

### `.IFDEF` / `.IFNDEF`

Test whether a symbol is defined:

```asm
.IFDEF USE_SOUND
        JSR INIT_SOUND
.ENDIF

.IFNDEF SKIP_INTRO
        JSR SHOW_INTRO
.ENDIF
```

### Nested Conditionals

Conditionals can be nested:

```asm
.IF PLATFORM == 1
    .IFDEF HAS_SID
        JSR PLAY_MUSIC
    .ENDIF
.ENDIF
```

---

## Loops

### `.DO` / `.WHILE`

Repeat a block of code while a condition is true. Useful with `.VAR` for assembly-time iteration:

```asm
.VAR I = 0
.DO
        .BYTE I
.VAR I = I + 1
.WHILE I < 10
```

This generates: `.BYTE 0, 1, 2, 3, 4, 5, 6, 7, 8, 9`

### `.VAR` - Assembly-Time Variables

Variables differ from equates in that they can be modified during assembly:

```asm
.VAR COUNT = 0          ; Define variable
COUNT = COUNT + 1       ; Modify variable
```

### Loop Example: Generate Sine Table

```asm
; Generate 256-byte sine table (conceptual example)
.VAR I = 0
SINE_TABLE:
.DO
        .BYTE (I * I) / 256     ; Simplified calculation
        I = I + 1
.WHILE I < 256
```

---

## Complete Instruction Reference

Use the `-li` command line option to display all supported instructions with their addressing modes and cycle counts:

```bash
pasm -li
```

### Standard 6502 Instructions

| Mnemonic | Description |
|----------|-------------|
| `ADC` | Add with Carry |
| `AND` | Logical AND |
| `ASL` | Arithmetic Shift Left |
| `BCC` | Branch if Carry Clear |
| `BCS` | Branch if Carry Set |
| `BEQ` | Branch if Equal (Zero) |
| `BIT` | Bit Test |
| `BMI` | Branch if Minus |
| `BNE` | Branch if Not Equal |
| `BPL` | Branch if Plus |
| `BRK` | Break |
| `BVC` | Branch if Overflow Clear |
| `BVS` | Branch if Overflow Set |
| `CLC` | Clear Carry |
| `CLD` | Clear Decimal |
| `CLI` | Clear Interrupt Disable |
| `CLV` | Clear Overflow |
| `CMP` | Compare Accumulator |
| `CPX` | Compare X Register |
| `CPY` | Compare Y Register |
| `DEC` | Decrement Memory |
| `DEX` | Decrement X |
| `DEY` | Decrement Y |
| `EOR` | Exclusive OR |
| `INC` | Increment Memory |
| `INX` | Increment X |
| `INY` | Increment Y |
| `JMP` | Jump |
| `JSR` | Jump to Subroutine |
| `LDA` | Load Accumulator |
| `LDX` | Load X Register |
| `LDY` | Load Y Register |
| `LSR` | Logical Shift Right |
| `NOP` | No Operation |
| `ORA` | Logical OR |
| `PHA` | Push Accumulator |
| `PHP` | Push Processor Status |
| `PLA` | Pull Accumulator |
| `PLP` | Pull Processor Status |
| `ROL` | Rotate Left |
| `ROR` | Rotate Right |
| `RTI` | Return from Interrupt |
| `RTS` | Return from Subroutine |
| `SBC` | Subtract with Carry |
| `SEC` | Set Carry |
| `SED` | Set Decimal |
| `SEI` | Set Interrupt Disable |
| `STA` | Store Accumulator |
| `STX` | Store X Register |
| `STY` | Store Y Register |
| `TAX` | Transfer A to X |
| `TAY` | Transfer A to Y |
| `TSX` | Transfer SP to X |
| `TXA` | Transfer X to A |
| `TXS` | Transfer X to SP |
| `TYA` | Transfer Y to A |

### 65C02 Extended Instructions (requires `-65c02`)

| Mnemonic | Description |
|----------|-------------|
| `BRA` | Branch Always |
| `PHX` | Push X Register |
| `PHY` | Push Y Register |
| `PLX` | Pull X Register |
| `PLY` | Pull Y Register |
| `STZ` | Store Zero |
| `TRB` | Test and Reset Bits |
| `TSB` | Test and Set Bits |
| `BBR0`-`BBR7` | Branch on Bit Reset |
| `BBS0`-`BBS7` | Branch on Bit Set |
| `RMB0`-`RMB7` | Reset Memory Bit |
| `SMB0`-`SMB7` | Set Memory Bit |
| `STP` | Stop Processor |
| `WAI` | Wait for Interrupt |

### Illegal/Undocumented Instructions (requires `-il`)

| Mnemonic | Description |
|----------|-------------|
| `SLO` | ASL + ORA |
| `RLA` | ROL + AND |
| `SRE` | LSR + EOR |
| `RRA` | ROR + ADC |
| `SAX` | Store A AND X |
| `LAX` | Load A and X |
| `DCP` | DEC + CMP |
| `ISC` | INC + SBC |
| `ANC` | AND + set Carry |
| `ALR` | AND + LSR |
| `ARR` | AND + ROR |
| `XAA` | TXA + AND |
| `AXS` | A AND X - operand |
| `AHX` | Store A AND X AND high byte |
| `SHY` | Store Y AND high byte |
| `SHX` | Store X AND high byte |
| `TAS` | Transfer A AND X to SP |
| `LAS` | Load A, X, SP with SP AND memory |
| `USBC` | Unofficial SBC |

---

## Sample Program

```asm
; Hello World for Commodore 64
; Assemble with: pasm hello.asm -c64 -o hello.prg

        .ORG $0801

; BASIC stub: 10 SYS 2062
        .WORD NEXT_LINE
        .WORD 10            ; Line number
        .BYTE $9E           ; SYS token
        .BYTE "2062", 0     ; Address as string
NEXT_LINE:
        .WORD 0             ; End of BASIC

; Main program starts here
START:
        LDX #0
@LOOP:
        LDA MESSAGE,X
        BEQ DONE
        JSR $FFD2           ; CHROUT
        INX
        BNE @LOOP
DONE:
        RTS

MESSAGE:
        .BYTE "HELLO, WORLD!", 13, 0
```

---

## Error Messages

PASM provides descriptive error messages including file name and line number:

```
Error: Unknown opcode 'LDZ' at hello.asm:15
Error: Opcode 'LDA' does not support addressing mode Indirect at test.asm:23
Error: Division by zero at math.asm:10
Error: Relative branch target out of range (-128 to 127) at game.asm:156
```

---

## Tips and Best Practices

1. **Use local labels** for loop targets to avoid name collisions
2. **Use anonymous labels** (`+`/`-`) for very short forward/backward branches
3. **Organize code with includes** to separate data, macros, and main logic
4. **Use macros** for common code patterns
5. **Use conditional assembly** to build debug and release versions from the same source
6. **Use `.VAR` with `.DO`/`.WHILE`** for generating data tables at assembly time

---

*PASM - Written by Paul Baxter*

