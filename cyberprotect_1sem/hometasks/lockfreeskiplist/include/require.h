#ifndef REQUIRE_HEADER
#define REQUIRE_HEADER

#include <limits.h>

#if __WORDSIZE != 64
    #error "64 bit platform is required"
#endif

static_assert(sizeof(void*) == 8, "64 bit pointers are required");

#endif // #define REQUIRE_HEADER
 