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

#define OPTION_SUBLIST_1        (char *) "sublist-1"
#define OPTION_SUBLIST_2        (char *) "sublist-2"
#define OPTION_OVERLAP_LIST     (char *) "overlap-list"
#define SEQUENCE_DELIMITER      (char) ','

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ntest for overlapping lists (no cycles) \
(EPI problem 8.4, page 116)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_SUBLIST_1,
            "1st non-overlapping sublist of integers",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_SUBLIST_2,
            "2nd non-overlapping sublist of integers",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_OVERLAP_LIST,
            "overlapping sublist of integers, which will be appended to the end \
of both sublists.",
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

std::shared_ptr<List_Node<int>> create_list(std::string seq_str) {

    std::vector<int> seq = extract_sequence(seq_str, SEQUENCE_DELIMITER);

    // a dummy head for the new linked list to return
    std::shared_ptr<List_Node<int>> dummy_head(new List_Node<int>);
    auto curr_node = dummy_head;

    for (unsigned i = 0; i < seq.size(); i++) {
        // create new node, give it the value of seq[i] and 
        // insert it in the list, after curr_node
        std::shared_ptr<List_Node<int>> new_node(new List_Node<int>);
        new_node->data = seq[i];
        insert_after(curr_node, new_node);
        // update curr_node to point to the node just inserted
        curr_node = new_node;
    }

    return dummy_head;
}

std::string to_str(std::shared_ptr<List_Node<int>> list) {

    std::string list_str;
    auto curr_node = list->next;

    while(curr_node) {
        list_str += std::to_string(curr_node->data) + ", ";
        curr_node = curr_node->next;
    }

    return list_str;
}

// this problem has easy 'brute force' solutions:
//  - hash table keeps visited nodes so far, iterate over each list at a time, 
//    stop when a node already in the table is found. O(n) time, O(n) space.
//  - iterate slowly over list 1, fast over list 2, in a nested loop. if 
//    the nodes are equal, stop. O(n^2) time, O(1) space.
// 
// the key observation to solve this efficiently, i.e. O(n) time, is that 
// if 2 lists overlap at any point, then they will keep overlapping till the 
// tail, meaning both will have the same tail node. as such, we can 
// measure the length of each list, and use that to iterate over the list 
// in tandem, starting from the head of the shortest list.
std::shared_ptr<List_Node<int>> get_overlapping_node(
    std::shared_ptr<List_Node<int>> list_1,
    std::shared_ptr<List_Node<int>> list_2) {

    // determine sizes of list 1 and 2
    int list_1_size = 0, list_2_size = 0;
    auto list_iter = list_1;

    while(list_iter) { list_iter = list_iter->next; list_1_size++; }
    list_iter = list_2;
    while(list_iter) { list_iter = list_iter->next; list_2_size++; }

    // the common node - if existent - must exist after the first 
    // node of the shortest list. so, advance the pointer of the 
    // largest list by largest_size - smallest_size positions.
    if (list_1_size > list_2_size)
        while (list_1_size-- > list_2_size) list_1 = list_1->next;
    else 
        while (list_1_size < list_2_size--) list_2 = list_2->next;

    // now, iterate through both lists in tandem until finding a 
    // common node
    while ((list_1 && list_2) && (list_1 != list_2)) {
        list_1 = list_1->next;
        list_2 = list_2->next;
    }

    return list_1;
}

void append_lists(
    std::shared_ptr<List_Node<int>> before,
    std::shared_ptr<List_Node<int>> after) {

    auto list_iter = before;
    while(list_iter->next) { list_iter = list_iter->next; }

    list_iter->next = after->next;
}

int main (int argc, char **argv) {

    std::string sublist_1_str, sublist_2_str, overlapping_list_str;
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
        std::cerr << "overlap::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        sublist_1_str = arg_parser->optionValue(OPTION_SUBLIST_1);
        sublist_2_str = arg_parser->optionValue(OPTION_SUBLIST_2);
        overlapping_list_str = arg_parser->optionValue(OPTION_OVERLAP_LIST);
    }

    delete arg_parser;

    auto sublist_1 = create_list(sublist_1_str);
    auto sublist_2 = create_list(sublist_2_str);
    auto overlapping_list = create_list(overlapping_list_str);
    // create the overlapping list
    append_lists(sublist_1, overlapping_list);
    append_lists(sublist_2, overlapping_list);

    std::cout << "overlap::main() : [INFO] list info : "
        << "\n\t[LIST_1] : " << to_str(sublist_1)
        << "\n\t[LIST_2] : " << to_str(sublist_2);

    // find the overlapping point
    auto overlapping_node = get_overlapping_node(sublist_1, sublist_2);
    std::cout << "\n\t[OVERLAPPING NODE] : ";

    if (overlapping_node == NULL) {

        std::cout << "NONE" << std::endl;

    } else {

        std::cout << overlapping_node->data << std::endl;
    }

    exit(0);
}