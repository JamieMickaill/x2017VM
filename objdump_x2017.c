/*######################################################################

    USYD 2021 COMP2017

    ASSIGNMENT 2: VM & OBJDUMP

    OBJDUMP_X2017.C

    STUDENT: 500611960

    JAMIE MICKAILL

########################################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "objdump_x2017.h"

/*#############################################################################

    Function definitions

#############################################################################*/

struct instruction* init_instruction () {
    
    /*#########################################################################

        Init_instructions_pointer_array

        Allocates memory for a single instruction
        Initializes default struct values to avoid junk read in VM when JUMPing

        Output: Pointer to instruction

    ##########################################################################*/

    struct instruction* instruction = malloc( sizeof(struct instruction) );
    instruction->opcode = INIT_VAL;
    strcpy(instruction->type1,"VAL");
    instruction->val1.value = INIT_VAL;
    strcpy(instruction->type2,"VAL");
    instruction->val2.value = INIT_VAL;

    return instruction;
}

struct instruction** init_instructions_pointer_array (  
    unsigned int number_of_instructions) {

/*###########################################################################

        Init_instructions_pointer_array

        Allocates memory for an array of instructions for use within a function

        Input: Number of instructions in array (Integer)

        Output: Pointer to malloc'd instruction pointer array

    #########################################################################*/

    struct instruction** instructions = 
        malloc( sizeof(char*) * number_of_instructions);

    for(int i=0; i<number_of_instructions; i++) {
        instructions[i] = init_instruction();

    }
    return instructions;

}

struct function* init_function (unsigned int number_of_instructions_in, 
    struct instruction** instructions_in) {

    /*#########################################################################

        init_Function

        Allocates memory for a function structure, initializes function fields

        Input: Number of instructions in function, instruction array

        Output: Pointer to malloc'd function

    #########################################################################*/

    struct function* functionA = malloc(sizeof(struct function));
   
    functionA -> instructions = instructions_in;
    functionA -> number_of_instructions = number_of_instructions_in;

    return functionA;

}

struct function** init_functions_array () {

    /*#########################################################################

        Init_functions_array

        Allocates memory for an array of functions
        Initializes default struct fields

        Output: Pointer to malloc'd function array

    #########################################################################*/

    struct function** functions_array = 
    malloc(MAX_NUMBER_OF_FUNCTIONS * sizeof(char*));

    for (int i=0; i< MAX_NUMBER_OF_FUNCTIONS ; i++) {
        functions_array[i] = malloc(sizeof(struct function));
        functions_array[i]->number_of_instructions = INIT_VAL;
        functions_array[i]->header = INIT_VAL;
    }
    return functions_array;
}

char get_symbol (int i, unsigned char * return_value) {

    /*#########################################################################

        Get_symbol

        Returns the char associated with a certain stack/pointer symbol int value
        Uses ASCII value difference to calculate value

        Output: Stack symbol Char

        Updates: return_value if error encountered

    ###########################################################################*/

    char symbol = INIT_VAL;

    if (i+65 >= 'A' && i+65 <= 'Z') {
        symbol = i+65;
    }
    else if (i+71 >= 'a' && i+71 <= 'z') {
        symbol = (i+71);
    }
    else {
        *return_value = PROG_STATE_ERROR;
    }

    return symbol;

}

void assign_stack_symbol (char * c, char* used_symbol_values,
    int* num_symbols_used) {

    /*#########################################################################

        Assign_stack_symbol

        Assigns stack symbols based on order of appearance within the function.
        Checks whether a stack symbol has been used within the current function
        If the symbol has been used, it returns the same symbol.
        If it has not been used, it assigns the next available symbol.

        Input: 
            * Symbol from instruction (char), 
            * Array of used symbol valeus from current scope,
            * Number of symbols allocated so far in the scope (int)

        Output: Void

        Updates: Used symbols array, 

    #########################################################################*/

    unsigned int symbol_value = INIT_VAL;

    if(*num_symbols_used > INIT_VAL ) {

        for(int i=0; i<MAX_NUMBER_OF_INSTRUCTIONS; i++) {
            //Used symbol case
            if(used_symbol_values[i] == *c) {
                symbol_value = i;
                *c = get_symbol(symbol_value, INIT_VAL);
                return;
                }
        }

    }else {
        //First symbol case
        used_symbol_values[*num_symbols_used] = *c;
        *num_symbols_used +=1;
        symbol_value = 0;
        *c = get_symbol(symbol_value, 0);
        return;
    }
    //New symbol case
    if(symbol_value == 0) {
        used_symbol_values[*num_symbols_used] = *c;
        symbol_value = *num_symbols_used;
        *num_symbols_used +=1;
        *c = get_symbol(symbol_value, 0);
        return;
    }

}

char* get_opcode_string(BYTE curr_opcode) {

    /*#########################################################################

        Get_opcode_string

        Returns the string associated with a certain opcode

        Output: opcode string

    #########################################################################*/

    char * opcode_string_return = "";

    switch(curr_opcode) {

            case 0:
            opcode_string_return = "MOV";                    
            break;

            case 1:
            opcode_string_return = "CAL";
            break;

            case 2:
            opcode_string_return = "RET";
            break;

            case 3:
            opcode_string_return = "REF";
            break;

            case 4:
            opcode_string_return = "ADD";                    
            break;

            case 5:
            opcode_string_return = "PRINT";                    
            break;

            case 6:
            opcode_string_return = "NOT";
            break;

            case 7:
            opcode_string_return = "EQU";
            break;
        }

    return opcode_string_return;
}

void update_value(BYTE* curr_location_in_byte, BYTE *buffer_in, 
    TWO_BYTES *buffer2_in, FILE* binary_file, BYTE* value_to_be_set, 
    BYTE mask, BYTE size_in_bits, BYTE *offset, unsigned char * return_value) {


    /*#########################################################################

        Update_value

        Reads next byte from binary file into buffer.

        Updates specified value to be set (i.e. Instruction struct fields)

        Updates buffer state variables.

        1) check if current Byte contains value
        2) If yes -> use mask to obtain value
        3) If no -> Read in 16bits, flip endianness, 
            shift by current place in previous byte and read with mask
        4) Update buffer to latest location in file, 
            incrementing offset in current byte
        5) Shift buffer by bit offset in current byte 
            (modulo 8 if 2byte buffer used.)
            Buffer is now ready for next read.

        Inputs: Current byte location, Fread Buffer , Fread Buffer 2
            Binary File Pointer, Given value, Bit mask, Size of read, 
            Current Fseek offset, Return value (If failed)

        Output: Void

        Updates: Buffer , Given value,  Current Fseek offset, 
        Current byte location, Binary File Pointer, Return value (If failed)

    #########################################################################*/

    //check whether the current read is contained within the current byte
    if(*curr_location_in_byte + size_in_bits <= SINGLE_BYTE_READ_BITS ) {
        *value_to_be_set = (*buffer_in & mask);
        *curr_location_in_byte += size_in_bits;
        *buffer_in = *buffer_in >> size_in_bits;
    }

    //If we need to read the next byte, read in 2 bytes from incremented offset
    else {

        *buffer2_in = INIT_VAL;
        uint8_t seek = fseek(binary_file, -(*offset + 1 ), SEEK_END);

        if (seek != SUCCESSFUL_SEEK ) {
            fprintf(stderr,"fseek error\n");
            *return_value = PROG_STATE_ERROR;
            return;
        }

        uint8_t read = fread(buffer2_in, SINGLE_BYTE_READ, 
            DOUBLE_BYTE_READ, binary_file);

        if ( read == FAILED_READ ) {
            fprintf(stderr,"fread error\n");
            *return_value = PROG_STATE_ERROR;
            return;
        }

        //flip endianness of read for bitshifting
        uint16_t buffer2_big_endian = ntohs(*buffer2_in);

        //shift and mask bits
        buffer2_big_endian = buffer2_big_endian >> *curr_location_in_byte;
        *value_to_be_set = (buffer2_big_endian & mask);

        //increment current byte location
        *curr_location_in_byte += size_in_bits;

        if( *curr_location_in_byte == DOUBLE_BYTE_READ_BITS ) {
            *curr_location_in_byte = SINGLE_BYTE_READ_BITS;
        }
        else {
            *curr_location_in_byte = *curr_location_in_byte % SINGLE_BYTE_READ_BITS;
        }

        //increment fseek offset
        *offset+=SINGLE_BYTE_READ;

        seek = fseek(binary_file, -(*offset), SEEK_END);

        *buffer_in = INIT_VAL;

        //update buffer for next read
        BYTE read2 = fread(buffer_in, SINGLE_BYTE_READ, SINGLE_BYTE_READ, binary_file);
        if(read2 == INIT_VAL){
            *return_value = PROG_STATE_ERROR;
            return;
        }

        *buffer_in = *buffer_in >> *curr_location_in_byte;
    }


}

void get_next_instruction(BYTE *number_of_instructions, BYTE* curr_location_in_byte, BYTE *buffer, 
    TWO_BYTES *buffer2, FILE* binary_file, BYTE *offset, unsigned char * return_value, 
    unsigned int* i, struct function* functionA){

    /*#####################################################################

        Get_next_instruction

        Reads next instruction from binary file and copies to function array

        Output: Void

        Updates: Return value if error encountered

    #########################################################################*/

            //Set flags for # of opcode args to read
            BYTE single_arg = INIT_VAL;
            BYTE double_arg = INIT_VAL;

            //Initialize instruction field values
            BYTE curr_opcode = INIT_VAL;
            BYTE type1 = INIT_VAL;
            BYTE type2 = INIT_VAL;
            BYTE value1 = INIT_VAL;
            BYTE value2 = INIT_VAL;

            //init Instruction
            struct instruction* instructionA = init_instruction();

            //read in current instruction opcode
            update_value(curr_location_in_byte, buffer, buffer2, 
            binary_file, &curr_opcode, FIRST_3_BIT_MASK, 
            SIZE_OF_OPCODE, offset,return_value);

            //depending on opcode, we read 0,1 or 2 values
            switch(curr_opcode) {
            
                //if OPCODe = RET, 0 values
                case 2:
                    break;

            //All non-RET instructions require at least one first value and type
                //MOV //REF //ADD //CAL //PRINT //NOT //EQU
                case 0:
                case 3:
                case 4:
                case 1:
                case 5:
                case 6:
                case 7:

                //set flag to state that a single arg has been recieved
                single_arg = 1;

                //read first VALUE TYPE 
                update_value(curr_location_in_byte, buffer, buffer2, 
                binary_file, &type1, FIRST_2_BIT_MASK, SIZE_OF_TYPE, 
                offset,return_value);

                //read in next value based on VALUE TYPE 
                switch(type1) {
                    
                    //type 00 = 8 bit value
                    case 0 :
                        update_value(curr_location_in_byte, buffer, buffer2, 
                        binary_file, &value1, TOTAL_8_BIT_MASK, SIZE_OF_VALUE, 
                        offset,return_value);
                        break;
                            
                    //type 01 = regaddr 3 bits
                    case 1:
                        update_value(curr_location_in_byte, buffer, buffer2, 
                        binary_file, &value1, FIRST_3_BIT_MASK, SIZE_OF_REG, 
                        offset,return_value);
                        break;
                    
                    //type 10 or 11 = pointer or stack symbol
                    case 2:
                    case 3:
                        update_value(curr_location_in_byte, buffer, buffer2, 
                        binary_file, &value1, FIRST_5_BIT_MASK, SIZE_OF_SYMBOL, 
                        offset,return_value);
                        break;
                        }
                break;
            }

            //We now collect the second value from the non-unary opcodes 
            //0,3,4 (MOV,REF,ADD)
            switch(curr_opcode) {

                case 0:
                case 3:
                case 4: 

                //Set flag to state that curr isntruction has 2 values 
                double_arg = 1;

                //Read Type 2
                update_value(curr_location_in_byte, buffer, buffer2, 
                binary_file, &type2, FIRST_2_BIT_MASK, SIZE_OF_TYPE,
                 offset,return_value);

                //Read in next value based on TYPE 2
                switch(type2) {
                    
                    //Type 00 = 8 bit value
                    case 0:
                        update_value(curr_location_in_byte, buffer, buffer2, 
                        binary_file, &value2, TOTAL_8_BIT_MASK, SIZE_OF_VALUE, 
                        offset,return_value);
                        break;
        
                    //Type 01 = regaddr 3 bits
                    case 1:
                        update_value(curr_location_in_byte, buffer, buffer2, 
                        binary_file, &value2,  FIRST_3_BIT_MASK, SIZE_OF_REG, 
                        offset,return_value);
                        break;
                    
                    //Type 10 or 11 = pointer or stack symbol
                    case 2:
                    case 3:
                        update_value(curr_location_in_byte, buffer, buffer2, 
                        binary_file, &value2, FIRST_5_BIT_MASK, SIZE_OF_SYMBOL, 
                        offset,return_value);
                        break;
                    }
                break;
            }

            //Set opcode of instruction to that read from file
            instructionA->opcode = curr_opcode;

            //We have now read all instructions fields, update instruction struct
            if(single_arg) {

                switch(type1) {

                    //8bit value, write type string to instruction
                    case 0:
                        strcpy(instructionA->type1,"VAL");
                        instructionA->val1.value = value1;
                        break;

                    //3bit reg addr, write type string to instruction
                    case 1:
                        strcpy(instructionA->type1,"REG");
                        instructionA->val1.reg_addr = value1;
                        break;

                    //5bit stack symbol
                    case 2:
                        strcpy(instructionA->type1,"STK");
                        instructionA->val1.stack_symbol = 
                        get_symbol(value1, return_value);
                        break;
                    
                    //5 bit pointer value
                    case 3:
                        strcpy(instructionA->type1,"PTR");
                        instructionA->val1.pointer_value = 
                        get_symbol(value1, return_value); 
                        break;
                }
            }

            //Copy second arg value and type to instruction
            if(double_arg) {
                
                switch(type2) {

                    //8bit value, convert to string
                    case 0:
                        strcpy(instructionA->type2,"VAL");
                        instructionA->val2.value = value2;
                        break;

                    //3bit reg addr, convert to string
                    case 1:
                        strcpy(instructionA->type2,"REG");
                        instructionA->val2.reg_addr = value2;
                        break;             

                    //5bit stack symbol
                    case 2:
                        strcpy(instructionA->type2,"STK");
                        instructionA->val2.stack_symbol = 
                        get_symbol(value2, return_value);
                        break;
                    
                    //5 bit pointer value
                    case 3:
                        strcpy(instructionA->type2,"PTR");
                        instructionA->val2.pointer_value = 
                        get_symbol(value2, return_value);                       
                        break;
                }
            }

            //put instruction in instruction array
            memcpy(functionA->instructions[(*number_of_instructions -1)- *i], 
            instructionA, sizeof(struct instruction));
            *i+=1;

            //free instruction
            free(instructionA);
            
}

struct function** get_function_array(char* filename, 
    unsigned char * return_value) {

    /*#####################################################################

        Get_functions_array

        Reads Binary file 
        (From end to beginning -> this is to account for padding), 
        and dumps interpreted instructions into function struct array.

        1) Read in file, checking if exists and valid
        2) Create a function array to be populated
        3) Fseek to end of file
        4) Initialize buffers, bit counter, fseek offset, funtion_end flag
        5) Read first byte from file into buffer
        6) Set index for reading in instructions to end of function array 
            (as we are reading the file in reverse)
        7) Begin binary read loop, terminating upon finding final function
        8) Read all instructions for each function, followed by func label
        9) Repeat for all remaining functions
        10) See below code for more detailed explanation

        Output: Pointer to malloc'd function array

    #########################################################################*/


    FILE * binary_file;
    binary_file = fopen(filename, "rb");

    if(!binary_file) {
        fprintf(stderr,"FILE NOT FOUND!\n");
        exit(1);
    }
    
    struct function** functions_array = init_functions_array();

    if(fseek(binary_file, -1, SEEK_END) != SUCCESSFUL_SEEK) {
        fprintf(stderr,"fseek failed\n");
        *return_value = PROG_STATE_ERROR;
        return NULL;
    }

    //Initialize buffer, offset, byte location and function flag
    BYTE buffer = INIT_VAL;
    TWO_BYTES buffer2 = INIT_VAL;
    BYTE offset = 1;
    BYTE curr_location_in_byte = INIT_VAL;
    BYTE last_function_found = INIT_VAL;

    if(fread(&buffer, SINGLE_BYTE_READ , SINGLE_BYTE_READ, binary_file) == FAILED_READ) {
        fprintf(stderr,"fread failed\n");
        *return_value = PROG_STATE_ERROR;
        return NULL;
    }

    //Reading in reverse
    int function_number = 7;

    //For each function, perform the below
    while(!last_function_found) {

        //Initialize label and number of instructions
        BYTE function_label = INIT_VAL;
        BYTE number_of_instructions = INIT_VAL;

        //Read in number of instructions
        update_value(&curr_location_in_byte, &buffer, &buffer2, 
        binary_file, &number_of_instructions, FIRST_5_BIT_MASK, 
        SIZE_OF_INSTRUCTION, &offset,return_value);

        //Create instructions array for number of instructions
        struct instruction** instructions_arrayA = 
        init_instructions_pointer_array(number_of_instructions);

        //Create function - to be linked to functions array after label is retrieved
        struct function* functionA = 
        init_function(number_of_instructions, instructions_arrayA);

        unsigned int i = 0;
        while (i < number_of_instructions && !*return_value) {

            get_next_instruction(&number_of_instructions, &curr_location_in_byte, &buffer, 
            &buffer2, binary_file, &offset, return_value, &i, functionA);

        }
        
        //We have now read total all instructions
        //Read in function label

        update_value(&curr_location_in_byte, &buffer, &buffer2, 
        binary_file, &function_label,  FIRST_3_BIT_MASK, 
        SIZE_OF_REG, &offset,return_value);

        if(function_label > 7) {
            fprintf(stderr,"Invalid function Label\n");
            *return_value = PROG_STATE_ERROR;
            return NULL;
        }

        //Check if reading last function from EOF (Beginning of file)
        if (ftell(binary_file) == 1 || ftell(binary_file) == 0 ) {
            last_function_found = 1;
        }
        
        //Update function header
        functionA->header = function_label;

        //Copy function to functions array
        memcpy(functions_array[function_number],
        functionA, sizeof(struct function));

        // Function number begins at 7 and decrements to 0 
        //-> This is because we read from the end of the file
        // The first function read is the last in the binary
        function_number-=1;

        //Free copied instruction
        free(functionA);
    }

    //Close binary file
    fclose(binary_file);

    //Return populated array
    return functions_array;
}

void print_instructions(struct function** functions_array) {

    /*#########################################################################

        Print_instructions

        Iterates over function array and prints each function label followed by 
        its associated instructions to stdout.

        Output: x2017 Homebrew to StdOut

    #########################################################################*/

    for(int i = 0; i<8; i++) {

        int num_symbols_used = INIT_VAL;
        char used_symbol_values[32] = {INIT_VAL};

        if(functions_array[i]->number_of_instructions > INIT_VAL) {

            printf("FUNC LABEL %i\n", functions_array[i]->header);

            for(int j=0; j<functions_array[i]->number_of_instructions; j++) {

                char *opcode_string = 
                get_opcode_string(functions_array[i]->instructions[j]->opcode);

                //Two args
                if ( functions_array[i]->instructions[j]->opcode == 0 || 
                    functions_array[i]->instructions[j]->opcode == 3  || 
                    functions_array[i]->instructions[j]->opcode == 4  ) {
                    
                    char typeval1[12];

                    //if type1 is an REG
                    if( !strncmp(functions_array[i]->instructions[j]->type1, 
                        "REG", 3)) {

                        strncpy(typeval1, "REG ", 5);
                        char regc[2] = "s";
                        char* reg = regc;
                        sprintf(reg, "%u", 
                        functions_array[i]->instructions[j]->val1.reg_addr);
                        strncat(typeval1, reg, 1);

                    }

                    //if type1 is a VAL
                    else if( !strncmp(
                    functions_array[i]->instructions[j]->type1,"VAL", 3)) {

                        strncpy(typeval1, "VAL ", 5);
                        char val1[6] = "0";
                        char *val = val1;

                        sprintf(val, "%u", 
                        functions_array[i]->instructions[j]->val1.value);

                        strncat(typeval1, val, 6);

                    }

                    //if type1 is a PTR
                    else if( !strncmp(
                        functions_array[i]->instructions[j]->type1,"PTR", 3)) {

                        strncpy(typeval1, "PTR ", 5);

                        char c = 
                        functions_array[i]->instructions[j]->val1.pointer_value;

                        assign_stack_symbol(&c, used_symbol_values, 
                            &num_symbols_used);
                        //check char used (c)
                            //if char in usedchars, return index as ascii
                            //if not in usedchars, 
                            //add to used chars[num_used_chars] 
                            //and increment num_used_chars
                        strncat(typeval1, &c, 1);

                    }

                    //if type1 is a STK
                    else {
                        strncpy(typeval1, "STK ", 5);
                        char c = 
                        functions_array[i]->instructions[j]->val1.stack_symbol;
                        assign_stack_symbol(&c, used_symbol_values, 
                        &num_symbols_used);
                        strncat(typeval1, &c, 1);
                    }

                    char typeval2[12];

                    //if type2 is a REG
                    if( !strncmp(
                        functions_array[i]->instructions[j]->type2, "REG", 3)) {

                        strncpy(typeval2, "REG ", 5);
                        char regc[2] = "s";
                        char* reg = regc;

                        sprintf(reg, "%u", 
                        functions_array[i]->instructions[j]->val2.reg_addr);

                        strncat(typeval2, reg, 1);
                    }

                    //if type2 is a VAL
                    else if( !strncmp(
                        functions_array[i]->instructions[j]->type2, "VAL", 3)) {

                        strncpy(typeval2, "VAL ", 5);
                        char val1[6] = "0";
                        char *val = val1;
                        sprintf(val, "%u", 
                        functions_array[i]->instructions[j]->val2.value);
                        strncat(typeval2, val, 5);     

                    }

                    //if type2 is a PTR
                    else if( !strncmp(
                        functions_array[i]->instructions[j]->type2, "PTR", 3)) {

                        strncpy(typeval2, "PTR ", 5);

                        char c = 
                        functions_array[i]->instructions[j]->val2.pointer_value;

                        assign_stack_symbol(&c, used_symbol_values, 
                        &num_symbols_used);

                        strncat(typeval2, &c, 1);
                    }

                    //if type2 is a STK
                    else {
                        strncpy(typeval2, "STK ", 5);
                        char c = 
                        functions_array[i]->instructions[j]->val2.stack_symbol;
                        assign_stack_symbol(&c, used_symbol_values, 
                        &num_symbols_used);
                        strncat(typeval2, &c, 1);
                    }

                    printf("    %s %s %s\n", opcode_string, typeval1, typeval2);
                }

                //1 arg
                else if( functions_array[i]->instructions[j]->opcode == 1 || 
                    functions_array[i]->instructions[j]->opcode == 5|| 
                    functions_array[i]->instructions[j]->opcode == 6|| 
                    functions_array[i]->instructions[j]->opcode == 7 ) {

                    char typeval1b[12];

                    //if type1 is a REG
                    if( !strncmp(
                        functions_array[i]->instructions[j]->type1, "REG", 3)) {

                        strncpy(typeval1b, "REG ", 5);
                        char regc[2] = "s";
                        char* reg = regc;
                        sprintf(reg, "%u", 
                        functions_array[i]->instructions[j]->val1.reg_addr);

                        strncat(typeval1b, reg, 1);
                    }
                    //if type1 is a VAL
                    else if( !strncmp(
                        functions_array[i]->instructions[j]->type1, "VAL", 3)) {

                        strncpy(typeval1b, "VAL ", 5);
                        char val1[6] = "0";
                        char*val = val1;
                        sprintf(val, "%u", 
                        functions_array[i]->instructions[j]->val1.value);

                        strncat(typeval1b, val, 4);

                    }
                    //if type1 is a PTR
                    else if( !strncmp(
                        functions_array[i]->instructions[j]->type1, "PTR", 3)) {

                        strncpy(typeval1b, "PTR ", 5);
                        char c = 
                        functions_array[i]->instructions[j]->val1.pointer_value;

                        assign_stack_symbol(&c, used_symbol_values, 
                        &num_symbols_used);

                        strncat(typeval1b, &c, 1);
                    }

                    //if type1 is a STK
                    else {
                        strncpy(typeval1b, "STK ", 5);
                        char c = 
                        functions_array[i]->instructions[j]->val1.stack_symbol;
                        assign_stack_symbol(&c, used_symbol_values, 
                        &num_symbols_used);
                        strncat(typeval1b, &c, 1);
                    }
                printf("    %s %s\n", opcode_string, typeval1b);
                }
                //RET
                else {
                printf("    %s\n", opcode_string);    
                    }
            }
        }
    }
}

void free_the_malloc(struct function** function_array) {

    /*#######################################################################

    Free_the_Malloc

    Frees all memory allocated within the program

    Input: function array

    Output: Void

    #########################################################################*/

    for(int i=0; i<MAX_NUMBER_OF_FUNCTIONS; i++) {
        
        for(int j = 0; j<function_array[i]->number_of_instructions; j++) {
            free(function_array[i]->instructions[j]);
        }
        if(function_array[i]->number_of_instructions > INIT_VAL) {
            free(function_array[i]->instructions);
        }
        free(function_array[i]);

    }
        free(function_array);
}
