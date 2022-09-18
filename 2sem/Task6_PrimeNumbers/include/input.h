#ifndef UTIL_INPUT_H
#define UTIL_INPUT_H

namespace util
{

// Get input number using strtol
// @return inputString String that represents number
// @return radix Radix of this number
// @return Number
// @note Throws in case of errors
long strtol(const char* inputString, int radix = 10);

// Get input number using strtoull
// @return inputString String that represents number
// @return radix Radix of this number
// @return Number
// @note Throws in case of errors
unsigned long long strtoull(const char* inputString, int radix = 10);

}

#endif // define UTIL_INPUT_H
