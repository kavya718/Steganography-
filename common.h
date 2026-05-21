#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>

#ifndef MAX_FILE_SUFFIX
#define MAX_FILE_SUFFIX 8  // Global constant for file extensions
#endif

#define MAGIC_STRING "#*"  // Default magic string used for encoding/decoding


//Function: get_magic_string
//Prompts the user to enter a custom magic string 
//used during encoding.

static inline void get_magic_string(char *magic_string)
{
    printf("Enter the magic string to use for encoding: ");
    fflush(stdout);  // Ensure prompt prints before input
    fgets(magic_string, 50, stdin);
    magic_string[strcspn(magic_string, "\n")] = '\0';  // Remove newline if present
}

#endif




