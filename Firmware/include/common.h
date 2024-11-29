#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "const.h"
#include <Arduino.h>

enum class RetRes
{
    OK = 0,
    ERROR,
    ERROR_BAD_PARAMS,
    ERROR_BAD_RESULT,
    ERROR_NULL_POINTER
};


#endif