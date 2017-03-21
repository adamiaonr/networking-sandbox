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

#define OPTION_SET          (char *) "set"
#define OPTION_SET_SIZE     (char *) "set-size"
#define OPTION_SUBSET_SIZE  (char *) "subset-size"
#define SEQUENCE_DELIMITER  (char) ','

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ncompute a random permutation and random subset \
(EPI problem 6.13 and 6.14, pages 80 and 81)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_SET,
            "set of distinct (integer) elements e.g. '--set 0,1,2,3,4,5,6,7,8,9'",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_SET_SIZE,
            "size of set from which to generate a random subset. e.g. '--set-size 100'",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_SUBSET_SIZE,
            "size of random subset to generate. e.g. '--subset-size 4'",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

std::string to_intgr_str(std::vector<int> * intgr_ptr) {

    std::vector<int> intgr = *intgr_ptr;
    std::string intgr_str = "";

    for (int i = 0; i < intgr.size(); i++)
        intgr_str = intgr_str + std::to_string(intgr[i]) + " ";

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

// solution which calls PRNG O(n) times
std::vector<int> gen_random_permutation(std::vector<int> set) {

    int remaining_in_set = set.size();
    std::vector<int> permutation;

    // UPDATE: just found out that rand() mod n is a bad 
    // idea to get a uniform distribution for [0, n - 1].
    // instead, use some C++11 fancy list of things, as shown below:
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());

    while (remaining_in_set > 1) {

        // pick an index of the set at random, in the range 
        // [0, (remaining_in_set - 1)]
        std::uniform_int_distribution<int> distr(0, --remaining_in_set);
        int random_index = distr(generator);
        // add set[random_index] to the permutation
        permutation.push_back(set[random_index]);
        // discard the picked value in set, by swapping it with 
        // the last eligible element of the set
        std::swap(set[random_index], set[remaining_in_set]);

        std::cout << "random-permutation::gen_random_permutation() : " << to_intgr_str(&set) << std::endl;
    }

    // avoids 1 call to PRNG (in the end, only set[0] is free for
    // picking)
    permutation.push_back(set[0]);

    return permutation;
}

// wasn't able to think about this one before checking 
// the solution, but it's so smart, i had to replicate it 
// myself : it's O(k) in space and time, as it doesn't 
// require the generation of a n sized set with 
// elements {0, 1, ..., n - 1}.
//
// the key observation is that only k entries (out of n) of the 
// set used to generate the subset are modified. when modified, 
// these entries become set[i] != i, while all others remain 
// with the typical set[i] = i mappings. so why not track these 
// abnormal mappings only? the word 'mapping' is key here : what 
// we need is a 'map' (or hash table). here's how it works:
//  - for a position i of the subset (i < k), we pick a random value r
//  - if neither r nor i are in the hash table H, we add 2 entries to H:
//      - H[i] = r (the first of k subset elements)
//      - H[r] = i (i is still a valid pick, which can be eventually 
//        chosen in the future if r is chosen again)
//  - for i + 1, if r is chosen again, we update H as:
//      - H[i] = r (as before)
//      - H[i + 1] = H[r] = i (i becomes one of the elements of the subset)
//      - H[r] = i + 1 (since i + 1 is now used as a subset index in H, we 
//        update H[r] to map to i + 1. note that i + 1 is still a valid pick)
//  - for i + m (with i + m < k), in which H[i + m] is already occupied (e.g. 
//     because a previous r was equal to i + m). notice that it should be 
//     occupied with a number that is "still game":
//      - H[r] = H[i + m]
//      - H[i + m] = r
//
// and that's it! brilliant.
std::vector<int> gen_random_subset(int set_size, int subset_size) {

    // the map H of modified values
    std::unordered_map<int, int> H;
    // random number generator
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());

    for (int i = 0; i < subset_size; i++) {

        // generate a random value in the range [k, (set_size - 1)]
        std::uniform_int_distribution<int> distr(i, (set_size - 1));
        int random_index = distr(generator);

        // are random_index and/or i already in H?
        auto random_index_ptr = H.find(random_index);
        auto i_ptr = H.find(i); 

        // we have x possibilities:
        //  1) neither random_index nor i are in H
        //  2) random_index is chosen again and i hasn't been chosen yet
        //  3) i is already occupied (e.g. because a previous random_index was equal 
        //     to i) and random_index isn't in H 
        //  4) both random_index and i are in H
        if (random_index_ptr == H.end() && i_ptr == H.end()) {

            H[i] = random_index;
            H[random_index] = i;

        } else if (random_index_ptr != H.end() && i_ptr == H.end()) {

            // set H[i] to the value mapped to H[random_index]
            H[i] = random_index_ptr->second;
            // update H[random_index] to i
            random_index_ptr->second = i;

        } else if (random_index_ptr == H.end() && i_ptr != H.end()) {

            H[random_index] = i_ptr->second;
            i_ptr->second = random_index;

        } else {

            int temp = i_ptr->second;
            H[i] = random_index_ptr->second;
            H[random_index] = temp;
        }
    }

    std::vector<int> subset;
    for (int i = 0; i < subset_size; i++)
        subset.emplace_back(H[i]);

    return subset;
}

int main (int argc, char **argv) {

    std::string set_str;
    int set_size, subset_size;
    std::vector<int> set, subset, permutation;

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
        std::cerr << "random-permutation::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        set_str = arg_parser->optionValue(OPTION_SET);
        set_size = std::stoi(arg_parser->optionValue(OPTION_SET_SIZE));
        subset_size = std::stoi(arg_parser->optionValue(OPTION_SUBSET_SIZE));
    }

    delete arg_parser;

    // extract set
    extract_sequence(set_str, SEQUENCE_DELIMITER, &set);
    // generate random subset
    permutation = gen_random_permutation(set);
    subset = gen_random_subset(set_size, subset_size);

    std::cout << "random-permutation::main() : [INFO] permutation :"
        << "\n\tSET: " << set_str 
        << "\n\tRANDOM PERMUTATION: " << to_intgr_str(&permutation)
        << "\n\tRANDOM SUBSET (" << subset_size << " out of " << set_size << "): " << to_intgr_str(&subset) << std::endl;

    exit(0);
}