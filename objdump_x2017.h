/*######################################################################

    USYD 2021 COMP2017

    ASSIGNMENT 2: VM & OBJDUMP

    OBJDUMP_X2017.H

    STUDENT: 500611960

    JAMIE MICKAILL

########################################################################*/



#ifndef OBJDUMP_X2017_H
#define OBJDUMP_X2017_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

/*######################################################################

    MACRO DEFINITIONS

    INIT_VAL: Initial value of ZERO

    PROG_STATE_COMPLETE_EXIT_0: Used to update program_state to 0 
        for return value upon successful execution of program

    PROG_STATE_COMPLETE: Used to update program state to 2 
        when return called from main

    PROG_STATE_RUNNING: Program state = 0 when executing without error

    PROG_STATE_ERROR: Program state = 1 when error encountered during ops

    SINGLE_BYTE_READ: Used to specify single byte fread (8 bits)

    SINGLE_BYTE_READ_BITS: Used to specify 8 bits in a byte

    DOUBLE_BYTE_READ: Used to specify double byte fread (16 bits)

    BYTE: 8bit Unsigned char type

    TWO_BYTES: 16bit Unsigned short int

    FIRST_2_BIT_MASK / FIRST_3_BIT_MASK / FIRST_5_BIT_MASK / TOTAL_8_BIT_MASK:
        Bit masks used for bitwise AND operations when reading binary
    
    SIZE_OF_OPCODE: instruction opcode size value in bits (3)

    SIZE_OF_TYPE: instruction size value in bits (2)

    SIZE_OF_VALUE: instruction value size in bits (8)

    SIZE_OF_SYMBOL: instruction symbol size in bits (5)

    SIZE_OF_HEADER: function header value size in bits (3)

    SIZE_OF_INSTRUCTION: instruction size value in bits (5)

    SIZE_OF_REG: Register address size value in bits (3)

    SIZE_OF_TYPE_STR: Size of type String in bits (4)

    MAX_NUMBER_OF_FUNCTIONS: The maximum number of allocable functons 
    with 3bit label, 2^3

    MAX_NUMBER_OF_INSTRUCTIONS: The maximum number of instructions 
    with 5bit label, 2^5

    SUCCESSFUL_SEEK: 0 returned upon successful seek

    FAILED_READ: 0 returned upon failed read

########################################################################*/


#define INIT_VAL (0)
#define PROG_STATE_COMPLETE_EXIT_0 (0)
#define PROG_STATE_COMPLETE (2)
#define PROG_STATE_RUNNING (0)
#define PROG_STATE_ERROR (1)
#define SINGLE_BYTE_READ (1)
#define SINGLE_BYTE_READ_BITS (8)
#define DOUBLE_BYTE_READ (2)
#define DOUBLE_BYTE_READ_BITS (16)
#define BYTE unsigned char
#define TWO_BYTES unsigned short int
#define FIRST_2_BIT_MASK (0x03)
#define FIRST_3_BIT_MASK (0x07)
#define FIRST_5_BIT_MASK (0x1F)
#define TOTAL_8_BIT_MASK (0xFF)
#define SIZE_OF_OPCODE (3)
#define SIZE_OF_TYPE (2)
#define SIZE_OF_VALUE (8)
#define SIZE_OF_SYMBOL (5)
#define SIZE_OF_HEADER (3)
#define SIZE_OF_INSTRUCTION (5)
#define SIZE_OF_REG (3)
#define SIZE_OF_TYPE_STR (4)
#define MAX_NUMBER_OF_FUNCTIONS (8)
#define MAX_NUMBER_OF_INSTRUCTIONS (32)
#define SUCCESSFUL_SEEK (0)
#define FAILED_READ (0)

/*#############################################################################

    Struct Definitions

#############################################################################*/

union VALUE {

    /*########################################################################

        Union containing instruction value based on given type
            1) 1 Byte Integer Value
            2) 3 Bit Register Address
            3) 1 Byte Stack Symbol
            4) 1 Byte Pointer Value

    ########################################################################*/

    BYTE value;
    unsigned int reg_addr: SIZE_OF_REG;
    BYTE stack_symbol;
    BYTE pointer_value;

};

struct instruction {

    /*######################################################################

        Instruction struct -> Holds a single instruction for a function 

            1) 1 Byte opcode
            2) String value instruction value type 1
            3) Union containing instruction value 1
            4) String value instruction value type 2
            5) Union containing instruction value 2

    ######################################################################*/

    BYTE opcode;
    char type1[SIZE_OF_TYPE_STR];
    union VALUE val1;
    char type2[SIZE_OF_TYPE_STR];
    union VALUE val2;

};

struct function {

    /*#####################################################################

        Function struct -> A single function containing multiple instructions
            1) 3 bit Function header
            2) 5 bit instruction count value
            3) Instruction pointer array

    ######################################################################*/

    unsigned int header: SIZE_OF_HEADER;
    unsigned int number_of_instructions: SIZE_OF_INSTRUCTION;
    struct instruction ** instructions;

};

/*#############################################################################

    Function Declarations

#############################################################################*/

struct instruction* init_instruction () ;

struct instruction** init_instructions_pointer_array (  
    unsigned int number_of_instructions) ;

struct function* init_function (unsigned int number_of_instructions_in, 
    struct instruction** instructions_in);

struct function** init_functions_array () ;

char get_symbol (int i, unsigned char * return_value) ;

void assign_stack_symbol (char * c, char* used_symbol_values,
    int* num_symbols_used) ;

char* get_opcode_string(BYTE curr_opcode);

void update_value(BYTE* curr_location_in_byte, BYTE *buffer_in, 
    TWO_BYTES *buffer2_in, FILE* binary_file, BYTE* value_to_be_set, 
    BYTE mask, BYTE size_in_bits, BYTE *offset, unsigned char * return_value) ;

void get_next_instruction(BYTE *number_of_instructions, BYTE* curr_location_in_byte, BYTE *buffer, 
    TWO_BYTES *buffer2, FILE* binary_file, BYTE *offset, unsigned char * return_value, 
    unsigned int* i, struct function* functionA);

struct function** get_function_array(char* filename, 
    unsigned char * return_value) ;

void print_instructions(struct function** functions_array) ;

void free_the_malloc(struct function** function_array) ;

#endif