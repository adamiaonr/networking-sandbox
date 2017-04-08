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
#define OPTION_K_LAST           (char *) "k-last"
#define SEQUENCE_DELIMITER      (char) ','

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ndelete a node from a singly liked list \
and remove the k-th last node from a list (EPI problems 8.6 and 8.7, pages 119 \
and 120)\n\n\nby adamiaonr@gmail.com");
    parser->setHelpOption("h", "help", "help page");

    parser->defineOption(
            OPTION_LIST,
            "list of integers",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_K_LAST,
            "value of k, k > 1. e.g. '--k-last 2' means we remove the 2nd to last \
node of the list.",
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

// the algorithm is very simple, and the key observations to solve it are 
// the following (obs. 1 is very obvious, obs. 2 not so much):
//  1) in order to delete a node n in O(1) time, we would need to re-direct 
//     the next ptr of node n - 1 to n + 1
//  2) since we don't have a way to get n - 1 in O(1) time, a O(1) solution seems 
//     impossible. however, note that we can simply use obs. 1 on node n + 1 
//     instead! we set n->data = (n + 1)->data, n->next = (n + 1)->next = (n + 2), 
//     and simply delete n + 1.
void remove_node_by_ptr(
    std::shared_ptr<List_Node<int>> list,
    std::shared_ptr<List_Node<int>> node_to_delete) {

    node_to_delete->data = node_to_delete->next->data;
    node_to_delete->next = node_to_delete->next->next;

    // NOTE : this isn't necessary, since shared_ptr automatically takes 
    // care of the deletion of the object poited by the shared_ptr if all 
    // shared_ptrs holding a ref to the object are assigned a new value.
    // delete old_next;
}

// the algorithm uses some notions used in previous problems:
//  1) use 2 iterators, k_last_ptr and list_iter, advance list_iter 
//     by (k - 1) positions (assume k > 1, i.e. we never ask for the removal of
//     the last node)
//  2) then advance both iterators in tandem, until last_iter reaches the end 
//     of the list
//  3) at this point, use the solution of problem 8.6 to remove the node pointed 
//     by k_last_ptr
//
void remove_k_last_node(
    std::shared_ptr<List_Node<int>> list,
    int k) {

    auto k_last_ptr = list->next, list_iter = list->next;
    while (list_iter->next && (--k > 0)) { list_iter = list_iter->next; }
    while (list_iter->next) { list_iter = list_iter->next; k_last_ptr = k_last_ptr->next; }

    remove_node_by_ptr(list, k_last_ptr);
}

int main (int argc, char **argv) {

    std::shared_ptr<List_Node<int>> list;
    int k_last = 0;

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
        std::cerr << "delete-node::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        list = create_list(arg_parser->optionValue(OPTION_LIST));
        k_last = std::stoi(arg_parser->optionValue(OPTION_K_LAST));
    }

    delete arg_parser;

    if (k_last < 2) {

        std::cerr << "delete-node::main() : [ERROR] k-last > 1 (passed k-last : " 
            << k_last << ")" << std::endl;
        exit(-1);
    }

    std::cout << "delete-node::main() : [INFO] list info : "
        << "\n\t[LIST] : " << to_str(list)
        << "\n\t[K-TH LAST INDEX TO REMOVE] : " << k_last << std::endl;

    remove_k_last_node(list, k_last);
    std::cout << "\t[LIST W/ NODE REMOVED] : " << to_str(list) << std::endl;

    exit(0);
}