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

// size of random sequence to generate
#define OPTION_SEQUENCE_SIZE    (char *) "seq-size"
#define OPTION_NUM_COLORS       (char *) "num-colors"

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ndutch national flag problem \
(EPI problem 6.1, page 61)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_SEQUENCE_SIZE,
            "size of random sequence to be sorted (default: 10)",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_NUM_COLORS,
            "nr. of colors of the flag (default: 3)",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

std::string to_flag_str(std::vector<int> * flag_ptr) {

    std::vector<int> flag = *flag_ptr;
    std::string flag_str = "";

    for (unsigned i = 0; i < flag.size(); i++)
        flag_str = flag_str + std::to_string(flag[i]) + " ";

    return flag_str;
}

void gen_random_flag(int size, int num_colors, std::vector<int> * flag_ptr) {

    std::vector<int> & flag = *flag_ptr;

    for (int i = 0; i < size; i++)
        flag.push_back(rand() % num_colors);
}

void three_way_partition(int pivot, std::vector<int> * flag_ptr) {

    std::vector<int> & flag = *flag_ptr;

    // typical variables of a 2-way partition step in quicksort
    int left_brdr = 0, right_brdr = (flag.size() - 1);
    int left_ptr = left_brdr - 1, right_ptr = right_brdr + 1;
    // these additional pointers delimit the 'equal' portions of the flag, 
    // located at the very left and right ends of it.
    int equal_left_ptr = left_brdr - 1, equal_right_ptr = right_brdr + 1;
    // get the color value of the pivot, whose index is passed as argument
    int pivot_color = flag[pivot];

    // std::cout << "dutch::three_way_partition() : [INFO] pivot : " << pivot << ", value : " << pivot_color << std::endl;

    for(;;) {

        // move the left pointer to the right of the left border, until 
        // the flag color is larger than (or equal to) pivot_color.
        while(flag[++left_ptr] < pivot_color);

        // move the right pointer to the left of the right border, until 
        // the flag color is smaller than (or equal to) pivot_color.
        while(pivot_color < flag[--right_ptr]);

        // if the left and right cross, the partitioning is finished.
        if (left_ptr >= right_ptr) 
            break;

        // std::cout << "dutch::three_way_partition() : [INFO] swapping flag[" 
        //     << left_ptr << "] : " << flag[left_ptr] << " (left) with flag[" 
        //     << right_ptr << "] : " << flag[right_ptr] << " (right)" << std::endl;

        std::swap(flag[left_ptr], flag[right_ptr]);

        // additional swap operations depend on whether the left and right 
        // pointers point to values equal to pivot_color. swap left_ptr to the 
        // left end of the left part of the flag. same for right_ptr. 
        if (flag[left_ptr] == pivot_color) {

            std::swap(flag[++equal_left_ptr], flag[left_ptr]);

            // std::cout << "dutch::three_way_partition() : [INFO] equal swapped (1) flag[" 
            //     << equal_left_ptr << "] : " << flag[equal_left_ptr] << " (left) with flag[" 
            //     << left_ptr << "] : " << flag[left_ptr] << " (right)" << std::endl;
        }

        if (flag[right_ptr] == pivot_color) {

            std::swap(flag[right_ptr], flag[--equal_right_ptr]);

            // std::cout << "dutch::three_way_partition() : [INFO] equal swapped (2) flag[" 
            //     << right_ptr << "] : " << flag[right_ptr] << " (left) with flag[" 
            //     << equal_right_ptr << "] : " << flag[equal_right_ptr] << " (right)" << std::endl;
        }
    }

    std::cout << "dutch::three_way_partition() : [INFO] flag (after 1st stage) : " << to_flag_str(&flag) << std::endl;

    // note that left_ptr is now 'to the right' of right_ptr by +1. and right_ptr 
    // is 'to the left' of left_ptr by +1. update them.
    left_ptr = left_ptr - 1;
    right_ptr = right_ptr + 1;

    // we move the 'equal' portions of the array into position, by swapping 
    // their values with those on the respective 'smaller' / 'larger' portions.
    std::cout << "dutch::three_way_partition() : [INFO] pointers: "<< std::endl;
    std::cout << "\t[EQUAL_LEFT_PTR] : " << equal_left_ptr << std::endl;
    std::cout << "\t[LEFT_PTR] : " << left_ptr << std::endl;
    std::cout << "\t[EQUAL_RIGHT_PTR] : " << equal_right_ptr << std::endl;
    std::cout << "\t[RIGHT_PTR] : " << right_ptr << std::endl;

    for (int k = left_brdr; k <= equal_left_ptr; k++, left_ptr--)
        std::swap(flag[k], flag[left_ptr]);

    for (int k = right_brdr; k >= equal_right_ptr; k--, right_ptr++)
        std::swap(flag[k], flag[right_ptr]);
}

int main (int argc, char **argv) {

    int pivot = 0, size = 0, num_colors = 0;
    std::vector<int> flag;

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
        std::cerr << "dutch::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        size = std::stoi(arg_parser->optionValue(OPTION_SEQUENCE_SIZE));
        num_colors = std::stoi(arg_parser->optionValue(OPTION_NUM_COLORS));
    }

    delete arg_parser;

    // generate a random (unordered) flag
    // initialize pseudo random number generator
    srand(time(NULL));
    gen_random_flag(size, num_colors, &flag);
    std::cout << "dutch::main() : [INFO] flag : " << to_flag_str(&flag) << std::endl;

    // pivot index is chosen randomly
    pivot = rand() % size;
    std::cout << "dutch::main() : [INFO] pivot : " << pivot << ", value : " << flag[pivot] << std::endl;

    // perform the 3-way partition
    three_way_partition(pivot, &flag);
    std::cout << "dutch::main() : [INFO] 'dutched' flag : " << to_flag_str(&flag) << std::endl;

    exit(0);
}