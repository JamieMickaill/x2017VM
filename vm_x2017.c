/*######################################################################

    USYD 2021 COMP2017

    ASSIGNMENT 2: VM & OBJDUMP

    VM_x2017.C

    STUDENT: 500611960

    JAMIE MICKAILL

########################################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "vm_x2017.h"


/*###########################################################################

    MAIN HELPER FUNCTION DESCRIPTIONS

############################################################################ */

void underflow_overflow_check(unsigned char stack_pointer, BYTE* registers) {

    /*#######################################################################

    Underflow_Overflow_Check

    Checks stack pointer is in safe location before proceding with next opcode

    Input: stack_pointer, registers

    Output: Void

    #########################################################################*/

    if (stack_pointer >= STACK_SIZE-1) {
        fprintf(stderr,"Stack overflow imminent, Exiting gracefully\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;

    }
    else if (stack_pointer < MIN_STACK_ADDRESS) {
        fprintf(stderr,"Stack underflow imminent, Exiting gracefully\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;
    }

}

unsigned char find_entry_point(BYTE function_label, 
    struct function** function_array, BYTE* registers) {

    /* ###########################################################################

        Find_Entry_Point

        Given a function label value, iterates over array of functions,
        retrieving the index of the desired function for use with program_code.

        This value is multiplied by 32 to give the program_code index 
        of the entry opcode for the function. 

        If function is not found, program state register is updated to ERROR.

        Input: Function label (Unsigned char), Function array, Registers
        Output: Program_code index (unsigned char)

    #########################################################################*/

    //iterate over functions to find the given label, 
    //ensuring at least 1 valid instruction exists within the label
    for(int i=0; i<MAX_NUMBER_OF_FUNCTIONS; i++) {
        if (function_array[i]->header == function_label && 
        function_array[i]->number_of_instructions > INIT_VAL) {
            return i*MAX_NUMBER_OF_INSTRUCTIONS;
        }
    }

    //if Not found, update error state
    registers[PROG_STATE] = PROG_STATE_ERROR;
    return 33;

};

void populate_instruction_array(struct instruction** program_code, 
    struct function** function_array) {

    /* #######################################################################

        Populate_Instruction_Array

        Iterates over a given array of functions 
        and retrieves all valid instructions.

        These are copied to a program_code array, 
        for contiguous array access to all instructions

        This allows for indexed access to all instructions with the PC

        Input: program_code array, Function array, 
        Output: Void

    #########################################################################*/

    //Iterate over instruction array of each function with >0 instructions
    //copy instructions from function_array to contiguous program_code array
    int prog_code_index = 0;
    for(int i=0; i < MAX_NUMBER_OF_FUNCTIONS; i++) {
        for(int j = 0; j<function_array[i]->number_of_instructions; j++) {

            memcpy(program_code[prog_code_index+j],
            function_array[i]->instructions[j], sizeof(struct instruction));
        }

        prog_code_index += MAX_NUMBER_OF_INSTRUCTIONS;
    }
}

BYTE get_stack_address_of_symbol(char symbol, BYTE frame_pointer, 
    BYTE* registers) {

    /* ########################################################################

        Get_Stack_Address_Of_Symbol

        Updates stack pointer to point to a given symbol in current stack frame
        returns address of stack symbol

        REFERENCE:
        stackoverflow.com/questions/1469711/converting-letters-to-numbers-in-c

        Input: Function array, program_code array
        Output: Stack Pointer pointing at symbol in current frame

    ##########################################################################*/

    //needs to check whether symbol exists
    //Use ASCII values to determine correct offset
    BYTE symbol_offset = 0;
    if (symbol >= 'A' && symbol <= 'Z') {
        symbol_offset = symbol - 'A';
        }
    else if (symbol >= 'a' && symbol <= 'z') {
        symbol_offset = (symbol - 'a') +26;
        }
    else{
        fprintf(stderr, "Invalid Stack Symbol\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;
    }

    //set stack pointer to frame pointer 
    //+ number of stack symbols in use
    registers[STK_PTR] = 
    registers[FRAME_PTR] + symbol_offset + 1;

    //frame pointer + number of symbols already used = 
    //address of given symbol
    return registers[STK_PTR];

};

void free_the_mallocs(struct function** function_array, 
    struct instruction** program_code) {

    /*#######################################################################

    Free_the_Mallocs

    Frees all memory allocated within the program

    Input: program_code, function array

    Output: Void

    #########################################################################*/

    //Free the mallocs!

    for (int i=0; i < MAX_NUMBER_OF_FUNCTIONS; i++) {
        for (int j = 0; j<function_array[i]->number_of_instructions; j++) {
            free(function_array[i]->instructions[j]);
        }
        if (function_array[i]->number_of_instructions > 0) {
            free(function_array[i]->instructions);
        }
        free(function_array[i]);
        }

    for (int i=0; i<PROG_MEM_SIZE; i++) {
        free(program_code[i]);
    }

    free(program_code);
    free(function_array);
}

/* ###########################################################################

    MACHINE FUNCTIONS

############################################################################*/

void MOVE(struct instruction *curr_instruction, BYTE* stack, 
    BYTE* registers) {
    /* ########################################################################

        [MOV A B]

        Copies the value at some point B in memory to another point A in memory
        (register or stack). The destination may not be value typed.

        Addresses are retrieved from instruction,
        based on associated address type within instruction.

        Input: Current instruction (struct), Stack and Register arrays
        Output: Void

        Updates: State register if error encountered

    ##########################################################################*/

    // MOV VAL = INVALID!
    if ( !strncmp(curr_instruction->type1, "VAL", SIZE_OF_OPCODE) ) {
        fprintf(stderr,"ERROR: Unable to MOV to a value!\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;
        return;
    }

    // MOV REG
    else if ( !strncmp(curr_instruction->type1, "REG", SIZE_OF_OPCODE)) {

        // MOV REG X VAL X
        if ( !strncmp(curr_instruction->type2, "VAL", SIZE_OF_OPCODE)) {
            registers[curr_instruction->val1.reg_addr] = 
            curr_instruction->val2.value;
        }

        // MOV REG X REG X
        if ( !strncmp(curr_instruction->type2, "REG", SIZE_OF_OPCODE)) {
            registers[curr_instruction->val1.reg_addr] = 
            registers[curr_instruction->val2.reg_addr];
        }
        
        // MOV REG X PTR X
        if ( !strncmp(curr_instruction->type2, "PTR", SIZE_OF_OPCODE)) {
            BYTE stack_address = get_stack_address_of_symbol(
                curr_instruction->val2.pointer_value, 
                registers[FRAME_PTR],registers);

            registers[curr_instruction->val1.reg_addr]= 
            stack[stack[stack_address]];
        }

        // MOV REG X STK X
        if ( !strncmp(curr_instruction->type2, "STK", SIZE_OF_OPCODE)) {

            BYTE stack_address = get_stack_address_of_symbol(
                curr_instruction->val2.stack_symbol, 
                registers[FRAME_PTR],registers);

            registers[curr_instruction->val1.reg_addr] = stack[stack_address];
        }

    }

    // MOV STK X
    else if ( !strncmp(curr_instruction->type1, "STK", SIZE_OF_OPCODE)) {
        
        BYTE stack_address = get_stack_address_of_symbol(
            curr_instruction->val1.stack_symbol, 
            registers[FRAME_PTR],registers);

        // MOV STK X VAL X
        if ( !strncmp(curr_instruction->type2, "VAL", SIZE_OF_OPCODE)) {
            stack[stack_address] = curr_instruction->val2.value;
        }
        // MOV STK X REG X
        if ( !strncmp(curr_instruction->type2, "REG", SIZE_OF_OPCODE)) {
            stack[stack_address] = registers[curr_instruction->val2.reg_addr];
        }

        // MOV STK X STK X
        if ( !strncmp(curr_instruction->type2, "STK", SIZE_OF_OPCODE)) {
            BYTE stack_address2 = 
            get_stack_address_of_symbol(curr_instruction->val2.stack_symbol, 
            registers[FRAME_PTR],registers);

            stack[stack_address] = stack[stack_address2];
        }

        // MOV STK X PTR X
        if ( !strncmp(curr_instruction->type2, "PTR", SIZE_OF_OPCODE)) {
            BYTE stack_address2 = 
            get_stack_address_of_symbol(curr_instruction->val2.pointer_value, 
            registers[FRAME_PTR],registers);

            stack[stack_address] = stack[stack[stack_address2]];
        }

    }

    // MOV PTR X
    else if ( !strncmp(curr_instruction->type1, "PTR", SIZE_OF_OPCODE)) {

        BYTE stack_address = get_stack_address_of_symbol(
            curr_instruction->val1.pointer_value, 
        registers[FRAME_PTR],registers);


        // MOV PTR X VAL X
        if ( !strncmp(curr_instruction->type2, "VAL", SIZE_OF_OPCODE)) {           
            stack[stack[stack_address]] = 
            curr_instruction->val2.value;
        }

        // MOV PTR X REG X
        if ( !strncmp(curr_instruction->type2, "REG", SIZE_OF_OPCODE)) {         
            stack[stack[stack_address]] = 
            registers[curr_instruction->val2.reg_addr];
        }

        // MOV PTR X STK X
        if ( !strncmp(curr_instruction->type2, "STK", SIZE_OF_OPCODE)) {

            BYTE stack_address2 = get_stack_address_of_symbol(
                curr_instruction->val2.stack_symbol, 
            registers[FRAME_PTR],registers);
      
            stack[stack[stack_address]] = stack[stack_address2];
        }

        // MOV PTR X PTR X
        if ( !strncmp(curr_instruction->type2, "PTR", SIZE_OF_OPCODE)) {

            BYTE stack_address2 = get_stack_address_of_symbol(
                curr_instruction->val2.pointer_value, 
                registers[FRAME_PTR],registers);
           
            stack[stack[stack_address]] = stack[stack[stack_address2]];
        }
    }    
    return;
};

void CALL(struct instruction *curr_instruction,  
    BYTE* stack, BYTE*registers, struct function ** function_array) {
    
    /*########################################################################

        [CAL A]

        Calls another function 
        
        The first argument is a single byte (using the VALUE type)
        containing the label of the calling function.

        The state of the registers is preserved between CAL and RET operations

        1) Type check of instruction values
        2) Check function label is valid (0->7)
        3) Check call wont cause stack overflow
        4) Push previous stack frame pointer to stack
        5) Push previous stack pointer to stack
        6) Push return address from PC to stack
        7) Increment Frame and Stack pointers to new stack frame
        8) Update Program Counter to new function entry point
        
        Input: Current instruction (struct), Stack and Register array, 
        Function array

        Output: Void

        Updates: State register if error encountered

    ##########################################################################*/

    if ( !strncmp(curr_instruction->type1, "REG", SIZE_OF_OPCODE) || 
        !strncmp(curr_instruction->type1, "STK", SIZE_OF_OPCODE) || 
        !strncmp(curr_instruction->type1, "PTR", SIZE_OF_OPCODE)) {

        fprintf(stderr,"CALL can only be used with values\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;
        return;

    }

    if ( curr_instruction->val1.value > 7 || curr_instruction->val1.value < 0) {
        fprintf(stderr,"Function does not exist, aborting\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;
        return;

    }

    //Ensure enough stack space to call function
    if (registers[FRAME_PTR]+ 3 + MAX_NUMBER_OF_SYMBOLS >= STACK_SIZE) {
        fprintf(stderr,"CALL will cause overflow, aborting\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;
        return;

    }

    //Push STK pointer to first free location on stack
    stack[registers[FRAME_PTR] + 1 + MAX_NUMBER_OF_SYMBOLS] = 
    registers[STK_PTR];

    //Push Frame pointer to second free location on stack
    stack[registers[FRAME_PTR]+ 2 + MAX_NUMBER_OF_SYMBOLS] = 
    registers[FRAME_PTR];

    //Push return address to third next free location on stack
    stack[registers[FRAME_PTR]+ NUMBER_OF_REGISTERS_PUSHED_FROM_CALL 
    + MAX_NUMBER_OF_SYMBOLS] = registers[PC];

    //update STK & FRAME_PTR to point to new stack frame (return address)
    registers[STK_PTR] = registers[FRAME_PTR]+ 
    NUMBER_OF_REGISTERS_PUSHED_FROM_CALL + MAX_NUMBER_OF_SYMBOLS;

    registers[FRAME_PTR] = registers[FRAME_PTR]+ 
    NUMBER_OF_REGISTERS_PUSHED_FROM_CALL +MAX_NUMBER_OF_SYMBOLS;

    //update PC to the entry point of the specified function label
    registers[PC] = find_entry_point(curr_instruction->val1.value,
        function_array, registers);

    return;

}

void RETURN(struct instruction *curr_instruction,  
    BYTE* stack, BYTE*registers) {

    /*########################################################################

        [RET]

        Terminates the current function, 
        this is guaranteed to always exist at the end of each function. 

        There may be more than one RET in a function. 

        If this function is the entry-point, then the program terminates.

        The state of the registers is preserved between CAL and RET operations

        1) Check if returning from main
        2) Check return won't cause underflow
        3) Restore previous Stack pointer pushed from external function state
        4) Set program counter to return address of previous stack frame
        5) Restore frame pointer to return address of previous stack frame
        
        Input: Current instruction (struct), Register array
        Output: Void

        Updates Machine State register if error encountered or return from main

    ##########################################################################*/

    //If the frame pointer is at the bottom of the stack, we are in main
    if (registers[FRAME_PTR] == INIT_VAL) {
        registers[PROG_STATE] = PROG_STATE_COMPLETE;
        return;
    }

    //if pushing previous state registers will cause underflow, abort
    if ((registers[FRAME_PTR] - NUMBER_OF_REGISTERS_PUSHED_FROM_CALL ) < 0) {
        fprintf(stderr,"Return will cause underflow, aborting\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;
        return;
    }

    //Restore register values from previous stack frame
    //Integers are offsets from the frame PTR
    registers[STK_PTR] = stack[registers[FRAME_PTR] - 2 ];
    registers[PC] = stack[registers[FRAME_PTR]];
    registers[FRAME_PTR] = stack[registers[FRAME_PTR] - 1 ];

    return;
}

void REFERENCE(struct instruction *curr_instruction,  
    BYTE* stack, BYTE*registers) {

    /*#######################################################################

    [REF A B]

    Takes a stack symbol B and stores its corresponding stack address in A

    1) Check valid input types
    2) Get Stack address
    3) Check for valid stack address
    4) Check A type
    5) Store stack symbol B address in A

    Input: Current instruction (struct), Stack and Register array
    Output: Void

    Updates: State register if error encountered

    #########################################################################*/   

    //Ensure correct value types recieved
    if ( !strncmp(curr_instruction->type2, "VAL", SIZE_OF_OPCODE) || 
    !strncmp(curr_instruction->type2, "REG", SIZE_OF_OPCODE)) {

        fprintf(stderr,"REF variable 2 must be STK/PTR\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;
        return;

    }

    if ( !strncmp(curr_instruction->type1, "VAL", SIZE_OF_OPCODE)) {

        fprintf(stderr,"Cant REF to a value!\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;

        return;
    }


    BYTE stack_address = 0;
    //Stack address obtained from current instruction
    //If PTR, reference address from stack
    if (!strncmp(curr_instruction->type2, "PTR", SIZE_OF_OPCODE)){
        stack_address = get_stack_address_of_symbol(
        curr_instruction->val2.pointer_value, registers[FRAME_PTR],registers);
        stack_address = stack[stack_address];
    }else{
    stack_address = get_stack_address_of_symbol(
        curr_instruction->val2.stack_symbol, registers[FRAME_PTR],registers);
    }

    //REF REG X STK/PTR X
    if ( !strncmp(curr_instruction->type1, "REG", SIZE_OF_OPCODE)) {

        registers[curr_instruction->val1.reg_addr] = stack_address;
        return;
    }
    
    //REF STK X STK/PTR X
    else if ( !strncmp(curr_instruction->type1, "STK", SIZE_OF_OPCODE)) {

        BYTE stack_address2 = get_stack_address_of_symbol(
            curr_instruction->val1.stack_symbol, 
        registers[FRAME_PTR], registers);

        stack[stack_address2] = stack_address;
        return;
    }

    //REF PTR X STK/PTR X
    else {
        
    BYTE stack_address2 = get_stack_address_of_symbol(
            curr_instruction->val1.pointer_value, 
        registers[FRAME_PTR], registers);

        stack[stack[stack_address2]]= stack_address;
        return;
    }
    return;
}

void ADD(struct instruction *curr_instruction, BYTE *registers) {

    /*########################################################################

    [ADD A B]

    Takes two register addresses and ADDs their values, 
    storing the result in the first listed register.

    Input: Current instruction (struct), Register array
    Output: Void

    Updates: State register if error encountered

    ##########################################################################*/

    if ( !strncmp(curr_instruction->type1, "VAL", SIZE_OF_OPCODE) || 
    !strncmp(curr_instruction->type1, "STK", SIZE_OF_OPCODE) || 
    !strncmp(curr_instruction->type1, "PTR", SIZE_OF_OPCODE) ||
    !strncmp(curr_instruction->type2, "VAL", SIZE_OF_OPCODE) || 
    !strncmp(curr_instruction->type2, "STK", SIZE_OF_OPCODE) || 
    !strncmp(curr_instruction->type2, "PTR", SIZE_OF_OPCODE)) {

        fprintf(stderr,"ADD can only be used with registers\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;
        return ;

    }

    registers[curr_instruction->val1.reg_addr] = 
    registers[curr_instruction->val1.reg_addr]+
    registers[curr_instruction->val2.reg_addr];

    return;
}

void PRINT(struct instruction *curr_instruction,  
    BYTE* stack, BYTE*registers) {

    /*#######################################################################

    [PRINT A]

    Takes any address type and prints the contents 
    to a new line of standard output as an unsigned integer.

    Input: Current instruction (struct), Stack and Register array
    Output: Void

    Updates: State register if error encountered

    #########################################################################*/

        if ( !strncmp(curr_instruction->type1, "VAL", SIZE_OF_OPCODE)) {
            printf("%u\n",curr_instruction->val1.value);
            return;
        }

        if ( !strncmp(curr_instruction->type1, "REG", SIZE_OF_OPCODE)) {
            printf("%u\n",registers[curr_instruction->val1.reg_addr]);
            return;
        }

        if ( !strncmp(curr_instruction->type1, "STK", SIZE_OF_OPCODE)) {

            BYTE stack_address = get_stack_address_of_symbol(
                curr_instruction->val1.stack_symbol, 
            registers[FRAME_PTR], registers);


            printf("%u\n",stack[stack_address]);
            return;

        }

        if ( !strncmp(curr_instruction->type1, "PTR", SIZE_OF_OPCODE)) {

            BYTE stack_address = get_stack_address_of_symbol(
                curr_instruction->val1.pointer_value, 
            registers[FRAME_PTR], registers);


            printf("%u\n",stack[stack[stack_address]]);
            return;
        }
    return;    
}

void NOT(struct instruction *curr_instruction, BYTE *registers) {

    /*#######################################################################

    [NOT A]

    Takes a register address and performs a bitwise not 
    operation on the value at that address. 
    
    The result is stored in the same register

    1) Check valid input types
    2) Store Bitwise Not value of reg A in reg A

    Input: Current instruction (struct), Stack and Register array
    Output: Void

    Updates: State register if error encountered

    #########################################################################*/  
    
    //Type Check
    if ( !strncmp(curr_instruction->type1, "STK", SIZE_OF_OPCODE) ||
    !strncmp(curr_instruction->type1, "PTR", SIZE_OF_OPCODE) ||
     !strncmp(curr_instruction->type1, "VAL", SIZE_OF_OPCODE) ) {

        fprintf(stderr,"NOT operation may only be performed on REG type\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;
        return;

    }

    //Perform bitwise NOT on value held in register
    if ( !strncmp(curr_instruction->type1, "REG", SIZE_OF_OPCODE) ) {
        registers[curr_instruction->val1.reg_addr] = 
        ~(registers[curr_instruction->val1.reg_addr]);
        return;
    }

    return;

}

void EQU(struct instruction *curr_instruction, BYTE *registers) {

    /*#######################################################################

    [EQU A]

    Takes a register address and tests if it equals zero. 
    The value in the register will be set to 1 if it is 0, or 0 if it is not. 
    The result is stored in the same register 

    1) Check valid input types
    2) Check REG value == 0 and store result in REG

    Input: Current instruction (struct), Stack and Register array
    Output: Void

    Updates: State register if error encountered

    #########################################################################*/
    
    //Type check
    if ( !strncmp(curr_instruction->type1, "STK", SIZE_OF_OPCODE) ||
    !strncmp(curr_instruction->type1, "PTR", SIZE_OF_OPCODE) ||
     !strncmp(curr_instruction->type1, "VAL", SIZE_OF_OPCODE) ) {

        fprintf(stderr,"EQU operation may only be performed on REG type\n");
        registers[PROG_STATE] = PROG_STATE_ERROR;
        return;

    }

    //Perform NOT on value within register
    if ( !strncmp(curr_instruction->type1, "REG", SIZE_OF_OPCODE) ) {
        
        if (registers[curr_instruction->val1.reg_addr] == 0) {
            registers[curr_instruction->val1.reg_addr] = 1;
            return;
        }else{
            registers[curr_instruction->val1.reg_addr] = 0;
        }
        return;
    }

    return;

}

void execute_instruction(struct instruction *curr_instruction, BYTE curr_opcode,
    BYTE* stack, BYTE* registers, struct function** function_array) {

    /*#######################################################################

    Execute_instruction

    Executes current opcode

    1) Check opcode is valid
    2) Calls associated function

    Input: Current instruction (struct), current opcode, 
    Stack and Register array, function array

    Output: Void

    Updates: State register if error encountered

    #########################################################################*/

    //Check function label is valid.
    if (curr_opcode > 7 ||curr_opcode < 0) {
        registers[PROG_STATE] = PROG_STATE_ERROR;
        return;
    }

    switch(curr_opcode) {

        //MOV
        case 0 :
            MOVE(curr_instruction, stack, 
            registers);
            break;

        //CAL
        case 1:
            CALL(curr_instruction, stack, registers,function_array);
            break;

        //RET
        case 2 :
            RETURN(curr_instruction, stack, registers);
            break;

        //REF
        case 3 :
            REFERENCE(curr_instruction, stack, registers);
            break;

        //ADD
        case 4 :
            ADD(curr_instruction, registers);
            break;

        //PRINT
        case 5:
            PRINT(curr_instruction, stack, registers);
            break;

        //NOT
        case 6 :
            NOT(curr_instruction, registers);
            break;

        //EQU
        case 7 :
            EQU(curr_instruction, registers);
            break;

    }
    return;
}



