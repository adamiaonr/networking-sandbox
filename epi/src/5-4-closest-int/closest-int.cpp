#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <ctime>
#include <thread>
#include <bitset>
#include <string>
#include <vector>

#include "common.h"
#include "argvparser.h"

using namespace CommandLineProcessing;

#define OPTION_INPUT    (char *) "input"
#define MAX_DIGITS      2048
#define UINT_SIZE   (sizeof(unsigned int) * 8)

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nclosest integer w/ same weight (EPI problem page 50)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_INPUT,
            "input number",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

unsigned int rightmost_bit_set(unsigned int x) {

    return x & (~(x - 1));
}

unsigned int rightmost_bit_unset(unsigned int x) {

    return ~x & (x + 1);
}

unsigned int get_number_set_bit(unsigned int x) {

    int num_set_bits = 0;

    while(x) {
        num_set_bits += x & 1;
        x >>= 1;
    }

    return num_set_bits;
}

int main (int argc, char **argv) {

    ArgvParser * arg_parser = create_argv_parser();

    unsigned int input = 0, output = 0, pos = 0;
    // parse() takes the arguments to main() and parses them according to 
    // ArgvParser rules
    int parse_result = arg_parser->parse(argc, argv);

    if (parse_result == ArgvParser::ParserHelpRequested) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        delete arg_parser;
        exit(-1);

    } else if (parse_result != ArgvParser::NoParserError) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        std::cerr << "closest-int::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        input = (unsigned int) std::stoi(arg_parser->optionValue(OPTION_INPUT));
    }

    delete arg_parser;

    // trivial cases 0 and all 1s
    if (!input) {
        output = 0;
    } else if (!(~input)) {
        output = ~0;
    // the trick is to swap the rightmost consecutive bits which are 
    // different from each other (see EPI 50)
    } else if ((pos = rightmost_bit_set(input)) > 1) {

        // to swap the rightmost consecutive bits which are different, we :
        //  1) isolate the RBS
        //  2) set a bit to 1 to the right of RBS : (pos | (pos >> 1))
        //  3) XOR input with (pos | (pos >> 1))

        // e.g. say input = 36 (00100100b). RBS = 4 (100b). step 2 would result 
        // in RBS' = 6 (110b), and XORing input w/ RBS' yields 34 (00100010b).
        std::cout << "closest-int::main() : [INFO] RBS = " << pos << " RBU' = " << (pos | (pos >> 1)) << std::endl;
        output = input ^ (pos | (pos >> 1));

    } else {
        // same as above, but now we look for the rightmost UNSET bit (all bits 
        // to its right are 1)
        pos = rightmost_bit_unset(input);
        std::cout << "closest-int::main() : [INFO] RBU = " << pos << " RBU' = " << (pos | (pos >> 1)) << std::endl;
        output = input ^ (pos | (pos >> 1));
    }

    std::cout << "closest-int::main() : [INFO] closest integer results:" << std::endl
        << "\t[x] : " << input << ", bin : " << std::bitset<UINT_SIZE>(input) << ", weight : " << get_number_set_bit(input) << std::endl
        << "\t[y] : " << output << ", bin : " << std::bitset<UINT_SIZE>(output) 
            << ", weight : " << get_number_set_bit(output) 
            << ", |y - x| : " << abs((int) input - (int) output) << std::endl;

    exit(0);
}