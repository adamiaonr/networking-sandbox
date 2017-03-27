#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

#define OPTION_OPERANDS     (char *) "operands"
#define OPERAND_DELIMITER   (char) '^'

#define RECURSIVE       0x00
#define TAIL_RECURSIVE  0x01
#define NON_RECURSIVE   0x02

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ngiven x (double) and \
y (integer), compute x^y (EPI problem \
5.7, page 53)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_OPERANDS,
            "x^y (separated by '^', e.g. '2.56^3')",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

// taken from http://stackoverflow.com/a/236803/6660861
void split_str(
    const std::string & s, 
    char delim, 
    std::vector<std::string> & tokens) {
    
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        tokens.push_back(item);
    }
}

// my own solution, using normal recursion
double power_rec(double base, int exponent, int & n_iter) {

    double result = 0.0;
    n_iter++;

    if ((exponent >> 1) > 1) {
        // std::cout << "power::power_rec() : [INFO] at " << exponent << " going for " << (exponent >> 1) << std::endl;
        result = power_rec(base, (exponent >> 1), n_iter);

    } else {
        // base case
        // std::cout << "power::power_rec() : [INFO] base case " << exponent << std::endl;
        result = base;
    }

    if (exponent & 1) {
        // if exponent is not even, add +1 base multiplication
        return (result * result * base);
    } else {
        return (result * result);
    }
}

// understanding tail and normal recursion
double power_tail_rec(double base, int exponent, double result, int & n_iter) {

    if (exponent) {

        // extra +1 base multiplication if exponent is odd
        if (exponent & 1) {
            result *= base;
        }

        // we square the result at each pass
        base *= base;

        return power_tail_rec(base, (exponent >> 1), result, n_iter);

    } else {

        return result;
    }
}

// as shown in the book
double power_no_rec(double base, int exponent, int & n_iter) {

    double result = 1.0;

    while (exponent) {

        n_iter++;
        if (exponent & 1) {
            result *= base;
        }

        base = base * base;

        std::cout << "power::power_no_rec() : [INFO] exponent = " << exponent 
            << ", result = " << result
            << ", base = " << base << std::endl;

        exponent = exponent >> 1;
    }

    return result;
}

double power(double base, int exponent) {

    int n_iter = 0;
    double result = 1.0;

    result = power_rec(base, exponent, n_iter);
    std::cout << "power::power() : [INFO] (normal recursion) "
        << base << "^" << exponent << " = " << result 
        << " (" << pow(base, double(exponent)) << ")" << std::endl;
    std::cout << "power::power() : [INFO] (normal recursion) n_iter = " 
        << n_iter << std::endl;

    n_iter = 0;
    // important initialization for base recursion
    result = 1.0;
    result = power_tail_rec(base, exponent, result, n_iter);
    std::cout << "power::power() : [INFO] (tail recursion) "
        << base << "^" << exponent << " = " << result 
        << " (" << pow(base, double(exponent)) << ")" << std::endl;
    std::cout << "power::power() : [INFO] (tail recursion) n_iter = " 
        << n_iter << std::endl;

    n_iter = 0;
    result = power_no_rec(base, exponent, n_iter);
    std::cout << "power::power() : [INFO] (no recursion) "
        << base << "^" << exponent << " = " << result 
        << " (" << pow(base, double(exponent)) << ")" << std::endl;
    std::cout << "power::power() : [INFO] (no recursion) n_iter = " 
        << n_iter << std::endl;

    return result;
}

int main (int argc, char **argv) {

    ArgvParser * arg_parser = create_argv_parser();
    double base = 0.0, result = 0.0;
    int exponent = 0;
    // parse() takes the arguments to main() and parses them according to 
    // ArgvParser rules
    int parse_result = arg_parser->parse(argc, argv);

    if (parse_result == ArgvParser::ParserHelpRequested) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        delete arg_parser;
        exit(-1);

    } else if (parse_result != ArgvParser::NoParserError) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        std::cerr << "power::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        // get the input operands (passed as a string '<op1>x<op2>'')
        std::string ops_str = arg_parser->optionValue(OPTION_OPERANDS);
        // split ops_str into 2 operand strings
        std::vector<std::string> tokens;
        split_str(ops_str, OPERAND_DELIMITER, tokens);
        // extract the 2 operands
        if (tokens.size() != 2) {
            std::cerr << "power::main() : [ERROR] only 2 operands please" << std::endl;
            delete arg_parser;
            exit(-1);
        }

        base = (double) std::stod(tokens[0]);
        exponent = (int) std::stoi(tokens[1]);
    }

    delete arg_parser;

    if (exponent < 0) {
        result = power((1.0 / base), -exponent);
    } else {
        result = power(base, exponent);
    }

    std::cout << "power::main() : [INFO] "
        << base << "^" << exponent << " = " << result 
        << " (" << pow(base, double(exponent)) << ")" << std::endl;

    exit(0);
}