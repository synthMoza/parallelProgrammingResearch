#include <limits.h>
#include <stdlib.h>
#include <errno.h>

#include <input.h>

int input(const char* str, long* res) 
{
    const char *strptr = NULL;
    char* endptr = NULL;
    long input = 0;
    
    strptr = str;
    input = strtol(strptr, &endptr, 10);

    if (endptr == strptr || *endptr != '\0')
        return -EINVAL;

    if (input <= 0)
        return -EINVAL;

    if (errno == ERANGE && (input == LONG_MAX || input == LONG_MIN))
        return -ERANGE;

    *res = input;
    return 0;
}