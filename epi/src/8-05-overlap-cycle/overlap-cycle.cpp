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
#define OPTION_HAS_CYCLE        (char *) "has-cycle"
#define OPTION_MERGE_INDEX      (char *) "merge-index"
#define SEQUENCE_DELIMITER      (char) ','

ArgvParser * create_argv_parser() {

    ArgvParser * parser = new ArgvParser();

    parser->setIntroductoryDescription("\n\ntest for overlapping lists (with cycles) \
(EPI problem 8.5, page 118)\n\n\nby adamiaonr@gmail.com");
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
of both sublists. leave empty if a non-overlapping case is required.",
            ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);

    parser->defineOption(
            OPTION_HAS_CYCLE,
            "if set, the overlapping list is wrapped around itself in a cycle, \
starting at the specified index.",
            ArgvParser::OptionRequiresValue);

    parser->defineOption(
            OPTION_MERGE_INDEX,
            "overlapping sublist indexes at which sublists 1 and 2 merge.\
e.g. --merge-index '2,3' means sublist 1 will merge at index 2 of overlapping list, \
sublist 2 at index 3. if the index is '-1', the corresponding sublist won't be \
appended to the overlapping list.",
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

void wrap_list(std::shared_ptr<List_Node<int>> list, int cycle_index) {

    if (cycle_index < 0) return;

    // look for the end of the list
    int i = 0;
    auto curr_node = list->next, cycle_node = list->next;
    while(curr_node->next) { 

        if (i++ == cycle_index) cycle_node = curr_node;
        curr_node = curr_node->next; 
    }

    // wrap the list into a cycle
    curr_node->next = cycle_node;
}

std::shared_ptr<List_Node<int>> find_cycle_node(
    std::shared_ptr<List_Node<int>> list) {

    // detect a cycle using slow and fast iterators. slow advances by 1 
    // position, fast by 2 positions.
    auto fast = list, slow = list;

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;

        if (fast == slow) break;
    }

    // if no cycle exists, return NULL (this looks ugly, right?)
    if (fast == NULL || fast->next == NULL) 
        return NULL;

    int cycle_size = 0;
    do { cycle_size++; slow = slow->next; } while (fast != slow);
    // first and last pointers
    auto first = list->next, last = list->next;
    // set last cycle_size positions ahead of first
    while (--cycle_size) last = last->next;
    // advance the last and first in tandem, until last points to first
    while (last->next != first) { last = last->next; first = first->next; }

    return first;
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
std::shared_ptr<List_Node<int>> get_overlapping_node_no_cycle(
    std::shared_ptr<List_Node<int>> list_1,
    std::shared_ptr<List_Node<int>> list_2) {

    // determine sizes of list 1 and 2
    int list_1_size = 0, list_2_size = 0;
    auto list_iter = list_1;

    while (list_iter) { list_iter = list_iter->next; list_1_size++; }
    list_iter = list_2;
    while (list_iter) { list_iter = list_iter->next; list_2_size++; }

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

int get_list_size(
    std::shared_ptr<List_Node<int>> start,
    std::shared_ptr<List_Node<int>> end) {

    int size = 0;
    auto iter = start;
    while (iter != end) { iter = iter->next; size++; }

    return size;
}

// here's the algorithm, using a combination of previous solutions:
//  1) check L_1 and L_2 for cycles.
//  2) if L_x has a cycle and L_y doesn't, return NULL : the lists do not 
//     overlap
//  3) if both lists don't have cycles, use the solution of problem 8.4 (i.e. 
//     the solution reduces to find the overlapping node in lists without cycles)
//
//  4) if both lists have cycles, we can have 3 sub-cases:
//
//     4.1) lists don't overlap
//     4.2) lists merge before the cycle
//     4.3) lists merge within the cycle
// 
// notice this uses solutions used in problems 8.3 and 8.4, which are O(n) in 
// time and O(1) in space. as such, this solution is also O(n) in time and 
// O(1) in space.
std::shared_ptr<List_Node<int>> get_overlapping_node(
    std::shared_ptr<List_Node<int>> list_1,
    std::shared_ptr<List_Node<int>> list_2) {

    // check each list for cycles
    auto l1_cycle_node = find_cycle_node(list_1), l2_cycle_node = find_cycle_node(list_2);

    // if only one of the lists has a cycle, return NULL because the lists 
    // don't overlap
    if ((l1_cycle_node && (l2_cycle_node == NULL)) 
        || ((l1_cycle_node == NULL) && l2_cycle_node)) {

        return NULL;

    } else if (l1_cycle_node) {

        // both lists have cycles. verify the occurrence of 1 of 3 sub-cases

        // 1) cycles do not overlap : to check this, iterate over one of 
        // the lists to check if - at any point - there's a common node
        auto l1_iter = l1_cycle_node->next;
        while (l1_iter != l1_cycle_node && l1_iter != l2_cycle_node)
            l1_iter = l1_iter->next;

        if (l1_iter != l2_cycle_node) return NULL;

        // 2 or 3) lists merge before the cycle : in this case, we first try to 
        // determine if lists merge BEFORE the cycle. if that doesn't happen,
        // then we simply return one of the cycle nodes (book specifies that 
        // as a satisfactory answer)

        // to determine if lists merge before the cycle, use the technique of 
        // problem 8.4 : find the sizes of the lists in-between head and 
        // cycle nodes.
        int l1_size_before_cycle = get_list_size(list_1, l1_cycle_node);
        int l2_size_before_cycle = get_list_size(list_2, l2_cycle_node);

        // then, advance the iterator of the longer list according to the 
        // difference in sizes.
        l1_iter = list_1;
        auto l2_iter = list_2;
        if (l1_size_before_cycle > l2_size_before_cycle) {

            while (l1_size_before_cycle-- > l2_size_before_cycle) l1_iter = l1_iter->next;

        } else {

            while (l2_size_before_cycle-- > l1_size_before_cycle) l2_iter = l2_iter->next;
        }

        // finally, iterate the 2 lists in tandem, until 1) both nodes are the 
        // same, or 2) one of the cycle nodes is reached.
        while ((l1_iter != l2_iter) && l1_iter != l1_cycle_node && l2_iter != l2_cycle_node) {
            l1_iter = l1_iter->next;
            l2_iter = l2_iter->next;
        }

        // if both are the same, then return it, if not, return a cycle node
        return (l1_iter == l2_iter ? l1_iter : l1_cycle_node);


    } else {

        // if both lists DON'T have cycles, use solution to problem 8.4
        return get_overlapping_node_no_cycle(list_1, list_2);
    }
}

std::string to_str(std::shared_ptr<List_Node<int>> list) {

    std::string list_str;
    auto curr_node = list->next;
    // if the list has cycles, we should be careful printing them
    auto cycle_node = find_cycle_node(list);
    bool in_cycle = false;

    while(curr_node) {

        list_str += std::to_string(curr_node->data) + ", ";
        curr_node = curr_node->next;

        if (curr_node == cycle_node) {
            if (in_cycle) {
                break;
            } else {
                in_cycle = true;
            }
        }
    }

    return list_str;
}

void append_lists(
    std::shared_ptr<List_Node<int>> before,
    std::shared_ptr<List_Node<int>> after,
    int index) {

    if (index < 0) return;

    auto before_end = before, after_start = after->next;
    while(before_end->next) { before_end = before_end->next; }
    int i = 0;
    while(after_start->next && i < index) { after_start = after_start->next; i++; }

    before_end->next = after_start;
}

int main (int argc, char **argv) {

    std::shared_ptr<List_Node<int>> list_1, list_2, overlapping_list;
    int has_cycle = -1;
    std::vector<int> merge_index;

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
        std::cerr << "overlap-cycle::main() : [ERROR] use option -h for help." << std::endl;

        delete arg_parser;
        exit(-1);

    } else {

        // extract sublists 1 and 2, common sublist, cycle index for common 
        // sublist and merging index array
        list_1 = create_list(arg_parser->optionValue(OPTION_SUBLIST_1));
        list_2 = create_list(arg_parser->optionValue(OPTION_SUBLIST_2));
        
        overlapping_list = create_list(arg_parser->optionValue(OPTION_OVERLAP_LIST));
        if (arg_parser->foundOption(OPTION_HAS_CYCLE)) {
            has_cycle = std::stoi(arg_parser->optionValue(OPTION_HAS_CYCLE));
        }

        merge_index = extract_sequence(arg_parser->optionValue(OPTION_MERGE_INDEX), SEQUENCE_DELIMITER);
    }

    delete arg_parser;

    if (overlapping_list->next) {

        // wrap the list, if instructed in the merging index array
        wrap_list(overlapping_list, has_cycle);

        // append overlapping lists to sublists, at the specified indexes
        append_lists(list_1, overlapping_list, merge_index[0]);
        append_lists(list_2, overlapping_list, merge_index[1]);
    }

    std::cout << "overlap-cycle::main() : [INFO] list info : "
        << "\n\t[LIST_1] : " << to_str(list_1)
        << "\n\t[LIST_2] : " << to_str(list_2);

    // find the overlapping point
    auto overlapping_node = get_overlapping_node(list_1, list_2);
    std::cout << "\n\t[OVERLAPPING NODE] : ";

    if (overlapping_node == NULL) {

        std::cout << "NONE" << std::endl;

    } else {

        std::cout << overlapping_node->data << std::endl;
    }

    exit(0);
}