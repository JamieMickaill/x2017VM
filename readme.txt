/*######################################################################

    USYD 2021 COMP2017

    ASSIGNMENT 2: VM & OBJDUMP

    README.TXT

    STUDENT: 500611960

    JAMIE MICKAILL

########################################################################*/

This program is a basic Virtual Machine Emulator capable of implementing 
and performing simple operations.

This virtual machine can store and reference variables 
and stack frame contexts and read a set of pseudo assembly 
instructions

This program consists of 2 main components, 
1) Objdump_x2017 , which interprets binary code into assembly instructions.
2) VM_x2017, which executes these instructions with a virtual RAM, Registers and program code.

USAGE: The program will take a single command line argument being the path to
the file containing your x2017 assembly code.

To run the Object Dump program
E.G. ./objdump_x2017 example_code.x2017

To run the VM
E.G. ./VM_x2017 example_code.x2017


VM components
• RAM - Contains 2^8 addresses of 1 byte each
• Register Bank - 8 registers of 1 byte each
• Program Code - Memory required to store the program to be executed.


x2017 Program code:
The virtual machine will be operating on a home brewed ‘x2017’ assembly language.
Each operation within ‘x2017’ contains an operation specifier code (op code) and takes zero, one or two arguments. 
Arguments are expressed as one of four different types, while operation codes are selected from a table

The arguments of each function precede the opcode and are expressed as follows:
e.g. ([Second Value][Second Type])([First Value][First Type])[Operation Code]

The type is a two bit field and specifies the type of the preceding value.

00 - value: 1 byte long. The value in the preceding 8 bits should be interpreted as a single byte
value.

01 - register address: 3 bits long. This address refers to one of the eight fixed registers

10 - stack symbol: 5 bits long. This refers to a particular symbol within the current stack frame.
A stack symbol is a value that is associated with an address on the stack

11 - pointer value: 5 bits long. This treats the contents of the address referred to by a particular 
symbol within the current stack frame as a variable. 
Pointers may reference variables on different stack frames.

Registers
    R00 -> Used by program
    R01 -> Used by program
    R03 -> Used by program
    R04 -> Stack Pointer
    R05 -> Frame Pointer
    R06 -> Program State
    R07 -> Program Counter


Opcodes:
000 - [MOV A B] - Copies the value at some point B in memory to another point A in memory
(register or stack). The destination may not be value typed.
001 - [CAL A] - Calls another function the first argument is a single byte (using the VALUE type)
containing the label of the calling function.
010 - [RET] - Terminates the current function, this is guaranteed to always exist at the end of each
function. There may be more than one RET in a function. If this function is the entry-point,
then the program terminates.
011 - [REF A B] - Takes a stack symbol B and stores its corresponding stack address in A.
100 - [ADD A B] - Takes two register addresses and ADDs their values, storing the result in the first
listed register.
101 - [PRINT A] - Takes any address type and prints the contents to a new line of standard output
as an unsigned integer.
110 - [NOT A] - Takes a register address and performs a bitwise not operation on the value at that
address. The result is stored in the same register
111 - [EQU A] - Takes a register address and tests if it equals zero. The value in the register will be
set to 1 if it is 0, or 0 if it is not. The result is stored in the same register.

Note:
* The state of the registers is preserved between CAL and RET operations.
* The first few bits of the file are padding bits to ensure that the total number of bits in the file accumulates to a whole number of bytes. The number of padding bits will always be strictly less than one
byte.

The file is broken up into a number of functions. Each function is defined with a three bit header
dictating the ’label’ of the function, and a five bit tail specifying the number of instructions in the
function. The function with the label 0 is the entry point and should be executed first.

[Padding bits]
[function label (3 bits)]
[OPCODE]
[OPCODE]
...
[RET]
[Number of instructions (5 bits)]
[function label (3 bits)]
[OPCODE]
[OPCODE]
...
[RET]
[Number of instructions (5 bits)]

EXAMPLE CODE
[function label (3 bits)]
[OPCODE]
[OPCODE]
...
[RET]
[Number of instructions (5 bits)]


EXAMPLE CODE:

FUNC LABEL 0
    MOV REG 0 VAL 3
    MOV REG 1 VAL 5
    ADD REG 0 REG 1
    MOV STK A REG 0
    RET

In Hex:
\x00\x03\x02\x01\x42\x82\x86\x04\x10\x45

In Binary:
0000000000000011000000100000000101000010
1000001010000110000001000001000001000101

See code comments for more detail on functionality.
Testing is run with "bash test.sh" -> See Testinfo doc.