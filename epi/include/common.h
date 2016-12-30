#ifndef COMMON_HH
#define COMMON_HH

// C headers
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// C++ headers
#include <iostream>
#include <ctime>
// custom C headers
// custom C++ headers
#include "argvparser.h"

using namespace CommandLineProcessing;

#define MAX_STRING_SIZE         256

class Common {

    public:

        Common() {}
        ~Common() {}

        static int get_str_arg(ArgvParser * parser, char * str, const char * arg);
        static int get_int_arg(ArgvParser * parser, int & value, const char * arg);
};

#endif