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

uint32_t integer_division(uint32_t dividend, uint32_t divisor) {

    // keep x2 divisor until it is > dividend. now, we can't use '>' or '<', 
    // so we must do this in some other way...
    uint32_t m = 0, quotient = 0, remainder = dividend, n_iter = 0;

    // we stop when the remainder is less than the divisor
    while (((remainder - divisor) & 0x80000000) != 0x80000000) {

        // calculate the largest power of 2 which still 'fits' in the remainder
        m = 0;
        while (((remainder - (divisor << (m + 1))) & 0x80000000) != 0x80000000) {
            m++;
            n_iter++;
        }

        // the quotient is thus incremeted by powers of 2
        quotient = quotient + (1 << m);
        // update the remainder, from which we will subtract other powers of 
        // 2, until the remainder is less than the divisor
        remainder = remainder - (divisor << m);
    }

    // std::cout << "divide::integer_division() : [INFO] q = " << quotient << std::endl;
    // std::cout << "divide::integer_division() : [INFO] r = " << remainder << std::endl;
    // std::cout << "divide::integer_division() : [INFO] n_iter = " << n_iter << std::endl;

    return quotient;
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