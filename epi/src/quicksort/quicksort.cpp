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

#define OPTION_SEQUENCE     (char *) "sequence"
#define OPERAND_DELIMITER   (char) ' '

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nquicksort implementation for \
        training purposes (context: EPI problem 6.1)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_SEQUENCE,
            "sequence of integers (separated by ' ') to be sorted",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

int partition(std::vector<int> * array_ptr, int left_brdr, int right_brdr) {

    std::vector<int> & array = *array_ptr;

    int left_ptr = left_brdr - 1, right_ptr = right_brdr;
    // chosen 'arbitrarily' (as in Sedgewick's Algo C++ book)
    int part_elmnt_value = array[right_brdr];

    for(;;) {

        // move the left pointer to the right of the left border, until 
        // the array value is larger than (or equal to) part_elmnt_value.
        while(array[++left_ptr] < part_elmnt_value);

        // move the right pointer to the left of the right border, until 
        // the array value is smaller than part_elmnt_value.
        while(part_elmnt_value < array[--right_ptr]) {

            // as safety, check if the right pointer moved all the way 
            // till the left border. if it did, finish the partitioning step.
            if (right_ptr == left_brdr)
                break;
        }

        // check if the left and right pointers have crossed. if they did, 
        // the partitioning is finished.
        if (left_ptr >= right_ptr) 
            break;

        // swap the array elements pointed by left_ptr and right_ptr
        std::swap(array[left_ptr], array[right_ptr]);
    }

    // in the end, swap array[left_ptr] with array[right_brdr]
    std::swap(array[left_ptr], array[right_brdr]);

    return left_ptr;
}

void quicksort(std::vector<int> * array_ptr, int left_brdr, int right_brdr) {

    // base case
    if (left_brdr >= right_brdr) return;

    // partition step
    int left_ptr = partition(array_ptr, left_brdr, right_brdr);
    // recursive step
    quicksort(array_ptr, left_brdr, left_ptr - 1);
    quicksort(array_ptr, left_ptr + 1, right_brdr);
}

void split_str(
    const std::string & s, 
    char delim, 
    std::vector<int> & tokens) {

    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        tokens.push_back(std::stoi(item));
    }
}

void print_array(std::vector<int> * array_ptr) {

    std::vector<int> array = *array_ptr;

    for (unsigned i = 0; i < array.size(); i++)
        std::cout << array[i] << " ";

    std::cout << std::endl;
}

int main (int argc, char **argv) {

    std::vector<int> sequence;
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
        std::cerr << "quicksort::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        std::string sequence_str = arg_parser->optionValue(OPTION_SEQUENCE);
        split_str(sequence_str, OPERAND_DELIMITER, sequence);
    }

    delete arg_parser;

    std::cout << "quicksort::main() : [INFO] unsorted array:" << std::endl;
    print_array(&sequence);

    quicksort(&sequence, 0, sequence.size() - 1);
    std::cout << "quicksort::main() : [INFO] sorted array:" << std::endl;
    print_array(&sequence);

    exit(0);
}