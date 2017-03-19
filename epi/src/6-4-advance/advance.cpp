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
#define OPTION_BOARD        (char *) "board"
#define BOARD_POS_DELIMITER (char) ','

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nadvance through an array \
(EPI problem 6.4, page 67)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_BOARD,
            "representation of board to check for completion. e.g. '--board 3,3,1,0,2,0,1'.",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

std::string to_intgr_str(std::vector<int> * intgr_ptr) {

    std::vector<int> intgr = *intgr_ptr;
    std::string intgr_str = "";

    for (int i = (intgr.size() - 1); i >= 0; i--)
        intgr_str = intgr_str + std::to_string(intgr[i]) + ",";

    return intgr_str;
}

void extract_board(
    const std::string & s, 
    char delim, 
    std::vector<int> * board_ptr) {

    std::vector<int> & board = *board_ptr;

    // we use a stringstream object for quick token separation, 
    // avoiding the use of the complicated strtok() function
    std::stringstream ss;
    // initialize ss from string s
    ss.str(s);
    // get each board position using getline()
    std::string board_pos;
    while (std::getline(ss, board_pos, delim)) {
        board.push_back(std::stoi(board_pos));
    }
}

// this is a terrible solution, O(n^n) time complexity (right?)
int is_solvable_rec(
    std::vector<int> * board_ptr, 
    std::vector<int> * solution_ptr,
    unsigned pos) {

    std::vector<int> board = *board_ptr;
    std::vector<int> & solution = *solution_ptr;
    int is_solvable = 0;
    
    // base cases : the last position or a 'dead end'
    if (pos == (board.size() - 1)) {
        solution.push_back(pos);
        return 1;
    }

    if (board[pos] == 0)
        return 0;

    // if not a dead end, advance to the positions 
    // which are reachable via the current position
    for (unsigned i = 1; i < (board[pos] + 1) && (pos + i) < (board.size()); i++) {
        is_solvable = is_solvable_rec(board_ptr, solution_ptr, pos + i);

        if (is_solvable > 0) {
            solution.push_back(pos);
            return 1;
        }
    }

    return 0;
}

// a better solution : O(n) time complexity
int is_solvable(std::vector<int> * board_ptr) {

    std::vector<int> board = *board_ptr;

    // let's update how far can we get as we iterate through the 
    // board. if we reach that point and fall short of the end of the board, 
    // we fail. if the furthest reachable position is equal or exceeds the 
    // end of the array, we succeed.
    int furthest_reachable_pos = 0, curr_reach_pos = 0;

    for (int curr_pos = 0; 
        curr_pos <= furthest_reachable_pos && (furthest_reachable_pos < (int) (board.size() - 1)); 
        curr_pos++) {

        curr_reach_pos = board[curr_pos] + curr_pos;
        if (curr_reach_pos > furthest_reachable_pos) {
            std::cout << "advance::is_solvable() : [INFO] updating furthest reach pos from " 
                << furthest_reachable_pos << " to " << curr_reach_pos << std::endl;    
            furthest_reachable_pos = curr_reach_pos;
        }
    }

    if (furthest_reachable_pos < (int) (board.size() - 1))
        return 0;
    else
        return 1;
}

int main (int argc, char **argv) {

    int res = 0;
    std::string board_str;
    std::vector<int> board, solution;

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
        std::cerr << "advance::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        board_str = arg_parser->optionValue(OPTION_BOARD);
    }

    delete arg_parser;

    // extract the board to an array of int
    extract_board(board_str, BOARD_POS_DELIMITER, &board);

    // is it solvable?
    res = is_solvable_rec(&board, &solution, 0);
    if (res)
        std::cout << "advance::main() : [INFO] solvable as : " 
            << "\n\tBOARD : " << board_str
            << "\n\tSOLUTION : " << to_intgr_str(&solution) << std::endl;
    else
        std::cout << "advance::main() : [INFO] unsolvable" << std::endl;

    std::cout << "advance::main() : [INFO] O(n) algorithm : " << ((is_solvable(&board) > 0) ? "solvable":"unsolvable") << std::endl;    

    exit(0);
}