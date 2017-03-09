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

#define OPTION_OPERANDS     (char *) "operands"
#define OPERAND_DELIMITER   (char) '/'

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ngiven 2 positive integers - x and \
        y - compute their quotient, using only '+', '-' and shifts (EPI problem \
        5.6, page 52)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_OPERANDS,
            "dividend/divisor (separated by '/', e.g. '1005/3')",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

// taken from http://stackoverflow.com/a/236803/6660861
void split_str(
    const std::string & s, 
    char delim, 
    std::vector<std::string> & tokens) {
    
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        tokens.push_back(item);
    }
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

    std::cout << "divide::integer_division() : [INFO] q = " << quotient << std::endl;
    std::cout << "divide::integer_division() : [INFO] r = " << remainder << std::endl;
    std::cout << "divide::integer_division() : [INFO] n_iter = " << n_iter << std::endl;

    return quotient;
}

int main (int argc, char **argv) {

    ArgvParser * arg_parser = create_argv_parser();
    uint32_t dividend = 0, divisor = 0, result = 0;
    // parse() takes the arguments to main() and parses them according to 
    // ArgvParser rules
    int parse_result = arg_parser->parse(argc, argv);

    if (parse_result == ArgvParser::ParserHelpRequested) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        delete arg_parser;
        exit(-1);

    } else if (parse_result != ArgvParser::NoParserError) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        std::cerr << "divide::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        // get the input operands (passed as a string '<op1>x<op2>'')
        std::string ops_str = arg_parser->optionValue(OPTION_OPERANDS);
        // split ops_str into 2 operand strings
        std::vector<std::string> tokens;
        split_str(ops_str, OPERAND_DELIMITER, tokens);
        // extract the 2 operands
        if (tokens.size() != 2) {
            std::cerr << "divide::main() : [ERROR] only 2 operands please" << std::endl;
            delete arg_parser;
            exit(-1);
        }

        dividend = (uint32_t) std::stoi(tokens[0]);
        divisor = (uint32_t) std::stoi(tokens[1]);
    }

    delete arg_parser;

    integer_division(dividend, divisor);

    exit(0);
}