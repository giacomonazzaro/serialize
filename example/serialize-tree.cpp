#include "../serialize.h"
#include "../../tree/tree.h"

void serialize_node(Serializer& srl, Node& node, bool save) {
    serialize(srl, node.parent, save);
    serialize_vector(srl, node.children, save);
    // bool has_data;
    // if(save) has_data = node.data != nullptr;
    // else     serialize(srl, has_data, save);
    // if(has_data) serialize(srl, *(node.data), save);
}

void serialize_tree(Serializer& srl, Tree& tree, bool save) {
    serialize_vector<Node>(srl, tree, save, serialize_node);
}

void test_write() {
    auto tree = make_random_tree(0);
    draw_tree(tree, "tree.txt");

    printf("tree size: %d\n", tree.size());
    auto writer = make_serializer("tree.bin", true, 10000);
    serialize_tree(writer, tree, true);
    close_serializer(writer);
}

void test_read() {
    auto reader = make_serializer("tree.bin", false, 10000);
    std::vector<Node> tree;
    serialize_tree(reader, tree, false);
    close_serializer(reader);
    draw_tree(tree, "tree-new.txt");
}

int main() {
    test_write();
    test_read();
}