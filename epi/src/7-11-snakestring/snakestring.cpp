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

#define OPTION_STRING       (char *) "string"
#define OPTION_HEIGHT       (char *) "height"

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nwrite a string sinusoidally (with variants) \
(EPI problem 7.11, page 106)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_STRING,
            "string to be converted into a 'snakestring'",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

        parser->defineOption(
            OPTION_HEIGHT,
            "height of the snakestring string.\
\ne.g. --height 1 results in 3 lines (1 middle, 1 top, 1 bottom); --height 2 results in 5 \
lines (1 middle, 2 top, 2 bottom).",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

std::string get_snakestring(std::string input_str, int amplitude) {

    std::string snakestring;

    // period of the sinusoidal, i.e. next index i of the middle 
    // row is always i += base_period.
    int base_period = 2 * amplitude;

    // pivot array for writing indeces at each sinusoidal level. each 
    // sinusoidal has (2 * amplitude) + 1 levels, i.e. 1 middle level + 
    // amplitude top levels + amplitude bottom levels
    std::vector<int> row_index;
    for (int i = 0; i < amplitude + 1; i++)
        row_index.push_back(amplitude - i);
    for (int i = (amplitude + 1); i < (2 * amplitude) + 1; i++)
        row_index.push_back(i + amplitude);

    // array for row steps at each level
    std::vector<int> row_step;
    row_step.push_back(2 * amplitude);
    for (int i = 1; i < amplitude + 1; i++)
        row_step.push_back(-((2 * amplitude) - (2 * i)));
    for (int i = (amplitude + 1); i < (2 * amplitude); i++)
        row_step.push_back(-((2 * i) - (2 * amplitude)));
    row_step.push_back(2 * amplitude);

    for (int row = 0; row < row_index.size(); row++) {
        for (int i = 0; i < input_str.size(); i++) {

            if (i == row_index[row]) {

                snakestring.push_back(input_str[i]);

                // updating the row index
                row_index[row] += (base_period + row_step[row]);
                // update the row step by flipping the signal
                if (row > 0 && row < (2 * amplitude))
                    row_step[row] = -row_step[row];

            } else {
                snakestring.push_back(' ');
            }
        }
        snakestring.push_back('\n');
    }

    return snakestring;
}

int main (int argc, char **argv) {

    std::string input_str;
    int height;

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
        std::cerr << "snakestring::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        input_str = arg_parser->optionValue(OPTION_STRING);
        height = std::stoi(arg_parser->optionValue(OPTION_HEIGHT));
    }

    delete arg_parser;

    std::cout << "snakestring::main() : [INFO] snakestring (height " << height << ") of " << input_str << ":" << std::endl;
    std::cout << get_snakestring(input_str, height) << std::endl;

    exit(0);
}