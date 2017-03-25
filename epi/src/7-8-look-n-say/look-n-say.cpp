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

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ncalcs n-th 'look-n-say' integer \
(EPI problem page 27)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_INPUT,
            "index of 'look-n-say' integer to determine",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

int get_look_n_say_str(
    char * input_number_str, 
    char * output_number_str) {

    int digit_qty = 0, insert_point = 0;

    memset(output_number_str, 0, strlen(output_number_str));

    snprintf(&output_number_str[0], 2, "%d", ++digit_qty);
    snprintf(&output_number_str[1], 2, "%c", input_number_str[0]);

    // std::cout << "look-n-say::get_look_n_say_str() : [INFO] look-n-say nr.: " << input_number_str 
    //     << " -> " << output_number_str << std::endl;

    for (unsigned int i = 1; i < strlen(input_number_str); i++) {

        if (input_number_str[i] != input_number_str[i - 1]) {

            insert_point += 2;
            digit_qty = 0;
        }

        snprintf(&output_number_str[insert_point], 2, "%d", ++digit_qty);
        snprintf(&output_number_str[insert_point + 1], 2, "%c", input_number_str[i]);

        // std::cout << "look-n-say::get_look_n_say_str() : [INFO] updating look-n-say nr.: " << input_number_str 
        //     << " -> " << output_number_str << std::endl;
    }

    return 0;
}

int main (int argc, char **argv) {

    ArgvParser * arg_parser = create_argv_parser();

    int index = 1;
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

        Common::get_int_arg(arg_parser, index, OPTION_INPUT);
    }

    delete arg_parser;

    char input_number_str[MAX_DIGITS] = "1", output_number_str[MAX_DIGITS] = "\0";

    for (int i = 0; i < index; i++) {

        get_look_n_say_str(input_number_str, output_number_str);
        std::cout << "look-n-say::main() : [INFO] " << i << "-th look-n-say nr.: " << input_number_str 
            << " -> " << output_number_str << std::endl;

        memset(input_number_str, 0, strlen(input_number_str));
        strncpy(input_number_str, output_number_str, MAX_DIGITS);
    }

    std::cout << "look-n-say::main() : [INFO] " << index << "-th look-n-say nr.: " 
        << output_number_str << std::endl;

    exit(0);
}