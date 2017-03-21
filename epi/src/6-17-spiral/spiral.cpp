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

#define OPTION_MATRIX_SIZE  (char *) "matrix-size"
#define SEQUENCE_DELIMITER  (char) ','

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ncompute spiral ordering of 2D array \
(EPI problem 6.17, pages 86)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_MATRIX_SIZE,
            "size of matrix to generate (N x N)",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

std::string to_matrix_str(std::vector<std::vector<int>> matrix) {

    std::string matrix_str = "";
    std::cout << "spiral::to_matrix_str() : [INFO] matrix size : " << matrix.size() << std::endl;

    for (unsigned r = 0; r < matrix.size(); r++) {
        for(unsigned c = 0; c < matrix[r].size(); c++)
            matrix_str = matrix_str + std::to_string(matrix[r][c]) + " ";

        matrix_str += "\n";
    }

    return matrix_str;
}

std::string to_intgr_str(std::vector<int> * intgr_ptr) {

    std::vector<int> intgr = *intgr_ptr;
    std::string intgr_str = "";

    for (unsigned i = 0; i < intgr.size(); i++)
        intgr_str = intgr_str + std::to_string(intgr[i]) + " ";

    return intgr_str;
}

std::vector<std::vector<int>> gen_square_matrix(int size) {

    std::vector<std::vector<int>> matrix;

    for (int r = 0; r < size; r++) {
        matrix.push_back(std::vector<int>());
        for(int c = 0; c < size; c++) {
            matrix[r].push_back((c + 1) + (r * size));
        }
    }

    return matrix;
}

// this is an O(n^2) time, O(n^2) space solution, n being the size of the 2D 
// array. some observations used in this solution:
//  - a spiral scans through all n x n elements of the matrix. so one possible 
//    stopping condition is when the number of visited elements reaches n x n.
//  - a spiral scan switches over rows and columns. the 
//    trick is in understanding when to make the switch, and the direction of 
//    the gathering (e.g. to the right / left over a row, down / up over a column).
//  - basically, every time the scan gets to one of the 'corners' of the spiral, 
//    the limits for the 'left', 'right', 'up' and 'down' scans increments / decrements by 1.
//  
std::vector<int> get_spiral_ordering(int size) {

    std::vector<std::vector<int>> matrix = gen_square_matrix(size);
    std::cout << "spiral::get_spiral_ordering() : [INFO] matrix :\n"
        << to_matrix_str(matrix) << std::endl;

    std::vector<int> spiral_order;

    int left_limit = 0, right_limit = size - 1, top_limit = 0, bottom_limit = size - 1;
    int nr_elements = (size * size);

    // switch between right, down, left and up scans, until all elements 
    // are scanned
    while (spiral_order.size() < (nr_elements - 1)) {

        for (int c = left_limit; c < right_limit; c++)
            spiral_order.push_back(matrix[top_limit][c]);

        for (int r = top_limit; r < bottom_limit; r++)
            spiral_order.push_back(matrix[r][right_limit]);

        for (int c = right_limit; c > left_limit; c--)
            spiral_order.push_back(matrix[bottom_limit][c]);

        for (int r = bottom_limit; r > top_limit; r--)
            spiral_order.push_back(matrix[r][left_limit]);

        // update limits
        left_limit++;
        top_limit++;
        right_limit--;
        bottom_limit--;

        std::cout << "spiral::get_spiral_ordering() : [INFO] var state :"
            << "\n\tL: " << left_limit
            << "\n\tR: " << right_limit
            << "\n\tT: " << top_limit
            << "\n\tB: " << bottom_limit
            << "\n\tSPIRAL ORDER SIZE: " << spiral_order.size() << std::endl;
    }

    // corner case for odd sizes
    if (spiral_order.size() < nr_elements)
        spiral_order.push_back(matrix[top_limit][left_limit]);

    return spiral_order;
}

int main (int argc, char **argv) {

    int matrix_size;

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
        std::cerr << "spiral::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        matrix_size = std::stoi(arg_parser->optionValue(OPTION_MATRIX_SIZE));
    }

    delete arg_parser;

    std::vector<int> spiral_order = get_spiral_ordering(matrix_size);

    std::cout << "spiral::main() : [INFO] spiral ordering :"
        << "\n\tSPIRAL ORDERING: " << to_intgr_str(&spiral_order) << std::endl;

    exit(0);
}