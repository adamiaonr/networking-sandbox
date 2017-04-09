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
#define OPTION_K_SHIFT          (char *) "k-shift"
#define SEQUENCE_DELIMITER      (char) ','

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nimplement cyclic right shift for \
singly linked lists (EPI problem 8.9, pages 122)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_LIST,
            "(sorted) list of integers, on which to apply k cyclic right shift",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_K_SHIFT,
            "value of k shift",
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

void cyclic_right_shift(std::shared_ptr<List_Node<int>> list, int k) {

    // find the tail node, and list size. why? since k may be k >> list_size, 
    // the time complexity may be O(k) instead of O(n). to avoid this, we 
    // observe that a rotation of k is equivalent to a rotation 
    // of k mod list_size (that's arithmetic mod n for you). as such, 
    // we calculate the list size first, then adjust k.
    auto tail = list->next;
    int list_size = 1;
    while (tail->next) { tail = tail->next; ++list_size; }
    k = k % list_size;

    // find the k-th to last node, which will be the new tail, using the same 
    // technique as problem 8.7 (k-th last node)
    auto new_tail = list->next; tail = list->next;
    while (tail->next && (k-- > 0)) { tail = tail->next; }
    while (tail->next) { new_tail = new_tail->next; tail = tail->next; }

    // finally, re-arrange the pointers
    tail->next = list->next;
    list->next = new_tail->next;
    new_tail->next = NULL;
}

int main (int argc, char **argv) {

    std::shared_ptr<List_Node<int>> list;
    int k = 0;

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
        std::cerr << "cyclic-right-shift::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        list = create_list(arg_parser->optionValue(OPTION_LIST));
        k = std::stoi(arg_parser->optionValue(OPTION_K_SHIFT));
    }

    delete arg_parser;

    std::cout << "cyclic-right-shift::main() : [INFO] list info : "
        << "\n\t[LIST] : " << to_str(list)
        << "\n\t[K RIGHT SHIFT] : " << k << std::endl;
    cyclic_right_shift(list, k);
    std::cout << "\t[SHIFTED LIST] : " << to_str(list) << std::endl;

    exit(0);
}