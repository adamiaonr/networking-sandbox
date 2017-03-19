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

// board representation
#define OPTION_SEQUENCE     (char *) "seq"
#define SEQUENCE_DELIMITER  (char) ','

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ndelete duplicates from a sorted array \
(EPI problem 6.5, page 68)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_SEQUENCE,
            "representation of sorted sequence. e.g. '--seq 2,3,5,5,7,11,11,11,13'",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

std::string to_intgr_str(std::vector<int> * intgr_ptr) {

    std::vector<int> intgr = *intgr_ptr;
    std::string intgr_str = "";

    for (int i = 0; i < intgr.size(); i++)
        intgr_str = intgr_str + std::to_string(intgr[i]) + ",";

    return intgr_str;
}

void extract_sequence(
    const std::string & s, 
    char delim, 
    std::vector<int> * seq_ptr) {

    std::vector<int> & seq = *seq_ptr;

    // we use a stringstream object for quick token separation, 
    // avoiding the use of the complicated strtok() function
    std::stringstream ss;
    // initialize ss from string s
    ss.str(s);
    // get each board position using getline()
    std::string seq_pos;
    while (std::getline(ss, seq_pos, delim)) {
        seq.push_back(std::stoi(seq_pos));
    }
}

int delete_duplicates(std::vector<int> * seq_ptr) {

    std::vector<int> & seq = *seq_ptr;
    int last_unique_pos = 0, last_value = seq[0];

    for (int i = 1; i < seq.size(); i++) {

        if (seq[i] != last_value) {

            last_value = seq[i];
            last_unique_pos++;
            std::swap(seq[last_unique_pos], seq[i]);
        }
    }

    // set remaining parts of the array to 0
    for (int i = last_unique_pos + 1; i < seq.size(); i++)
        seq[i] = 0;

    return (last_unique_pos + 1);
}

int main (int argc, char **argv) {

    std::string seq_str;
    std::vector<int> seq;

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
        std::cerr << "duplicates::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        seq_str = arg_parser->optionValue(OPTION_SEQUENCE);
    }

    delete arg_parser;

    // extract the board to an array of int
    extract_sequence(seq_str, SEQUENCE_DELIMITER, &seq);

    int num_valid = delete_duplicates(&seq);
    std::cout << "duplicates::main() : [INFO] deleted duplicates (" <<  num_valid << " valid elements) :"
        << "\n\tORIGINAL : " << seq_str 
        << "\n\tDELETED DUPLICATES : " << to_intgr_str(&seq) << std::endl;    

    exit(0);
}