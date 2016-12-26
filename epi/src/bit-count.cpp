#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <ctime>
#include <thread>
#include <bitset>
#include <string>

#include "argvparser.h"

#define MAX_STRING_SIZE         256

#define OPTION_INPUT    (char *) "input-number"
using namespace CommandLineProcessing;

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ncount nr. bits set to '1' in input (EPI problem page 25)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_INPUT,
            "number for bit count",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

int get_str_arg(ArgvParser * parser, char * str, const char * arg) {

    if (parser->foundOption(arg))
        strncpy(str, (char *) parser->optionValue(arg).c_str(), MAX_STRING_SIZE);
    else
        return 0;

    return 1;
}

int get_int_arg(ArgvParser * parser, int & value, const char * arg) {

    if (parser->foundOption(arg))
        value = std::stoi(parser->optionValue(arg));
    else
        return 0;

    return 1;
}

int clear_lowest_set_bit(int & number) {

    number = (number & (number - 1));
    return number;
}

int get_number_set_bit(int number) {

    int num_set_bits = 0;
    int _num = number;

    while(_num > 0) {
        clear_lowest_set_bit(_num);
        num_set_bits++;
    }

    return num_set_bits;
}

int main (int argc, char **argv) {

    ArgvParser * arg_parser = create_argv_parser();

    // default input is '7', meaning 3 bits set to 1
    int input_number = 7;
    // parse() takes the arguments to main() and parses them according to 
    // ArgvParser rules
    int parse_result = arg_parser->parse(argc, argv);

    if (parse_result == ArgvParser::ParserHelpRequested) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        delete arg_parser;
        exit(-1);

    } else if (parse_result != ArgvParser::NoParserError) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        std::cerr << "bit-count::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        get_int_arg(arg_parser, input_number, OPTION_INPUT);
    }

    delete arg_parser;

    int num_set_bits = get_number_set_bit(input_number);
    std::cout << "bit-count::main() : [INFO] nr. of bits set in " << input_number 
        << " (" << std::bitset<16>(input_number) << ")"
        << " " << num_set_bits 
        << " parity : " << (((num_set_bits % 2) > 0) ? "ODD" : "EVEN") << std::endl;

    exit(0);
}