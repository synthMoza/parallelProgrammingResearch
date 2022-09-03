#include "stdio.h"
#include "limits.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"
#include "input.h"

// Error list:
// -1 : missing argument 'n'
// -2 : too many arguments are provided
// -3 : the input string is not a number
// -4 : the input number is less than zero
// -5 : the input number is our of range (the range is [LONG_MIN; LONG_MAX])

long input(char* strptr)
{
    char *endptr = NULL;

    // Check the string
    long input = 0;
    input = strtol(strptr, &endptr, 10);

    if (endptr == strptr || *endptr != '\0')
    {
        printf("Wront input string\n");
        return -3;
    }

    if (errno == ERANGE && (input == LONG_MAX || input == LONG_MIN))
    {
        printf("Out of range\n");
        return -5;
    }

    return input;
}