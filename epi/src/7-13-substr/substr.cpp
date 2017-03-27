#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <thread>
#include <bitset>
#include <string>
#include <vector>
#include <deque>

#include "common.h"
#include "argvparser.h"

using namespace CommandLineProcessing;

#define OPTION_TEXT_FILE    (char *) "text-file"
#define OPTION_SUBSTRING    (char *) "substring"

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nfind the first occurrence of a substring \
(EPI problem 7.13, page 108)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_TEXT_FILE,
            ".txt file on which to search for the substring",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

        parser->defineOption(
            OPTION_SUBSTRING,
            "substring to search for",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

void get_text_str(std::string text_filename, std::string & text_str) {

    // open the .txt file, save it to a string. we used 
    // the following idiom:
    //  - ifstream to read the file, given it's file name
    std::ifstream in_file(text_filename);
    //  - use a stringstream to hold contents of ifstream and transform into string
    std::stringstream text_buffer;
    text_buffer << in_file.rdbuf();
    text_str = text_buffer.str();
}

int get_substr_rabin_karp(std::string text_filename, std::string substr) {

    // extract the text string from a .txt file
    std::string text_str; get_text_str(text_filename, text_str);

    // let's assume our alphabet goes from the space char ' ' to 
    // lowercase 'z', which gives us 90 diff. symbols
    const int k_base = ((int) 'z' - (int) ' ');
    std::cout << "substr::get_substr_rabin_karp() : [INFO] k_base = " << k_base << std::endl;
    // this will serve to hold the 'weight' of the first character of 
    // any substring. it will be useful for the 'rolling hash' below.
    int power_s = 1;
    // compute initial hash values for first n (n = substr.size())
    // chars of text
    int text_hash = 0, substr_hash = 0, n_collisions = 0;
    for (int i = 0; i < substr.size(); i++) {

        // iteratively calculate the 'weight' of the first character of 
        // any substring, for rolling hashes.

        // this is an interesting line. a ternary operator 
        // with an assignment as the conditional expression. 
        // if the first assignment results in something 
        // other than a 0, the value power_s * k_base is 
        // assigned to power_s instead. otherwise, the value 
        // 1 is assigned to power_s.
        power_s = i ? power_s * k_base : 1;

        // in the next 2 lines, we iteratively compute the hash values
        // for the n-sized substring and the first n chars of text_str.
        // 
        // let's analyze this computation in more detail. 
        // assuming k_base different symbols in the alphabet. at the 
        // same time, we're using an int to represent the hash code, 
        // which is in base 10. so the question is : how can we make 2 
        // diff. numbers in base k_base map to 2 diff. base 10 numbers?
        // one simple solution is to convert the sequence of base_k into 
        // base 10. collisions can occur because of int's precision (i.e. 
        // 2-4 byte or 16-32 bit).
        // another question is : how much does the nr. of collisions 
        // depends on:
        //  - the size of hashes, in bit? e.g. what if we used long?
        //  - the value of k_base?
        text_hash = text_hash * k_base + ((int) text_str[i] - (int) ' ');
        substr_hash = substr_hash * k_base + ((int) substr[i] - (int) ' ');
    }

    std::cout << "substr::get_substr_rabin_karp() : [INFO] substr_hash = " << substr_hash << std::endl;


    // check for a match between substr and {text_str[i - n, text_str[i - 1]}.
    // note that we start at i = n, so the first match check will be between 
    // substr and {text_str[0, text_str[n - 1]}, whose hash has been calculated 
    // above.
    for (int i = substr.size(); i < text_str.size(); i++) {

        // check if the rolling hashes of the text and sub strings match. 
        // if that happens, directly compare the strings, since a hash collision 
        // may have happened
        if (text_hash == substr_hash) {

            if (text_str.compare(i - substr.size(), substr.size(), substr) == 0) {

                std::cout << "substr::get_substr_rabin_karp() : [INFO] found substring match after " << n_collisions << " collisions" << std::endl;
                // return the index of text_str at which the match was found
                return (i - substr.size());

            } else {
                n_collisions++;
            }
        }

        // no match for the previous n chars. use the rolling hash to 
        // calculate text hash for next n chars (no need to calculate substr_hash, 
        // that one won't change). we do it in 2 steps:
        //  - remove the contribution of the first character
        text_hash = text_hash - power_s * ((int) text_str[i - substr.size()] - (int) ' ');
        //  - add contribution of text char at position i
        text_hash = (text_hash * k_base) + ((int) text_str[i] - (int) ' ');
    }

    // the last comparison between substr and {text_str[t - n, text_str[t - 1]}, 
    // with t being the size of text_str.
    if (text_hash == substr_hash) {

        if (text_str.compare(text_str.size() - substr.size(), substr.size(), substr) == 0) {

            std::cout << "substr::get_substr_rabin_karp() : [INFO] found substring match after " << n_collisions << " collisions" << std::endl;
            // return the index of text_str at which the match was found
            return (text_str.size() - substr.size());

        } else {

            n_collisions++;
        }
    }

    std::cout << "substr::get_substr_rabin_karp() : [INFO] no substring match found after " << n_collisions << " collisions" << std::endl;
    return -1;
}

int main (int argc, char **argv) {

    std::string text_filename, substr;

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
        std::cerr << "substr::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        text_filename = arg_parser->optionValue(OPTION_TEXT_FILE);
        substr = arg_parser->optionValue(OPTION_SUBSTRING);
    }

    delete arg_parser;

    int substr_pos = get_substr_rabin_karp(text_filename, substr);

    if (substr_pos < 0) {
        std::cout << "substr::main() : [INFO] no substring match found for " << substr << std::endl;
    } else {

        std::string text_str; get_text_str(text_filename, text_str);
        std::cout << "substr::main() : [INFO] substring match found for '" 
            << substr << "' at pos = " << substr_pos << " :" << std::endl;
        std::cout << "\t[SUBSTRING] : " << text_str.substr(substr_pos, substr.size()) << std::endl;
    }

    exit(0);
}