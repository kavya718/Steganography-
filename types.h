#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint;   // Common uint alias for all files

typedef enum {
    e_success,
    e_failure
} Status;

typedef enum {
    e_encode,
    e_decode,
    e_unsupported
} OperationType;

#endif


