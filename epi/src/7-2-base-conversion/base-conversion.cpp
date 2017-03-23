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

#define OPTION_VALUE        (char *) "value"
#define OPTION_BASE_FROM    (char *) "base-from"
#define OPTION_BASE_TO      (char *) "base-to"

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nbase conversion \
(EPI problem 7.2, pages 95)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_VALUE,
            "value to be converted",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_BASE_FROM,
            "original base of value",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_BASE_TO,
            "base value should be converted to",
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

int get_digit(char c) {

    int c_int = (int) c;

    // check if a letter (A to F) or a numerical digit
    if (c_int > (int) '9') {

        // if a letter, check if lowercase or uppercase (just in 'case'... pun 
        // intended)
        if (c_int > (int) 'Z')
            return c_int - (int) 'a';
        else
            return c_int - (int) 'A';

    } else {

        return c_int - (int) '0';
    }
}   

int to_base_10(std::string value_str, int base) {

    int value_base_10 = 0;
    for (unsigned i = 0; i < value_str.size(); i++)
        value_base_10 = (value_base_10 * base) + get_digit(value_str[i]);

    return value_base_10;
}
 
std::string base_convert(std::string value_str, int base_from, int base_to) {

    std::string value_base_to_str = "";
    bool is_negative = ((value_str[0] == '-') ? true : false);
    // convert from base_from to base 10
    int value_base_10 = to_base_10((is_negative) ? value_str.substr(1) : value_str, base_from);

    // convert from base 10 to base_to
    while (value_base_10 > 0) {

        // extract the least significant digit of the base_to representation
        int c_int = value_base_10 % base_to;
        // remove the least significant digit of the base_to representation
        value_base_10 /= base_to;
        // convert the digit into a char and append it to the return string
        char c = (c_int > 9 ? (char) (c_int - 10 + (int) 'a') : (char) (c_int + (int) '0'));
        value_base_to_str.push_back(c);
    }

    if (is_negative) value_base_to_str.push_back('-');

    return {value_base_to_str.rbegin(), value_base_to_str.rend()};
}

int main (int argc, char **argv) {

    std::string value_str;
    int base_from, base_to;

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
        std::cerr << "base-conversion::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        value_str   = arg_parser->optionValue(OPTION_VALUE);

        base_from   = std::stoi(arg_parser->optionValue(OPTION_BASE_FROM));
        base_to     = std::stoi(arg_parser->optionValue(OPTION_BASE_TO));
    }

    delete arg_parser;

    std::cout << "base-conversion::main() : [INFO] base conversion :"
        << "\n\t(" << value_str <<", b" << base_from << ") -> (" 
            << base_convert(value_str, base_from, base_to) << ", b" << base_to << ")" << std::endl;

    exit(0);
}