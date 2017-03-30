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
#include "linked-list.h"

using namespace CommandLineProcessing;

#define OPTION_LIST             (char *) "list"
#define OPTION_CYCLE_START      (char *) "cycle-start"
#define SEQUENCE_DELIMITER      (char) ','

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ntest for cyclicity \
(EPI problem 8.3, page 115)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_LIST,
            "list of integers",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_CYCLE_START,
            "index at which cycle starts (for testing purposes)",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

std::vector<int> extract_sequence(
    const std::string & s, 
    char delim) {

    std::vector<int> seq;

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

    return seq;
}

std::shared_ptr<List_Node<int>> create_list(
    std::string seq_str,
    int cycle_start) {

    std::vector<int> seq = extract_sequence(seq_str, SEQUENCE_DELIMITER);

    // a dummy head for the new linked list to return
    std::shared_ptr<List_Node<int>> dummy_head(new List_Node<int>);
    std::shared_ptr<List_Node<int>> cycle_start_node = NULL;
    auto curr_node = dummy_head;

    for (int i = 0; i < seq.size(); i++) {
        // create new node, give it the value of seq[i] and 
        // insert it in the list, after curr_node
        std::shared_ptr<List_Node<int>> new_node(new List_Node<int>);
        new_node->data = seq[i];
        insert_after(curr_node, new_node);
        // update curr_node to point to the node just inserted
        curr_node = new_node;

        if (i == cycle_start) cycle_start_node = curr_node;
    }

    // close the cycle (if applicable)
    curr_node->next = cycle_start_node;

    return dummy_head;
}

std::string to_str(
    std::shared_ptr<List_Node<int>> list,
    int cycle_start) {

    std::string list_str;
    std::shared_ptr<List_Node<int>> cycle_start_node = NULL;
    auto curr_node = list->next;

    int i = 0;
    while(curr_node->next != cycle_start_node) {
        list_str += std::to_string(curr_node->data) + ", ";
        curr_node = curr_node->next;

        if (i++ == cycle_start) cycle_start_node = curr_node;
    }

    return list_str;
}

// say we have the following list w/ 6 nodes and 1 cycle:
// 0    1       2       3      4      5       6
// L -> (23) -> (53) -> (1) -> (3) -> (16) -> (112)
//                       ^                      :
//                       :----------------------:
//
// how do we even *detect* the existence of the cycle? here's an idea for an 
// algorithm:
//  - have 2 pointers : back_node and list_iter.
//  - say we're testing if node 2 points back to a previous element in the 
//    list. as such, we set back_node to node 2.
//  - we set list_iter to point to the START of the list, i.e. list_iter = 
//    list->next (avoiding the dummy head)
//  - if back_node->next != list_iter, we set list_iter = list_iter->next WHILE 
//    list_iter->next != back_node
//  - if at any point during this iteration, back_node->next == list_iter, 
//    we can return back_node->next as the start of the cycle.
//  - we should also stop if back_node->next == NULL
//
// what is the complexity of this? O(n^2) time, O(1) space. apparently there's 
// an O(n) solution though...
std::shared_ptr<List_Node<int>> cyclicity_test(
    std::shared_ptr<List_Node<int>> list) {

    // setting the pointers to list->next skips the dummy head
    auto back_node = list->next;
    // keep advancing through the list until back_node points to NULL. in that 
    // case, we would know we don't have a cycle in the list.
    while(back_node->next) {

        // cycle consists of back_node pointing to itself...
        if (back_node->next == back_node) break;

        auto list_iter = list->next;
        while(list_iter != back_node) {

            if (back_node->next == list_iter) {
                return back_node->next;
            } else {
                list_iter = list_iter->next;
            }
        }

        back_node = back_node->next;
    }

    return back_node->next;
}

int main (int argc, char **argv) {

    std::string list_str;
    int cycle_start;

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
        std::cerr << "cyclicity::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        // list
        list_str = arg_parser->optionValue(OPTION_LIST);
        cycle_start = std::stoi(arg_parser->optionValue(OPTION_CYCLE_START));
    }

    delete arg_parser;

    auto list = create_list(list_str, cycle_start);
    std::cout << "cyclicity::main() : [INFO] reverse sublist : "
        << "\n\t[LIST] : " << to_str(list, cycle_start)
        << "\n\t[CYCLE START] : " << cycle_start;

    // reverse the sublist, in place
    auto cycle_start_node = cyclicity_test(list);
    std::cout << "\n\t[DETECTED CYCLE] : ";

    if (cycle_start_node == NULL) {

        std::cout << "NONE";

    } else {

        list = cycle_start_node;
     
        while(list->next != cycle_start_node) {
            std::cout << list->data << ", ";
            list = list->next;
        }
        std::cout << list->data << std::endl;
    }

    exit(0);
}