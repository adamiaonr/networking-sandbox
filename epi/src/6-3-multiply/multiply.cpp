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

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nmultiply 2 arbitrary precision integers \
(EPI problem 6.3, page 65)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_INT_SIZE,
            "size of the integer, in nr. of digits",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

std::string to_intgr_str(std::vector<int> * intgr_ptr) {

    std::vector<int> intgr = *intgr_ptr;
    std::string intgr_str = "";

    int i = intgr.size();
    while (intgr[--i] == 0);
    for (; i >= 0; i--)
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
    // which must be between 1 and 9, and can be negative.
    const int head[18] = {-9, -8, -7, -6, -5, -4, -3, -2, -1, 1, 2, 3 , 4, 5 , 6, 7, 8, 9};
    intgr.push_back(head[rand() % 18]);
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

void multiply(
    std::vector<int> * oprnd_1_ptr, 
    std::vector<int> * oprnd_2_ptr,
    std::vector<int> * res_ptr) {

    std::vector<int> oprnd_1 = *oprnd_1_ptr;
    std::vector<int> oprnd_2 = *oprnd_2_ptr;
    std::vector<int> & res = *res_ptr;

    // if an individual digit sum goes over 9, we carry +1 to the digit on 
    // the 'left'
    unsigned carry = 0, i = 0, j = 0;

    // take care of the final sign
    int sign = 1;
    if ((oprnd_2.back() * oprnd_1.back()) < 0) {
        sign = -1;
    }

    oprnd_1.back() = to_abs(oprnd_1.back());
    oprnd_2.back() = to_abs(oprnd_2.back());

    // pre-allocate size in the res array
    for (i = 0; i < (oprnd_1.size() + oprnd_2.size() + 1); i++)
        res.push_back(0);

    // we start with the least significant digit, and then go on from it.
    for (i = 0; i < oprnd_2.size(); i++) {
        for (j = 0; j < oprnd_1.size(); j++) {

            res[i + j] = res[i + j] + (oprnd_2[i] * oprnd_1[j]) + carry;
            carry = res[i + j] / 10;
            res[i + j] = res[i + j] % 10;
        }

        // if carry is still > 0, add it to the end of the 
        // intgr array
        if (carry > 0) res[i + j++] = carry;
        carry = 0;
    }

    res[i + j - 2] *= sign;
}

int main (int argc, char **argv) {

    int size = 0;
    std::vector<int> oprnd_1, oprnd_2, result;

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
        std::cerr << "multiply::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        size = std::stoi(arg_parser->optionValue(OPTION_INT_SIZE));
    }

    delete arg_parser;

    // generate a random (unordered) flag
    // initialize pseudo random number generator
    srand(time(NULL));
    gen_random_intgr(size, &oprnd_1);
    gen_random_intgr(size, &oprnd_2);
    std::cout << "multiply::main() : [INFO] multiply : " << to_intgr_str(&oprnd_1) << " x " << to_intgr_str(&oprnd_2) << std::endl;

    // multiply and show result
    multiply(&oprnd_1, &oprnd_2, &result);
    std::cout << "multiply::main() : [INFO] result : " << to_intgr_str(&result) 
        << " (" << std::stoi(to_intgr_str(&oprnd_1)) * std::stoi(to_intgr_str(&oprnd_2)) << ")" << std::endl;

    exit(0);
}