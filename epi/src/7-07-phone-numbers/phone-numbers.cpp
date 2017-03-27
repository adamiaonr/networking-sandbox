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

// keypad digit-to-char mappings
std::unordered_map<char, std::vector<char>> KEYPAD = {
    {'2', {'A', 'B', 'C'}}, 
    {'3', {'D', 'E', 'F'}}, 
    {'4', {'G', 'H', 'I'}}, 
    {'5', {'J', 'K', 'L'}}, 
    {'6', {'M', 'N', 'O'}}, 
    {'7', {'P', 'Q', 'R', 'S'}}, 
    {'8', {'T', 'U', 'V'}}, 
    {'9', {'W', 'X', 'Y', 'Z'}}};

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ncompute all mnemonics for a phone number \
(EPI problem 7.7, page 101)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_SEQUENCE,
            "phone number for which to calculate mnemonics",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

// O(n * 4^n) time solution, using recursion. the 'n *'' comes
// from the copy of mnemonic_prefix to mnemonics_ptr at each base case
void fill_mnemonics_rec(
    const std::string phone_number, unsigned pos, 
    std::string * mnemonic_prefix_ptr, 
    std::vector<std::string> * mnemonics_ptr) {

    // if we examined all digits in the phone number, add the mnemonic 
    // to the final array of mnemonics and return
    if (pos == phone_number.size()) {

        // why emplace_back() and not push_back()?
        // 
        mnemonics_ptr->emplace_back(*mnemonic_prefix_ptr);

    } else {

        // extract the current digit of phone_number
        char digit = phone_number[pos];

        // cycle through all character mappings for digit, add 
        // them to a new mnemonic prefix, follow to the next 
        // digit

        // some interesting C++ things now:
        //  - C++11 ranged-based loops make it sooo intuitive, almost like Python
        for (char c : KEYPAD[digit]) {

            // change the character at mnemonic_prefix[pos] to c. won't this mess 
            // up the other mnemonics? no, because each recursive branch goes all 
            // the way to the end of a mnemonic before calculating a new one
            (*mnemonic_prefix_ptr)[pos] = c;
            fill_mnemonics_rec(phone_number, pos + 1, mnemonic_prefix_ptr, mnemonics_ptr);
        }
    }
}

// variable depth loop, with no recursion. the key observation is 
// that in a set of nested loops, most of the work is done in the 
// innermost loop. 
//
// we can thus control the nested loop execution
// using a 'pivot array' of size n, in which each position i in [0, n - 1] 
// can take values in the range [0, KEYPAD[sequence[i]].size() - 1]. whenever 
// the i-th inner loop reaches the end of its range, we increment the 
// value of the (i - 1)-th loop. this should continue until the 
// 0-th loop (i.e. the outermost loop) reaches the end of its range.
std::vector<std::string> get_mnemonics_no_rec(const std::string phone_number) {

    // vector of strings, which will hold all possible mnemonics
    // for phone_number
    std::vector<std::string> mnemonics;
    // string used to hold a mnemonic
    std::string mnemonic_prefix(phone_number.size(), 0);
    // create an empty pivot array, of n entries set to 0
    std::vector<int> pivot(phone_number.size());

    // initialize mnemonic_prefix to the first possible 
    // mnemonic (pivot code '0000...0')
    for (unsigned i = 0; i < phone_number.size(); i++)
        mnemonic_prefix[i] = KEYPAD[phone_number[i]][0];

    // start at the deepest loop
    int curr_depth = phone_number.size() - 1;
    // keep incrementing the pivot values until the outermost index 
    // reaches the end of its range. if you analyze the conditions below,
    // this will happen when curr_depth = -1.
    while(curr_depth >= 0) {

        char digit = phone_number[curr_depth];

        // if curr_depth is the n-th loop level (i.e. the deepest) , cycle through 
        // all possible characters to generate final mnemonics and copy them to mnemonics_ptr
        if (curr_depth == (int) (phone_number.size() - 1)) {

            for (
                unsigned depth_index = 0; 
                depth_index < KEYPAD[digit].size();
                depth_index++) {

                mnemonic_prefix[curr_depth] = KEYPAD[digit][depth_index];
                mnemonics.emplace_back(mnemonic_prefix);
            }

            // we've finished the innermost loop. go up 1 level to the (n - 1)-th loop
            curr_depth--;

        } else {

            if (pivot[curr_depth] < (int) (KEYPAD[digit].size() - 1)) {

                // udpate mnemonic_prefix
                mnemonic_prefix[curr_depth] = KEYPAD[digit][++pivot[curr_depth]];
                // go down 1 level
                curr_depth++;
            
            } else {

                // we're at the end of the character range for the current 
                // depth. reset it and go back 1 level.
                pivot[curr_depth] = -1;
                curr_depth--;
            }
        }
    }

    return mnemonics;
}

std::vector<std::string> get_mnemonics(const std::string phone_number) {

    // string, whose pointer will be passed on each recursive call, 
    // which will be used to build an individual mnemonic
    std::string mnemonic_prefix(phone_number.size(), 0);
    // vector of strings, which will hold all possible mnemonics
    // for phone_number
    std::vector<std::string> mnemonics;
    // fill the mnemonics array
    fill_mnemonics_rec(phone_number, 0, &mnemonic_prefix, &mnemonics);

    return mnemonics;
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
        std::cerr << "phone-numbers::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        sequence = arg_parser->optionValue(OPTION_SEQUENCE);
    }

    delete arg_parser;

    std::vector<std::string> mnemonics_rec = get_mnemonics(sequence);
    std::cout << "phone-numbers::main() : [INFO] (recursive) " << mnemonics_rec.size() << " mnemonics for " 
        << sequence << std::endl;

    std::vector<std::string> mnemonics_no_rec = get_mnemonics_no_rec(sequence);
    std::cout << "phone-numbers::main() : [INFO] (non-recursive) " << mnemonics_no_rec.size() << " mnemonics for " 
        << sequence << std::endl;

    if (mnemonics_rec.size() != mnemonics_no_rec.size()) {

        std::cout << "phone-numbers::main() : [INFO] sizes are inconsistent : recursive " 
            << mnemonics_rec.size() << " vs. non-recursive " << mnemonics_no_rec.size() << std::endl;

    } else {

        for (unsigned i = 0; i < mnemonics_rec.size(); i++) {

            if (mnemonics_rec[i] != mnemonics_no_rec[i]) {
                std::cout << "phone-numbers::main() : [INFO] inconsistent mnemonics found" << std::endl;
                exit(1);
            }
        }

        std::cout << "phone-numbers::main() : [INFO] mnemonics are consistent" << std::endl;
    }

    exit(0);
}