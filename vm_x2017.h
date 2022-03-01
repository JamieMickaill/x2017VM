/*######################################################################

    USYD 2021 COMP2017

    ASSIGNMENT 2: VM & OBJDUMP

    VM_x2017.H

    STUDENT: 500611960

    JAMIE MICKAILL

########################################################################*/

#ifndef VM_X2017_H
#define VM_X2017_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include "objdump_x2017.h"

/*######################################################################

    MACRO DEFINITIONS

    STACK_SIZE: The maximum RAM size allowable according to specifications

    PROG_MEM_SIZE: The maximum number of instructions the VM can handle 
        (2^3 Functions * 2^5 instructions)

    NUMBER_OF_REGISTERS: The number of allowable registers

    MAX_NUMBER_OF_SYMBOLS: The maximum number of stack symbols 
        possible with 5 bits, 2^5

    MIN_STACK_ADDRESS: The minimum address allowable on the stack 
        (no negative indexing)

    NUMBER_OF_REGISTERS_PUSHED_FROM_CALL: 
        number of registers pushed onto the stack when CALL

########################################################################*/

#define STACK_SIZE (256)
#define PROG_MEM_SIZE (256)
#define NUMBER_OF_REGISTERS (8)
#define MAX_NUMBER_OF_SYMBOLS (32)
#define MIN_STACK_ADDRESS (0)
#define NUMBER_OF_REGISTERS_PUSHED_FROM_CALL (3)

enum REG{

/* ###########################################################################

    REGISTERS

    R00 -> Used by program
    R01 -> Used by program
    R03 -> Used by program
    R04 -> Stack Pointer
    R05 -> Frame Pointer
    R06 -> Program State
    R07 -> Program Counter

 ###########################################################################*/

    R00,
    R01,
    R02,
    R03,
    STK_PTR,
    FRAME_PTR,
    PROG_STATE,
    PC,
};

/*###########################################################################

    MAIN HELPER FUNCTION DESCRIPTIONS

############################################################################ */

void underflow_overflow_check(unsigned char stack_pointer, BYTE* registers);

unsigned char find_entry_point(BYTE function_label, 
    struct function** function_array, BYTE* registers);

void populate_instruction_array(struct instruction** program_code, 
    struct function** function_array) ;

BYTE get_stack_address_of_symbol(char symbol, BYTE frame_pointer, 
    BYTE* registers) ;

void free_the_mallocs(struct function** function_array, 
    struct instruction** program_code);

/* ###########################################################################

    MACHINE FUNCTIONS

############################################################################*/

void MOVE(struct instruction *curr_instruction, BYTE* stack, 
    BYTE* registers) ;

void CALL(struct instruction *curr_instruction,  
    BYTE* stack, BYTE*registers, struct function ** function_array) ;

void RETURN(struct instruction *curr_instruction,  
    BYTE* stack, BYTE*registers) ;

void REFERENCE(struct instruction *curr_instruction,  
    BYTE* stack, BYTE*registers) ;

void ADD(struct instruction *curr_instruction, BYTE *registers);
void PRINT(struct instruction *curr_instruction,  
    BYTE* stack, BYTE*registers) ;

void NOT(struct instruction *curr_instruction, BYTE *registers);

void EQU(struct instruction *curr_instruction, BYTE *registers);

void execute_instruction(struct instruction *curr_instruction, BYTE curr_opcode,
    BYTE* stack, BYTE* registers, struct function** function_array) ;
    
#endif