/*######################################################################

    USYD 2021 COMP2017

    ASSIGNMENT 2: VM & OBJDUMP

    VM_x2017_MAIN.c

    STUDENT: 500611960

    JAMIE MICKAILL

########################################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "vm_x2017.h"

/* ###########################################################################

    MAIN FUNCTION

############################################################################*/

int main(int argc, char **argv){
    
    /* #####################################################################

       1) Set program running flag
       2) Create RAM/stack array to STACK_SIZE 256 Bytes
       3) Initialize 8 * 1 Byte Registers
            R00 = Program use
            R01 = Program use
            R02 = Program use
            R03 = Program use
            R04 = Stack Pointer
            R05 = Frame Pointer
            R06 = Machine State Register
            R07 = Program Counter
       4) Initialize function array, retrieving instructions read from binary
       5) Initialize 256Byte instruction struct array (2^3 * 2^5)
       6) Copy instructions from function array to contiguous instruction array
       7) Set Program Counter to start of Function 0
       8) Begin Execution of FUNCTION 0, continue until PROG_STATE changes
            PROG_STATE 0 = Running / Finished Execution without error
            PROG_STATE 1 = ERROR
        9) Free all memory allocations
        10) Determine exit code based on Machine State Register
        11) Safely Exit.

    #######################################################################*/

    BYTE stack[STACK_SIZE] = {INIT_VAL};
    BYTE registers[NUMBER_OF_REGISTERS] = {INIT_VAL};

    struct function** function_array = 
        get_function_array(argv[1],&registers[PROG_STATE]);

    struct instruction **program_code = 
        init_instructions_pointer_array(PROG_MEM_SIZE);

    populate_instruction_array(program_code, function_array);

    registers[PC] = find_entry_point(INIT_VAL, function_array, registers);

    /* #####################################################################
 
        RUN LOOP
        1) Check for overflow/underflow of stack
        2) Point to current instruction in program memory
        3) Increment Program Counter
        4) Execute current instruction 

    #######################################################################*/

    while(registers[PROG_STATE] == PROG_STATE_RUNNING) {

        underflow_overflow_check(registers[STK_PTR], registers);

        struct instruction* curr_instruction = program_code[registers[PC]];

        registers[PC] +=1;

        execute_instruction(curr_instruction, 
        curr_instruction->opcode, stack, registers,function_array);

    }

    free_the_mallocs(function_array, program_code);

    if(registers[PROG_STATE] == PROG_STATE_COMPLETE){
        registers[PROG_STATE] = PROG_STATE_COMPLETE_EXIT_0;
    }

    exit(registers[PROG_STATE]);

}

