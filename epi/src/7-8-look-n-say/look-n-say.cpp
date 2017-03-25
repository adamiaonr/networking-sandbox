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

#define OPTION_N        (char *) "n"
#define MAX_DIGITS      2048

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nthe look-and-say problem \
(EPI problem 7.8, page 102)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_N,
            "index of 'look-n-say' integer to determine",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

std::string get_look_n_say(int n) {

    std::string prev_number = "1", next_number;

    // generate all look-n-say numbers till the n-th
    for (int i = 1; i < n; i++) {

        next_number = "";

        for (unsigned j = 0; j < prev_number.size(); j++) {

            int digit_cnt = 1;
            while ((j < prev_number.size() - 1) && prev_number[j] == prev_number[j + 1]) { ++j; ++digit_cnt; }
            next_number.push_back((char) (digit_cnt + (int) '0'));
            next_number.push_back(prev_number[j]);
        }

        std::cout << "look-n-say::get_look_n_say() : [INFO] next_number[" << i << "] = " << next_number << std::endl;
        prev_number = next_number;
    }

    return next_number;
}

int main (int argc, char **argv) {

    ArgvParser * arg_parser = create_argv_parser();

    int n = 0;
    // parse() takes the arguments to main() and parses them according to 
    // ArgvParser rules
    int parse_result = arg_parser->parse(argc, argv);

    if (parse_result == ArgvParser::ParserHelpRequested) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        delete arg_parser;
        exit(-1);

    } else if (parse_result != ArgvParser::NoParserError) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        std::cerr << "look-n-say::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        n = std::stoi(arg_parser->optionValue(OPTION_N));
    }

    delete arg_parser;

    std::cout << "look-n-say::main() : [INFO] " << n << "-th look-n-say nr.: " 
        << get_look_n_say(n) << std::endl;

    exit(0);
}