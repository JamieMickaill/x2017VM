/*######################################################################

    USYD 2021 COMP2017

    ASSIGNMENT 2: VM & OBJDUMP

    OBJDUMP_X2017_MAIN.c

    STUDENT: 500611960

    JAMIE MICKAILL

########################################################################*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "objdump_x2017.h"

/*#############################################################################

    MAIN FUNCTION

#############################################################################*/


int main (int argc, char **argv) {


    /*########################################################################

    1) Construct function array from binary file input at argv[1]
    2) Print out each Function Label, followed by its associated instructions
    3) Free all allocated memory
    4) Exit with return value

        If error encountered, return value set to 1, program safely exits.

    #########################################################################*/

    unsigned char return_value = INIT_VAL;
    
    if(argc < 2){
        fprintf(stderr, "Provide .x2017 binary file for reading");
        exit(1);
        }
    
    struct function** functions_array = 
    get_function_array(argv[1], &return_value);

    if(!return_value){
    print_instructions(functions_array);
    }

    free_the_malloc(functions_array);

    exit(return_value);
    return 0;
}

