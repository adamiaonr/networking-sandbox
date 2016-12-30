#include "common.h"

int Common::get_str_arg(ArgvParser * parser, char * str, const char * arg) {

    if (parser->foundOption(arg))
        strncpy(str, (char *) parser->optionValue(arg).c_str(), MAX_STRING_SIZE);
    else
        return 0;

    return 1;
}

int Common::get_int_arg(ArgvParser * parser, int & value, const char * arg) {

    if (parser->foundOption(arg))
        value = std::stoi(parser->optionValue(arg));
    else
        return 0;

    return 1;
}