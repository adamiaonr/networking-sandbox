#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <ctime>
#include <thread>
#include <bitset>
#include <string>
#include <vector>

#include "common.h"
#include "argvparser.h"

using namespace CommandLineProcessing;

#define OPTION_NUMBER       (char *) "number"

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nreverses digits of 1 integer \
(EPI problem 5.8, page 54)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_NUMBER,
            "number to be reversed (e.g. '--number 413' should result in 314)",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

int main (int argc, char **argv) {

    ArgvParser * arg_parser = create_argv_parser();
    int input = 0, result = 0;
    // parse() takes the arguments to main() and parses them according to 
    // ArgvParser rules
    int parse_result = arg_parser->parse(argc, argv);

    if (parse_result == ArgvParser::ParserHelpRequested) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        delete arg_parser;
        exit(-1);

    } else if (parse_result != ArgvParser::NoParserError) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        std::cerr << "reverse::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        // get the input operands (passed as a string '<op1>x<op2>'')
        std::string number_str = arg_parser->optionValue(OPTION_NUMBER);
        input = (int) std::stoi(number_str);
    }

    delete arg_parser;

    int number = input;
    while (number > 0) {
        
        result = (result * 10) + (number % 10);
        number = number / 10;
    }

    std::cout << "reverse::main() : [INFO] " << input << "->" << result << std::endl;

    exit(0);
}