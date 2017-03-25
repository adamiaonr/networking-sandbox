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

#define OPTION_SEQUENCE     (char *) "sequence"

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nreplace and remove \
(EPI problem 7.4, pages 97)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_SEQUENCE,
            "array of characters on which to 'replace & remove'",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

// O(n) time and O(1) space solution : 2 passes of the array
std::string replace_remove(std::string sequence) {

    int extra_chars = 0, prev_size = sequence.size() - 1;

    // 1st pass : determine nr. of extra characters to add, by adding +1 for 
    // each valid a, and by taking -1 for each b
    for (unsigned i = 0; i < sequence.size(); i++) {

        if (sequence[i] == 'b') {
            extra_chars--;
        } else if (sequence[i] == 'a') {
            extra_chars++;
        }
    }

    // add extra slots (fill 'em with '-') for extra chars
    for (int i = 0; i < extra_chars; i++)
        sequence.push_back('-');

    // 2nd pass : starting from the end, replace a's with d's, add an extra 
    // d and move (...)
    int write_index = sequence.size() - 1;
    for(int i = prev_size; i >= 0; i--) {

        if (sequence[i] == 'a') {

            // write 2 d's at write pointer
            sequence[write_index--] = 'd';
            sequence[write_index--] = 'd';

        } else if (sequence[i] != 'b') {
            sequence[write_index--] = sequence[i];
        }
    }

    return sequence;
}

int main (int argc, char **argv) {

    std::string sequence;

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
        std::cerr << "replace-remove::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        sequence = arg_parser->optionValue(OPTION_SEQUENCE);
    }

    delete arg_parser;

    std::cout << "replace-remove::main() : [INFO] replace & remove:"
        << "\n\tORIGINAL: " << sequence
        << "\n\tREPLACEd & REMOVED: " << replace_remove(sequence) << std::endl;

    exit(0);
}