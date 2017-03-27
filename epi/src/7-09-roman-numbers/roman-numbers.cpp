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

#define OPTION_ROMAN_NUMBER     (char *) "roman-number"
#define OPTION_DECIMAL_NUMBER   (char *) "decimal-number"

// roman number values
std::unordered_map<char, int> ROMAN_NUMBERS = {
    {'I', 1},
    {'V', 5},
    {'X', 10},
    {'L', 50},
    {'C', 100},
    {'D', 500},
    {'M', 1000}};

// roman exceptions
std::unordered_map<std::string, int> ROMAN_EXCEPTIONS = {
    {"IV", 4},
    {"IX", 9},
    {"XL", 40},
    {"XC", 90},
    {"CD", 400},
    {"CM", 900}};

std::unordered_map<int, char> INVERSE_ROMAN_NUMBERS = {
    {1,    'I'},
    {5,    'V'},
    {10,   'X'},
    {50,   'L'},
    {100,  'C'},
    {500,  'D'},
    {1000, 'M'}};

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nconvert from roman to decimal (and variants) \
(EPI problem 7.9, page 103)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_ROMAN_NUMBER,
            "roman number to be converted",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_DECIMAL_NUMBER,
            "decimal number to be converted to roman",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

bool is_valid_roman(std::string roman_number) {

    bool prev_exception = false;
    for (int i = roman_number.size() - 2; i >= 0; i--) {

        if (ROMAN_NUMBERS[roman_number[i]] < ROMAN_NUMBERS[roman_number[i + 1]]) {

            if (prev_exception) {
                // back-to-back exceptions aren't allowed
                return false;
            } else if (ROMAN_EXCEPTIONS.find(roman_number.substr(i, i + 1)) == ROMAN_EXCEPTIONS.end()) {
                // not a valid exception
                return false;
            } else {
                // valid exception, set prev_expression to TRUE
                prev_exception = true;
            }

        } else {
            prev_exception = false;
        }
    }

    return true;
}

int from_roman(std::string roman_number) {

    int decimal_number = ROMAN_NUMBERS[roman_number.back()];
    for (int i = roman_number.size() - 2; i >= 0; i--) {

        // check if i and i + 1 form an exception
        if (ROMAN_NUMBERS[roman_number[i]] < ROMAN_NUMBERS[roman_number[i + 1]]) {

            decimal_number -= ROMAN_NUMBERS[roman_number[i]];

        } else {

            decimal_number += ROMAN_NUMBERS[roman_number[i]];
        }
    }

    return decimal_number;
}

std::string to_roman(int decimal_number) {

    std::string roman_number;
    int power_10 = 1;
    
    while (decimal_number > 0) {

        int digit = (decimal_number % 10);

        if ((digit * power_10) < (4 * power_10)) {

            while(digit-- > 0) roman_number.push_back(INVERSE_ROMAN_NUMBERS[power_10]);

        } else if ((digit * power_10) < (5 * power_10)) {
            
            roman_number.push_back(INVERSE_ROMAN_NUMBERS[5 * power_10]);
            roman_number.push_back(INVERSE_ROMAN_NUMBERS[power_10]);

        } else if ((digit * power_10) < (9 * power_10)) {
            
            while (digit > 5) { roman_number.push_back(INVERSE_ROMAN_NUMBERS[power_10]); digit--; }
            roman_number.push_back(INVERSE_ROMAN_NUMBERS[5 * power_10]);

        } else {

            roman_number.push_back(INVERSE_ROMAN_NUMBERS[10 * power_10]);
            roman_number.push_back(INVERSE_ROMAN_NUMBERS[power_10]);
        }

        decimal_number = decimal_number / 10;
        power_10 *= 10;
    }

    return {roman_number.rbegin(), roman_number.rend()};
}

int main (int argc, char **argv) {

    std::string roman_number;
    int decimal_number = 0;

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
        std::cerr << "roman-numbers::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        roman_number = arg_parser->optionValue(OPTION_ROMAN_NUMBER);
        decimal_number = std::stoi(arg_parser->optionValue(OPTION_DECIMAL_NUMBER));
    }

    delete arg_parser;

    std::cout << "roman-numbers::main() : [INFO] " << roman_number << " = " << from_roman(roman_number) << std::endl;
    std::cout << "roman-numbers::main() : [INFO] is " << roman_number << " valid? " << (is_valid_roman(roman_number) ? "yes" : "no") << std::endl;
    std::cout << "roman-numbers::main() : [INFO] " << decimal_number << " = " << to_roman(decimal_number) << std::endl;

    exit(0);
}