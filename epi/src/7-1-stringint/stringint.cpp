#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <ctime>
#include <thread>
#include <bitset>
#include <string>
#include <vector>
#include <deque>

#include "common.h"
#include "argvparser.h"

using namespace CommandLineProcessing;

#define OPTION_OPERATION    (char *) "op"
#define OPTION_VALUE        (char *) "value"

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ninterconvert strings and integers \
(EPI problem 7.1, pages 94)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_OPERATION,
            "conversion operation : 'str_to_int' for string to int, 'int_to_str' for int to string",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_VALUE,
            "integer value to convert",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);


    return parser;
}

int to_abs(int value) {

    // here's the trick to abs() an integer. if the int is negative, you 
    // 'toggle' its bits and add 1 (2's complement arithmetic).

    // drag the sign bit to the right. if it is set, then this will set 
    // all bits to 1, if not set, this will keep it as 0.
    uint32_t drag_sign_bit = value >> ((sizeof(value) * 8) - 1);
    // then XOR value with sign_bit
    value ^= drag_sign_bit;
    value += (drag_sign_bit & 1);

    return value;  
}

std::string int_to_str(int value_int) {

    std::string value_str = "";
    bool is_negative = ((value_int < 0) ? true : false);
    value_int = to_abs(value_int);

    while (value_int > 0) {

        // extract the least sign. digit from value_int
        int digit = (value_int % 10);
        // add the ascii value of the digit (i.e. + the int value of the char '0')
        value_str.push_back((char) (digit + ((int) '0')));
        // remove the digit just added to value_str
        value_int = value_int / 10;
    }

    if (is_negative) value_str.push_back('-');

    // for (unsigned i = 0; i < (value_str.size() / 2); i++)
    //     std::swap(value_str[0], value_str[value_str.size() - 1 - i]);

    return {value_str.rbegin(), value_str.rend()};
}

int str_to_int(std::string value_str) {

    int value_int = 0, start_i = 0;

    if (value_str[0] == '-') start_i = 1;

    // for each digit added to value_int, shift one digit of 
    // value_int to the left (i.e. value_int x 10).
    // note how strings are organized : e.g. for string 'darth',
    // 0 1 2 3 4 5
    // d a r t h \0
    for (unsigned i = start_i; i < value_str.size(); i++)
        value_int = (value_int * 10) + (int) (value_str[i]) - ((int) '0');

    return (start_i ? -value_int : value_int);
}

int main (int argc, char **argv) {

    std::string op_str, value_str;
    int value_int;

    ArgvParser * arg_parser = create_argv_parser();

    // parse() takes the arguments to main() and parses them according to 
    // ArgvParser rules
    int parse_result = arg_parser->parse(argc, argv);

    if (parse_result == ArgvParser::ParserHelpRequested) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        delete arg_parser;
        exit(-1);

    } else if (parse_result != ArgvParser::NoParserError) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        std::cerr << "stringint::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        op_str = arg_parser->optionValue(OPTION_OPERATION);

        if (op_str == "str-to-int") {

            value_str = arg_parser->optionValue(OPTION_VALUE);

            std::cout << "stringint::main() : [INFO] conversion :"
                << "\n\tOPERATION: " << op_str
                << "\n\tVALUE: " << str_to_int(value_str) << std::endl;

        } else {

            value_int = std::stoi(arg_parser->optionValue(OPTION_VALUE));

            std::cout << "stringint::main() : [INFO] conversion :"
                << "\n\tOPERATION: " << op_str
                << "\n\tVALUE: " << int_to_str(value_int) << std::endl;
        }
    }

    delete arg_parser;

    exit(0);
}