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
#define OPTION_SUBLIST_START    (char *) "sublist-start"
#define OPTION_SUBLIST_END      (char *) "sublist-end"
#define SEQUENCE_DELIMITER      (char) ','

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nreverse a single sublist \
(EPI problem 8.2, page 113)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_LIST,
            "list of integers",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_SUBLIST_START,
            "start index of sublist to reverse",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_SUBLIST_END,
            "end index of sublist to reverse",
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

void reverse_sublist(
    std::shared_ptr<List_Node<int>> list, 
    int sublist_start, int sublist_end) {

    std::shared_ptr<List_Node<int>> tail = list, head, before_head, prev, next;

    // iterate till the start of the sublist
    // the +1 accounts for the dummy head (or sentinel)
    for (int i = 0; i < (sublist_start - 1); i++) {
        tail = tail->next;
    }

    // at this point we're at the node just before the start of the sublist. 
    // as such, we can record the following info : 
    //  - the node before the head of the reversed sublist 
    //  - the tail of the sublist, which will point to the continuation of 
    //    the unreversed list
    before_head = tail;
    tail = tail->next;

    // we now look for the head of the list (at position sublist_end)
    head = tail;
    prev = tail;
    for (int i = sublist_start; i < (sublist_end + 1) && head; i++) {

        // save the (still) next node, so that we can keep iterating 
        // over the original list
        next = head->next;
        // reverse the direction of the next pointer for each node within
        // the sublist
        head->next = prev;
        // update prev to the current node
        prev = head;

        // continue iterating
        head = next;
    }

    before_head->next = prev;
    tail->next = next;
}

int main (int argc, char **argv) {

    std::string list_str;
    int sublist_start, sublist_end;

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
        std::cerr << "reverse-sublist::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        // list
        list_str = arg_parser->optionValue(OPTION_LIST);
        // sublist indeces
        sublist_start = std::stoi(arg_parser->optionValue(OPTION_SUBLIST_START));
        sublist_end = std::stoi(arg_parser->optionValue(OPTION_SUBLIST_END));
    }

    delete arg_parser;

    auto list = create_list(list_str);

    std::cout << "reverse-sublist::main() : [INFO] reverse sublist : "
        << "\n\t[LIST] : " << to_str(list)
        << "\n\t[START | END] : " << sublist_start << " | " << sublist_end;

    // reverse the sublist, in place
    reverse_sublist(list, sublist_start, sublist_end);
    std::cout << "\n\t[W/ REVERSED SUBLIST] : " << to_str(list) << std::endl;

    exit(0);
}