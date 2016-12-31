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
#define OPERAND_DELIMITER   (char) 'x'
#define UINT_SIZE           (sizeof(uint32_t) * 8)

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nmultiply 2 non-negative integers w/ no arithmetic ops (EPI problem 5.5, page 51)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_OPERANDS,
            "operands (separated by 'x', e.g. '2x5')",
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

int main (int argc, char **argv) {

    ArgvParser * arg_parser = create_argv_parser();
    uint32_t op1 = 0, op2 = 0, result = 0;
    // parse() takes the arguments to main() and parses them according to 
    // ArgvParser rules
    int parse_result = arg_parser->parse(argc, argv);

    if (parse_result == ArgvParser::ParserHelpRequested) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        delete arg_parser;
        exit(-1);

    } else if (parse_result != ArgvParser::NoParserError) {

        std::cerr << arg_parser->parseErrorDescription(parse_result).c_str() << std::endl;
        std::cerr << "multiply::main() : [ERROR] use option -h for help." << std::endl;

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
            std::cerr << "multiply::main() : [ERROR] only 2 operands please" << std::endl;
            delete arg_parser;
            exit(-1);
        }

        op1 = (uint32_t) std::stoi(tokens[0]);
        op2 = (uint32_t) std::stoi(tokens[1]);
    }

    delete arg_parser;

    // this will keep the rightmost bit set (rbs) of op2
    uint32_t op1_aux = op1, op2_aux = op2, carry = 0, prev_carry = 0;
    // keep doing this while op2 has set bits
    while (op2) {

        // if the k-th bit (out of n) of op2 is set, we add 2^k * op1 to the 
        // result
        if (op2 & 1) {

            // check if there will be any carry bits
            carry = result & op1;

            // std::cout << "multiply::main() : [INFO] multiply elements:" 
            //     << "\n\t[OP1] " << op1 << "\t\t(" << std::bitset<UINT_SIZE>(op1) << ")"
            //     << "\n\t[OP2] " << "\t\t(" << std::bitset<UINT_SIZE>(op2) << ")"
            //     << "\n\t[PREV_RESULT] " << "\t(" << std::bitset<UINT_SIZE>(result) << ")" 
            //     << "\n\t[CARRY]  " << "\t(" << std::bitset<UINT_SIZE>(carry) << ")" 
            //     << std::endl;

            // 'add' the shifted op1 to the result by XORing (not accounting w/ 
            // carry bits yet)
            result ^= op1;

            // std::cout << "\t[NEW_RESULT] " << "\t(" << std::bitset<UINT_SIZE>(result) << ")" << std::endl;

            // keep adding the carry to result (XORing) until there are no more 
            // carry bits
            while (carry) {

                prev_carry = carry;
                // check if adding (XORing) the carry to result will result in any 
                // more carry bits
                carry = result & (prev_carry << 1);
                // add (XOR) the prev_carry bits, shifted left by 1, to the result
                result ^= (prev_carry << 1);

                // std::cout << "\t[<<CARRY] " << "\t(" << std::bitset<UINT_SIZE>((prev_carry << 1)) << ")" 
                //     << "\n\t[NEW_RESULT] " << "\t(" << std::bitset<UINT_SIZE>(result) << ")" 
                //     << "\n\t[NEW_CARRY]  " << "\t(" << std::bitset<UINT_SIZE>(carry) << ")" 
                //     << std::endl;
            }
        }

        // right shift op2 to get the k-th bit of op2 
        op2 >>= 1;
        // left shift op1 to get 2^k * op1
        op1 <<= 1;
    }

    std::cout << "multiply::main() : [INFO] " << op1_aux << " x " << op2_aux 
        << " = " << result << " (" << (op1_aux * op2_aux) << ")" << std::endl;

    exit(0);
}