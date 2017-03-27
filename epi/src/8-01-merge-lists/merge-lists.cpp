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

#define OPTION_SEQUENCE_1    (char *) "seq-1"
#define OPTION_SEQUENCE_2    (char *) "seq-2"

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\nmerge two sorted lists \
(EPI problem 8.1, page 112)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_SEQUENCE_1,
            "1st sequence of non-decreasing integers",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

        parser->defineOption(
            OPTION_SEQUENCE_2,
            "2nd sequence of non-decreasing integers",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    return parser;
}

std::shared_ptr<List_Node<int>> create_list(std::string seq) {

    // a dummy head for the new linked list to return
    std::shared_ptr<List_Node<int>> dummy_head(new List_Node<int>);
    auto curr_node = dummy_head;

    for (unsigned i = 0; i < seq.size(); i++) {
        // create new node, give it the value of seq[i] and 
        // insert it in the list, after curr_node
        std::shared_ptr<List_Node<int>> new_node(new List_Node<int>);
        new_node->data = (int) seq[i] - (int) '0';
        insert_after(curr_node, new_node);
        // update curr_node to point to the node just inserted
        curr_node = new_node;
    }

    return dummy_head;
}

std::string to_str(std::shared_ptr<List_Node<int>> list) {

    std::string list_str;
    auto curr_node = list;

    while(curr_node) {
        list_str += std::to_string(curr_node->data) + ", ";
        curr_node = curr_node->next;
    }

    return list_str;
}

// given 2 lists, in non-decreasing order, merge them in order to 
// form a single list, also in non-decreasing order.
//
// time complexity  : O(n + m), n = size(list_1), m = size(list_2)
// space complexity : O(1), since we simply re-assign the pointers 
//                    of the lists
std::shared_ptr<List_Node<int>> merge_lists(
    std::shared_ptr<List_Node<int>> list_1, 
    std::shared_ptr<List_Node<int>> list_2) {

    // create a dummy head for the new list. 
    // why is there a need for a new list head? the heads for list_1 
    // and list_2 will be useful as we remove elements from the 
    // heads of each into the tail of the merged list. 
    std::shared_ptr<List_Node<int>> dummy_head(new List_Node<int>);
    auto tail = dummy_head;

    // here's what can happen:
    //  1) while we still have nodes on both list_1 and list_2, we 
    //     keep choosing whichever is smallest
    //  2) once one of the lists is depleted, we add the elements from 
    //     the other list
    while (list_1 && list_2) {

        if (list_1->data < list_2->data) {

            tail->next = list_1;
            list_1 = list_1->next;

        } else {

            tail->next = list_2;
            list_2 = list_2->next;
        }

        tail = tail->next;
    }

    if (list_1)
        tail->next = list_1;
    else
        tail->next = list_2;

    return dummy_head;
}

int main (int argc, char **argv) {

    std::string seq_1, seq_2;

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
        std::cerr << "merge-lists::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        seq_1 = arg_parser->optionValue(OPTION_SEQUENCE_1);
        seq_2 = arg_parser->optionValue(OPTION_SEQUENCE_2);
    }

    delete arg_parser;

    auto list_1 = create_list(seq_1);
    auto list_2 = create_list(seq_2);

    std::cout << "merge-lists::main() : [INFO] merged linked list : "
        << "\n\t[LIST 1] : " << to_str(list_1)
        << "\n\t[LIST 2] : " << to_str(list_2);

    // notice how we skip the heads of each list by using list_x->next
    auto merged_list = merge_lists(list_1->next, list_2->next);
    std::cout << "\n\t[MERGED LIST] : ";

    merged_list = merged_list->next;
    while (merged_list) {

        std::cout << merged_list->data << ", ";
        merged_list = merged_list->next;
    }
    std::cout << std::endl;

    exit(0);
}