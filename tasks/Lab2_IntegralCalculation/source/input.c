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

long input(int argc, char *argv[])
{
    char *strptr = NULL, *endptr = NULL;
    // Check argument's amount
    if (argc < 2)
    {
        printf("Too less arguments\n");
        return -1;
    }
    else if (argc > 2)
    {
        printf("Too manya arguments\n");
        return -2;
    }

    // Check the string
    long input = 0;
    strptr = argv[1];
    input = strtol(strptr, &endptr, 10);

    if (endptr == strptr || *endptr != '\0')
    {
        printf("Wront input string\n");
        return -3;
    }

    if (input <= 0)
    {
        printf("The number must be greater then 0\n");
        return -4;
    }

    if (errno == ERANGE && (input == LONG_MAX || input == LONG_MIN))
    {
        printf("Out of range\n");
        return -5;
    }

    return input;
}