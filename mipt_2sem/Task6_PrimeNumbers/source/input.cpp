#include "stdio.h"
#include "limits.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"
#include <stdexcept>

#include "input.h"

namespace util
{

long strtol(const char* inputString, int radix)
{
    char *endptr = nullptr;
    long input = ::strtol(inputString, &endptr, radix);

    if (endptr == inputString || *endptr != '\0')
        throw std::runtime_error("Wrong input string");
    
    if (errno == ERANGE && (input == LONG_MAX || input == LONG_MIN))
        throw std::out_of_range("Input number is out of range");

    return input;
}

unsigned long long strtoull(const char* inputString, int radix)
{
    char *endptr = nullptr;
    unsigned long long input = ::strtoull(inputString, &endptr, radix);

    if (endptr == inputString || *endptr != '\0')
        throw std::runtime_error("Wrong input string");
    
    if (errno == ERANGE && (input == ULLONG_MAX || input == 0))
        throw std::out_of_range("Input number is out of range");

    return input;
}

}
