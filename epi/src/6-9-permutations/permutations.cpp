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

#include "common.h"
#include "argvparser.h"

using namespace CommandLineProcessing;

// board representation
#define OPTION_ORIGINAL     (char *) "original"
#define OPTION_PERMTATION   (char *) "permutation"
#define SEQUENCE_DELIMITER  (char) ','

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\npermute the elements of an array \
(EPI problems 6.9, page 73)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_ORIGINAL,
            "original array on which to apply the permutation \
(must be made of integers). e.g. '--original 0,1,2,3,4,5,6,7,8,9'",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_PERMTATION,
            "permutation to apply. e.g. '--original 8,6,4,2,0,9,7,5,3,1'",
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

// the key observation to solve this problem is understanding that any 
// permutation can be represented as a set of 'cyclic permutations'. 
//
// a cyclic permutation is a chain of pairwise permutations which 
// result in a cycle. e.g. consider the following permutation, in 
// 'matrix' notation:
//
//    start here
//         v
// orig. : 0 1 2 3 4 5 6 7 8 9
// perm. : 8 6 4 2 0 9 7 5 3 1
//
// let's start on the '0' in the orig array (top row), and look into 
// the number below it. this means that - according to the permutation 
// instructions on the perm array - the value at index 0 should go to 
// index 8. what about the value at index 8? reading again from bottom to 
// top, we learn index 8 should go to index 3. what about 3? well, we keep 
// following this pattern until we finally get back to index 0, resulting in 
// the following 'chain' of pairwise permutations (or 'cyclic permutation'):
// 
// 0 > 8 > 3 > 2 > 4 > 0
// 
// we can perform these pairwise permutations, inplace in the original array, 
// using std::swap(), until the cycle is complete. at that point we will have 
// the values of indexes 0, 8, 3, 2 and 4 at the right place. we then follow 
// the same procedure for the missing indeces, in this case 1. this would 
// actually result in the following cyclic permutation
// 
// 1 > 6 > 7 > 5 > 9
//
// and thus end the 'complete' permutation we were instructed to do. the trick 
// is thus to find a way to mark the elements in their final position as 
// 'already taken care of' before a new cyclic permutation starts.
void apply_permutation(
    std::vector<int> * original_ptr, 
    std::vector<int> * permutation_ptr) {

    std::vector<int> & original = *original_ptr;
    std::vector<int> & permutation = *permutation_ptr;

    // the 'taken care of' array. same size as original (or permutation), 
    // and initially all set to TRUE.
    std::deque<bool> handled(original.size(), true);

    for (int i = 0; i < original.size(); i++) {

        int next_in_cycle = i;

        // keep following the cyclic permutation, until you find its end 
        // in the 'taken care of' array.
        while (handled[next_in_cycle]) {

            // a pairwise permutation : we put the element at original[i] 
            // into its final position (e.g. 0 > 8).
            std::swap(original[i], original[permutation[next_in_cycle]]);

            // mark next_in_cycle has handled
            handled[next_in_cycle] = false;

            // note that the value at i is now the one that 
            // was previously at permutation[next_in_cycle]. we 
            // want to move it to the right place of original in the next 
            // iteration (e.g. 8 > 3) the right place is indicated by 
            // permutation[permutation[next_in_cycle]]. as such, we 
            // update next_in_cycle to permutation[next_in_cycle].
            next_in_cycle = permutation[next_in_cycle];
        }
    }
}

int main (int argc, char **argv) {

    std::string original_str, permutation_str;
    std::vector<int> original, permutation;

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
        std::cerr << "permutation::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        original_str = arg_parser->optionValue(OPTION_ORIGINAL);
        permutation_str = arg_parser->optionValue(OPTION_PERMTATION);
    }

    delete arg_parser;

    // extract original and permutation arrays
    extract_sequence(original_str, SEQUENCE_DELIMITER, &original);
    extract_sequence(permutation_str, SEQUENCE_DELIMITER, &permutation);

    // apply the permutation on original
    apply_permutation(&original, &permutation);

    std::cout << "permutation::main() : [INFO] permutation :"
        << "\n\tORIGINAL: " << original_str 
        << "\n\tPERMUTATION TO APPLY: " << permutation_str 
        << "\n\tAFTER PERMUTATION: " << to_intgr_str(&original) << std::endl;

    exit(0);
}