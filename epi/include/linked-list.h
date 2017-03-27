#include <memory>

template <typename T>
struct List_Node {

    T data;
    std::shared_ptr<List_Node<T>> next;
};

template <typename T>
std::shared_ptr<List_Node<T>> search_list(std::shared_ptr<List_Node<T>> L, T key) {

    while (L && L->data != key) L = L->next;

    return L;
}

template <typename T>
void insert_after(
    const std::shared_ptr<List_Node<T>> & node,
    const std::shared_ptr<List_Node<T>> & new_node) {

    new_node->next = node->next;
    node->next = new_node;
}

template <typename T>
void delete_after(
    const std::shared_ptr<List_Node<T>> & node,
    const std::shared_ptr<List_Node<T>> & new_node) {

    // we're deleting node->next. since we're using 
    // 'smart' pointers, just remove the reference 
    // to it held by node (i.e. node->next). in this 
    // case, we point it to the node after the deleted 
    // node:
    //
    // before : node -> node_to_delete -> other_node
    // after  : node -> other_node
    //             x -> node_to_delete -> other_node
    // question : would node_to_delete be deleted since
    // it's still pointing to other_node?
    node->next = node->next->next;
}
