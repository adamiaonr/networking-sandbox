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

#define OPTION_SENTENCE     (char *) "sentence"

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ntest for palindromicity & reverse words in sentence\
(EPI problems 7.5 and 7.6, pages 99 and 100)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_SENTENCE,
            "sentence to test for palindromicity and for word-reversal",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

bool is_char(char c) {

    int c_int = (int) c;
    if ((c_int > 64 && c_int < 91) || (c_int > 96 && c_int < 123))
        return true;
    else
        return false;
}

bool is_palindrome(std::string sentence) {

    int left_ptr = 0, right_ptr = sentence.size() - 1;

    while (left_ptr < right_ptr) {

        while ((!is_char(sentence[left_ptr++])) && left_ptr < right_ptr)
        while ((!is_char(sentence[right_ptr--])) && left_ptr < right_ptr)

        if (sentence[left_ptr++] != sentence[right_ptr--])
            return false;
    }

    return true;
}

void reverse(std::string & sentence, int start, int end) {

    while (start < end)
        std::swap(sentence[start++], sentence[end--]);
}

void reverse_words(std::string & sentence) {

    // swap all chars in string
    int left_ptr = 0, right_ptr = sentence.size() - 1;
    reverse(sentence, left_ptr, right_ptr);

    // reverse each word (except the last)
    left_ptr = 0;
    for (unsigned i = 0; i < sentence.size(); i++) {

        if (sentence[i] == ' ') {
            reverse(sentence, left_ptr, i - 1);
            left_ptr = i + 1;
        }
    }

    // reverse last word
    reverse(sentence, left_ptr, sentence.size() - 1);
}

int main (int argc, char **argv) {

    std::string sentence;

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
        std::cerr << "palindromicity::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        sentence = arg_parser->optionValue(OPTION_SENTENCE);
    }

    delete arg_parser;

    std::cout << "palindromicity::main() : [INFO] replace & remove:"
        << "\n\tORIGINAL SENTENCE: " << sentence
        << "\n\tPALINDROME?: " << (is_palindrome(sentence) ? "TRUE" : "FALSE") << std::endl;

    reverse_words(sentence);
    std::cout << "\tWORD REVERSED: " << sentence << std::endl;

    exit(0);
}