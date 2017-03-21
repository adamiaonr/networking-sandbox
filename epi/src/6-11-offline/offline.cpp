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
#include <deque>
#include <random>

#include "common.h"
#include "argvparser.h"

using namespace CommandLineProcessing;

// board representation
#define OPTION_SET          (char *) "set"
#define OPTION_SUBSET_SIZE  (char *) "subset-size"
#define SEQUENCE_DELIMITER  (char) ','

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nEPI problem 6.11 (page 77)\n\n\nby \
adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_SET,
            "set of distinct (integer) elements e.g. '--set 0,1,2,3,4,5,6,7,8,9'",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_SUBSET_SIZE,
            "size of random subset to generate. e.g. '--subset-size 4'",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

std::string to_intgr_str(std::vector<int> * intgr_ptr, int subset_size = 0) {

    std::vector<int> intgr = *intgr_ptr;
    std::string intgr_str = "";

    int left_limit = 0;
    if (subset_size > 0)
        left_limit = intgr.size() - subset_size;

    for (int i = intgr.size() - 1; i >= left_limit; i--)
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

void gen_random_subset(std::vector<int> * set_ptr, int subset_size) {

    std::vector<int> & set = *set_ptr;
    int last_pos = set.size();

    // UPDATE: just found out that rand() mod n is a bad 
    // idea to get a uniform distribution for [0, n - 1].
    // instead, use some C++11 fancy list of things, as shown below:
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());

    while (subset_size > 0) {

        // pick an index of the set at random
        // int rand_index = (rand() % (subset_size--));
        std::uniform_int_distribution<int> distr(0, subset_size--);
        int rand_index = distr(generator);
        // swap element at rand_index with last 
        // eligible element of the array
        std::swap(set[rand_index], set[--last_pos]);
    }
}

int main (int argc, char **argv) {

    std::string set_str;
    std::vector<int> set;
    int subset_size;

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
        std::cerr << "offline::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        set_str = arg_parser->optionValue(OPTION_SET);
        subset_size = std::stoi(arg_parser->optionValue(OPTION_SUBSET_SIZE));
    }

    delete arg_parser;

    // extract set
    extract_sequence(set_str, SEQUENCE_DELIMITER, &set);
    // generate random subset
    gen_random_subset(&set, subset_size);

    std::cout << "offline::main() : [INFO] permutation :"
        << "\n\tSET: " << set_str 
        << "\n\tRANDOM SUBSET (" << subset_size << "): " << to_intgr_str(&set, subset_size) << std::endl;

    exit(0);
}