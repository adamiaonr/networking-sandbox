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

    parser->setIntroductoryDescription("\n\nbuy and sell a stock once \
(EPI problem 6.6, page 69)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_SEQUENCE,
            "representation of stock price sequence. e.g. '--seq 310,315,275,295,260,290,230,255,250'",
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

// typical O(n^2) solution
int get_max_profit(std::vector<int> * seq_ptr) {

    int max_profit = 0;
    std::vector<int> & seq = *seq_ptr;

    for (unsigned i = 0; i < seq.size(); i++) {
        for (unsigned j = (i + 1); j < seq.size(); j++) {

            // std::cout << "stock::get_max_profit() : [INFO] profit (" 
            //     << i << "," << j << ") : " 
            //     << (seq[j] - seq[i]) << std::endl;
            max_profit = std::max(max_profit, (seq[j] - seq[i]));
        }
    }

    return max_profit;
}

// O(n) solution, using the 'current lowest buy price' insight
int get_max_profit_2(std::vector<int> * seq_ptr) {

    std::vector<int> & seq = *seq_ptr;
    int curr_lowest_buy_pos = 0, min_buy_price = seq[0], max_profit = 0;

    for (unsigned i = 1; i < seq.size(); i++) {

        // get the max. profit, by comparing the current (selling) price 
        // against the lowest buying price found so far
        // std::cout << "stock::get_max_profit_2() : [INFO] profit (" 
        //     << curr_lowest_buy_pos << "," << i << ") : " 
        //     << (seq[i] - seq[curr_lowest_buy_pos]) << std::endl;
        max_profit = std::max(max_profit, (seq[i] - seq[curr_lowest_buy_pos]));

        // update the buying price if you find a cheaper price for the stock
        // along the way...
        if (seq[i] < min_buy_price) {

            // std::cout << "stock::get_max_profit_2() : [INFO] updating lowest buying price (" 
            //     << curr_lowest_buy_pos << " to " << i << ") : " 
            //     << min_buy_price << " to " << seq[i] << std::endl;

            min_buy_price = seq[i];
            curr_lowest_buy_pos = i;
        }

    }

    return max_profit;
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
        std::cerr << "stock::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        seq_str = arg_parser->optionValue(OPTION_SEQUENCE);
    }

    delete arg_parser;

    // extract the board to an array of int
    extract_sequence(seq_str, SEQUENCE_DELIMITER, &seq);
    std::cout << "stock::main() : [INFO] max profit :"
        << "\n\tORIGINAL : " << seq_str 
        << "\n\tPROFIT (1): " << get_max_profit(&seq) 
        << "\n\tPROFIT (2): " << get_max_profit_2(&seq) << std::endl;    

    exit(0);
}