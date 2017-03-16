#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <algorithm>
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

// size of random integer to generate
#define OPTION_INT_SIZE (char *) "int-size"
#define OPTION_TO_ADD   (char *) "to-add"

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nincrement arbitrary precision integer \
(EPI problem 6.2, page 65)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_INT_SIZE,
            "size of the integer, in nr. of digits",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_TO_ADD,
            "integer to be added",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

std::string to_intgr_str(std::vector<int> * intgr_ptr) {

    std::vector<int> intgr = *intgr_ptr;
    std::string intgr_str = "";

    for (int i = (intgr.size() - 1); i >= 0; i--)
        intgr_str = intgr_str + std::to_string(intgr[i]);

    return intgr_str;
}

void gen_random_intgr(int size, std::vector<int> * intgr_ptr) {

    std::vector<int> & intgr = *intgr_ptr;

    // an intgr is saved w/ least significant digit in 
    // the first position of the array. this makes 
    // it easier for calculations.

    // for the remaining positions, add random digit from 0 to 9 
    // to sequence
    for (int i = 0; i < (size - 1) ; i++)
        intgr.push_back(rand() % 10);

    // last position holds the most significant digit, 
    // which must be between 1 and 9.
    intgr.push_back((rand() % 9) + 1);
}

void add(std::vector<int> * intgr_ptr, int add) {

    std::vector<int> & intgr = *intgr_ptr;

    // if an individual digit sum goes over 9, 
    // we carry +1 to the digit on the 'left'
    int carry = 0;

    // we start with the least significant digit, and 
    // then go on from it.
    for (unsigned i = 0; i < intgr.size(); i++) {

        // std::cout << "increment::add() : [INFO] @intgr[" << i << "] " << intgr[i] << std::endl;

        intgr[i] = intgr[i] + (add % 10) + carry;
        // std::cout << "\t add (unadjusted) : " << intgr[i] << std::endl;
        carry = intgr[i] / 10;
        // std::cout << "\t carry : " << carry << std::endl;
        intgr[i] = intgr[i] % 10;
        // std::cout << "\t add (adjusted) : " << intgr[i] << std::endl;

        // remove the added quantity from the add value
        add = (add / 10);
        // std::cout << "\t to add : " << add << std::endl;
    }

    // if carry is still 1, add a '1' to the end of the 
    // intgr array
    if (carry > 0) intgr.push_back(1);
}

int main (int argc, char **argv) {

    int to_add = 0, size = 0;
    std::vector<int> intgr;

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
        std::cerr << "increment::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        size = std::stoi(arg_parser->optionValue(OPTION_INT_SIZE));
        to_add = std::stoi(arg_parser->optionValue(OPTION_TO_ADD));
    }

    delete arg_parser;

    // generate a random (unordered) flag
    // initialize pseudo random number generator
    srand(time(NULL));
    gen_random_intgr(size, &intgr);
    std::cout << "increment::main() : [INFO] intgr : " << to_intgr_str(&intgr) << std::endl;
    std::cout << "increment::main() : [INFO] to add : " << to_add << std::endl;

    // perform the 3-way partition
    add(&intgr, to_add);
    std::cout << "increment::main() : [INFO] result : " << to_intgr_str(&intgr) << std::endl;

    exit(0);
}